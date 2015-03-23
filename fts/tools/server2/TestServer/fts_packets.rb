require 'bindata'

class MsgType
  LOGIN=1
  LOGOUT=2
  CHAT_SEND_MSG=0x30          #48
  CHAT_GET_MSG=0x31           #49
  JOINCHAT=0x33               #51
  SOMEONE_JOINS_THE_CHAT=0x34 #52
  QUITCHAT=0x35               #53
  GETCHATLIST=0x39            #57
  GETCHATUSER=0x3A            #58
  DESTROY_CHAN=0x43           #67
end

class ChatType
  NORMAL=1
  WHISPER=2
  SYSTEM=4
end

class PacketHeader < BinData::Record
  endian :little
  string  :ident, :length => 4
  uint8   :kind
  uint32  :len
end

class Packet < PacketHeader
  endian :little
  stringz :user, :onlyif => :isUserRequired?
  stringz :pwd
  stringz :room, :onlyif => lambda{ kind == MsgType::JOINCHAT or kind == MsgType::DESTROY_CHAN }
  uint8   :chat_type, :value => 1, :onlyif => lambda{ kind == MsgType::CHAT_SEND_MSG }
  uint8   :flags, :value => 0, :onlyif => lambda{ kind == MsgType::CHAT_SEND_MSG }
  stringz :toUser, :onlyif => lambda{ chat_type == ChatType::WHISPER and kind == MsgType::CHAT_SEND_MSG }
  stringz :text, :onlyif => lambda{ kind == MsgType::CHAT_SEND_MSG }
  def isUserRequired?
    kind == MsgType::LOGIN
  end
end

class PacketResp < PacketHeader
  endian  :little
  uint8   :result, :onlyif => :hasResult?
  uint8   :chat_type, :value => 1, :onlyif => lambda{ kind == MsgType::CHAT_GET_MSG }
  uint8   :flags, :value => 0, :onlyif => lambda{ kind == MsgType::CHAT_GET_MSG }
  uint32  :users, :onlyif => :isUserList?
  array   :data, :type => :uint8, :onlyif => :hasData?, :initial_length => lambda { len - 1 }
  array   :user_names, :type => :stringz, :onlyif => :isUserList?, :initial_length => lambda { users }
  stringz :name, :onlyif => lambda {kind == MsgType::QUITCHAT or kind == MsgType::SOMEONE_JOINS_THE_CHAT or kind == MsgType::CHAT_GET_MSG}
  stringz :text, :onlyif => lambda{ kind == MsgType::CHAT_GET_MSG }

  def hasData?
    not isUserList? and hasResult? and len > 1 and result == 0
  end
  def isUserList?
    kind == MsgType::GETCHATLIST
  end
  def hasResult?
    kind != MsgType::QUITCHAT and kind != MsgType::SOMEONE_JOINS_THE_CHAT and kind != MsgType::CHAT_GET_MSG
  end
end
