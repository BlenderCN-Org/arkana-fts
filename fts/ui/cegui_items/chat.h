#ifndef D_CEGUICHATITEMS_H
#define D_CEGUICHATITEMS_H

#include "main.h"
#include <elements/CEGUIListboxTextItem.h>
#include <CEGUIcolour.h>

#define D_MAX_CHAT_ENTRIES 100

namespace CEGUI {
    class String;
    class Listbox;
    class Combobox;
    class Size;
    class Image;
    class Vector3;
    class Rect;
    class RenderCache;
}

#include "dLib/dString/dString.h"

namespace FTS {

class ChatMembersListItem : public CEGUI::ListboxTextItem {
private:
    String m_sName;
    float m_fSkillPercent;
    char  m_cState;

public:
    ChatMembersListItem(const String & in_sName,
                        CEGUI::uint in_uiItemID = 0,
                        void *in_pItemData = 0,
                        bool in_bDisabled = false,
                        bool in_bAutoDelete = true);
    virtual ~ChatMembersListItem();

    void setAsDefault(CEGUI::Listbox * in_pLB);
    void addAsDefault(CEGUI::Listbox * in_pLB);

    void setState(char in_cState);
    char getState() const;

    void setSkillPercent(float in_fSkill);
    float getSkillPercent() const;

    String getName() const;

    void recalcText();
    CEGUI::Size getPixelSize() const;
};

class ChatMsgListItem : public CEGUI::ListboxItem {
public:
    enum Use {
        Normal,
        System,
        RecvWhisp,
        SentWhisp
    };

private:
    String m_sFrom;              ///< The message text that has been sent.
    String m_sTimeStr;           ///< The time this has been sent, as a string.
    String m_sMessage;           ///< The message text that has been sent.

    float m_fSkillPercent;        ///< The skill percent of the user who wrote this.

    CEGUI::Font * m_pFont;         ///< Font used for rendering the text.
    CEGUI::ColourRect m_textCols;  ///< The normal colour of the text.
    CEGUI::colour m_nickCol;       ///< The normal colour of the text.
    CEGUI::colour m_timeCol;       ///< The normal colour of the text.
    bool m_bWithTime;              ///< Display time or not.
    bool m_bForWhisp;              ///< Whether this message comes from a whisper or not.

    Use m_eUse;  ///< What kind of message this should be.

public:
    ChatMsgListItem(const String & in_sFrom,
                    const String & in_sMessage,
                    CEGUI::uint in_uiItemID = 0,
                    void *in_pItemData = 0,
                    bool in_bDisabled = false,
                    bool in_bAutoDelete = true);
    virtual ~ChatMsgListItem();

    void addLast(CEGUI::Listbox * in_pLB);

    CEGUI::Size getPixelSize() const;
    void draw(const CEGUI::Vector3 & in_position,
              float in_fAlpha, const CEGUI::Rect & in_clipper) const;
    void draw(CEGUI::RenderCache & in_cache,
              const CEGUI::Rect & in_targetRect,
              float in_fZBase,
              float in_fAlpha, const CEGUI::Rect * in_pClipper) const;

    inline ChatMsgListItem *setNickCol(const CEGUI::colour &in_col)
    {
        m_nickCol = in_col;
        this->buildText();
        return this;
    };

    inline ChatMsgListItem *setTimeCol(const CEGUI::colour &in_col)
    {
        m_timeCol = in_col;
        this->buildText();
        return this;
    };

    inline ChatMsgListItem *setTextCol(const CEGUI::colour &in_col)
    {
        m_textCols.setColours(in_col);
        this->buildText();
        return this;
    };

    inline ChatMsgListItem *setWithTime(bool b) {
        m_bWithTime = b;
        this->buildText();
        return this;
    }

    inline bool getWithTime() {return m_bWithTime;};

    inline ChatMsgListItem *setUse(Use in_eUse) {
        m_eUse = in_eUse;
        if(m_eUse == System)
            this->setTextCol(CEGUI::colour(0xFF404040));
        this->buildText();
        return this;
    }

    inline Use getUse() {return m_eUse;};

private:
    String buildText();
};

};

#endif                          /* D_CEGUICHATITEMS_H */

 /* EOF */
