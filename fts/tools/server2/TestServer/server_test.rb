require 'socket'
require_relative 'fts_packets'
require_relative 'fts_connection'

$statMsgSend = Hash.new( 0 )
$statMsgRecv = Hash.new( 0 )

$kindNames = Hash.new
$kindNames[MsgType::LOGIN] = 'Login'
$kindNames[MsgType::LOGOUT] = 'Logout'
$kindNames[MsgType::CHAT_SEND_MSG] = 'ChatSend'
$kindNames[MsgType::CHAT_GET_MSG] = 'ChatGet'
$kindNames[MsgType::JOINCHAT] = 'JoinChat'
$kindNames[MsgType::SOMEONE_JOINS_THE_CHAT] = 'Joined'
$kindNames[MsgType::QUITCHAT] = 'QuitChat'
$kindNames[MsgType::GETCHATLIST] = 'GetChatList'
# curretly not used $kindNames[MsgType::GETCHATUSER] = 'GetChatUser'

class Client
  attr_reader :failedHdr
  attr_reader :failedMsg
  attr_reader :port
  
  def initialize host, port, user, pwd
    @host = host
    @port = port
    @user = user
    @pwd = pwd
    @con = Connection.new host, port
    @isDone = false
    @failedHdr = Array.new
    @failedMsg = Array.new
    puts "Created Client #{@user}"
  end
  def start
    @treceiver = Thread.start do
      failed = 0
      until @isDone and failed > 3 do
        hdr = @con.recvdata(9)
        if hdr.size != 9
          failed += 1
          @failedHdr << hdr.size
          next
        end
        rhdr = PacketHeader.new
        rhdr.read(hdr)
        ans = @con.recvdata(rhdr.len)
        if ans.size != rhdr.len
          @failedMsg << ans.size
          next
        end
        answer = hdr + ans
        rp = PacketResp.new
        rp.read(answer)
        ba = answer.unpack('H*') if $preint_raw
        puts "#r #{rp} #{ba}" if $debug
        failed = 0
        $statMsgRecv[rp.kind.snapshot] += 1
      end
    end
  end
  def sender msg
    sleep(0.1) # make it some how realistic
    @con.senddata msg.to_binary_s
    puts "#s #{msg} #{sndlen} bytes" if $debug
    $statMsgSend[msg.kind.snapshot] += 1 # snapshot returns the value as FinNum a ruby object
  end
  def login
    msg = Packet.new( :ident => 'FTSS', :kind => MsgType::LOGIN, :user => @user, :pwd => @pwd )
    msg.len = msg.user.size + msg.pwd.size + 2 # 2 end string delimiter
    sender msg
  end
  def join
    msg = Packet.new( :ident => 'FTSS', :kind => MsgType::JOINCHAT, :pwd => @pwd, :room => "UselessChan" )
    msg.len = msg.pwd.size + "UselessChan".size + 2 

    sender msg
  end
  def listChatUsers
    msg = Packet.new( :ident => 'FTSS', :kind => MsgType::GETCHATLIST, :pwd => @pwd )
    msg.len = msg.pwd.size + 1 # 1 end string delimiter

    sender msg
  end
  def chatMessage
    msg = Packet.new( :ident => 'FTSS', :kind => MsgType::CHAT_SEND_MSG, :pwd => @pwd )
    msg.text = "Hello you. Here John Doe."
    msg.len = 2 + msg.pwd.size + msg.text.size + 2 # chat_type + flags+ 2 * end string delimiter

    sender msg
  end
  def logout
    msg = Packet.new( :ident => 'FTSS', :kind => MsgType::LOGOUT, :pwd => @pwd )
    msg.len = msg.pwd.size + 1 # 1 end string delimiter

    sender msg
  end
  def quit
    @isDone = true
    @treceiver.join
    @con.close
  end
end

def testCase1( client )
  client.start
  client.login
  client.join
  client.listChatUsers
  for i in 0..19
    client.chatMessage
  end
  sleep(0.1)
  client.logout
  client.quit
  puts "Client #{client.port} hdr #{client.failedHdr} msg #{client.failedMsg}" if client.failedHdr.size > 4
end

thr = Array.new
(0..9).each do |x|
  port = 44917 + x
  userpwd = "Test#{port}"
  thr << Thread.start do
    testCase1 Client.new 'localhost', port, userpwd, userpwd
  end
end

thr.each do |t|
  t.join
end

txt = "Result: "
$kindNames.each do |k,v|
  txt += "#{v}(#{$statMsgSend[k]}/#{$statMsgRecv[k]}) "
end
puts txt


