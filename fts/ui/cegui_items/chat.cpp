/**
 * \file cegui_chat_items.cpp
 * \author Pompei2
 * \date 04 July 2007
 * \brief This file implements all custom CEGUI items that have something to do with chatting.
 **/
#include <ctime>
#include <CEGUI.h>

#include "ui/cegui_items/chat.h"

#include "ui/ui.h"
#include "game/player.h"
#include "logging/logger.h"
#include "utilities/utilities.h"
#include "dLib/dString/dTranslation.h"

using namespace FTS;

//////////////////////////////////////////////////////////////////////////////
// CHAT MEMBERS LIST ITEM - CHAT MEMBERS LIST ITEM - CHAT MEMBERS LIST ITEM //
//////////////////////////////////////////////////////////////////////////////

/// Create an item for the channel members listbox.
/** This is the constructor for creating an item that will go into the
 *  listbox that contains the players that are on a channel.
 *
 * \param in_sName The name of the player to insert.
 * \param in_uiItemID Default CEGUI argument.
 * \param in_pItemData Default CEGUI argument.
 * \param in_bDisabled Default CEGUI argument.
 * \param in_bAutoDelete Default CEGUI argument.
 *
 *\author Pompei2
 */
FTS::ChatMembersListItem::ChatMembersListItem(const String & in_sName,
                                              CEGUI::uint in_uiItemID,
                                              void *in_pItemData,
                                              bool in_bDisabled,
                                              bool in_bAutoDelete)
                         :ListboxTextItem(in_sName,
                                          in_uiItemID,
                                          in_pItemData,
                                          in_bDisabled,
                                          in_bAutoDelete)
{
    try {
        this->setAutoDeleted(true);
        this->setSelectionBrushImage("ArkanaLook", "ListSelectionBrush");
        this->setSelectionColours(CEGUI::colour(1.0f, 1.0f, 1.0f, 1.0f));
        m_fSkillPercent = 0.0f;
        m_cState = DSRV_CHAT_USER::UNKNOWN;
        m_sName = in_sName;

        this->recalcText();
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
}

/// Default destructor for an item for the channel members listbox.
/** This is the default destructor for destroying an item that was
 *  in the listbox that contains the players that are on a channel.
 *
 * \author Pompei2
 */
FTS::ChatMembersListItem::~ChatMembersListItem()
{
}

/// sets this item as default in a list box.
/** This function sets this list item as the currently selected
 *  one in a listbox.
 *
 * \param in_pCB The list box where to select me
 *
 * \author Pompei2
 */
void FTS::ChatMembersListItem::setAsDefault(CEGUI::Listbox * in_pLB)
{
    try {
        in_pLB->setItemSelectState(this, true);
        in_pLB->setText(this->getText());
        this->setSelected(true);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
}

/// adds this item to a list box and selects it.
/** This function adds itself into a list box and then sets
 *  itself as being the currently selected item.
 *
 * \param in_pCB The list box where to add me
 *
 * \author Pompei2
 */
void FTS::ChatMembersListItem::addAsDefault(CEGUI::Listbox * in_pLB)
{
    try {
        in_pLB->addItem(this);
        this->setAsDefault(in_pLB);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
}

/// Sets the skill percent of this player.
/** This sets the skill percent of this player to a value and adjusts
 *  it's display color accordingly.
 *
 * \param in_fSkill The skill percent of the player (from 0.0f to 1.0f).
 *
 * \author Pompei2
 */
void FTS::ChatMembersListItem::setSkillPercent(float in_fSkill)
{
    m_fSkillPercent = in_fSkill;
    this->recalcText();
}

/// Gets the skill percent of this player.
/** This returns the skill percent that has been given to this player.
 *
 * \return The skill percent of the player (from 0.0f to 1.0f).
 *
 * \note This may not be the correct value, it is only the one it just got told.
 *
 * \author Pompei2
 */
float FTS::ChatMembersListItem::getSkillPercent() const
{
    return m_fSkillPercent;
}

/// Gets the name of this player.
/** This returns the name that has been given to this player.
 *
 * \return The name of the player.
 *
 * \author Pompei2
 */
String FTS::ChatMembersListItem::getName() const
{
    return m_sName;
}

/// Sets the state of this player.
/** This sets the state of this player to a value and adjusts
 *  it's display color accordingly.
 *
 * \param in_cState The state of the player (0 = normal, 1 = operator, 2 = admin).
 *
 * \author Pompei2
 */
void FTS::ChatMembersListItem::setState(DSRV_CHAT_USER in_cState)
{
    String sSuffix;
    m_cState = in_cState;

    this->recalcText();
}

/// Gets the state of this player.
/** This returns the state that has been given to this player.
 *
 * \return The state of the player (0 = normal, 1 = operator, 2 = admin).
 *
 * \note This may not be the correct value, it is only the one it just got told.
 *
 * \author Pompei2
 */
DSRV_CHAT_USER FTS::ChatMembersListItem::getState() const
{
    return m_cState;
}

/// Re-calculates the text.
/** This function re-calculates (and sets) the text, based on the state
 *  and muted state of the player.
 *
 * \author Pompei2
 */
void FTS::ChatMembersListItem::recalcText()
{
    String sText = m_sName;
    float fR = 0.0f, fG = 0.0f, fB = 0.0f, fA = 1.0f;
    bool bMuted = false;

    bMuted = g_pMeHacky->og_haveMuted(this->getName());

    // Get the color.
    if(bMuted) {
        fR = 0.7f;
    } else {
        skillPercentColor(this->getSkillPercent(), fR, fG, fB, fA);
    }

    Translation trans("ui");
    if( m_cState == DSRV_CHAT_USER::OPERATOR ) {
        sText += trans.get("Chat_Op_suffix");
    } else if( m_cState == DSRV_CHAT_USER::ADMIN ) {
        sText += trans.get("Chat_Admin_suffix");
    }

    if(bMuted)
        sText += trans.get("Chat_Muted_suffix");

    try {
        this->setText(sText);
        this->setTextColours(CEGUI::colour(fR, fG, fB, fA));

        if(this->getOwnerWindow())
            ((CEGUI::Listbox *)this->getOwnerWindow())->handleUpdatedItemData();
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
}

CEGUI::Size FTS::ChatMembersListItem::getPixelSize() const
{
    CEGUI::Size tmp(0, 0);
    float fHExtent = this->getFont()
                         ->getFormattedTextExtent(this->getText(),
                                                  this->getOwnerWindow()->getInnerRect(),
                                                  CEGUI::WordWrapLeftAligned);

    tmp.d_height = PixelAligned(this->getFont()->getLineSpacing());
    tmp.d_width = PixelAligned(fHExtent);

    return tmp;
}

///////////////////////////////////////////////////////////////////
// CHAT NORMAL MESSAGE LIST ITEM - CHAT NORMAL MESSAGE LIST ITEM //
///////////////////////////////////////////////////////////////////

/// Create a normal item for the channel messages listbox.
/** This is the constructor for creating an item that will go into the
 *  listbox that contains a normal message sent in a channel.
 *
 * \param in_sFrom The name of the player who sent the message.
 * \param in_sMessage The contents of the message.
 * \param in_uiItemID Default CEGUI argument.
 * \param in_pItemData Default CEGUI argument.
 * \param in_bDisabled Default CEGUI argument.
 * \param in_bAutoDelete Default CEGUI argument.
 *
 * \author Pompei2
 */
FTS::ChatMsgListItem::ChatMsgListItem(const String & in_sFrom,
                                      const String & in_sMessage,
                                      CEGUI::uint in_uiItemID,
                                      void *in_pItemData,
                                      bool in_bDisabled,
                                      bool in_bAutoDelete)
                     :CEGUI::ListboxItem(in_sFrom + in_sMessage,
                                         in_uiItemID,
                                         in_pItemData,
                                         in_bDisabled,
                                         in_bAutoDelete)
{
    m_sFrom = in_sFrom;
    m_sMessage = in_sMessage;

    time_t t = time(NULL);
    tm *time = localtime(&t);

    m_sTimeStr = "("+String::nr(time->tm_hour,2,'0')+":"
                    +String::nr(time->tm_min,2,'0')+":"
                    +String::nr(time->tm_sec,2,'0')+") ";

    try {
        this->setAutoDeleted(true);

        ChatMembersListItem *pcmli = NULL;
        FTSGetConvertWinMacro(CEGUI::Listbox, lb, "menu_online_main/lbUsers");

        // Find the item that represents this user in the users lb.
        // We do it like this, so we find the user even if he is Op or Admin.
        size_t items = lb ? lb->getItemCount() : 0;
        for( size_t i = 0 ; i < items ; i++ ) {
            pcmli = dynamic_cast<ChatMembersListItem *>(lb->getListboxItemFromIndex(i));
            if(pcmli && pcmli->getName() == in_sFrom)
                break;
            else pcmli = NULL;
        }

        // Get the skill percents of the user who sent the message.
        if(!pcmli) {
            m_fSkillPercent = 0.0f;
        } else {
            m_fSkillPercent = pcmli->getSkillPercent();
        }

        // Set the color - temporary (TODO)
        m_textCols.setColours(CEGUI::colour(0xFF1B56A0));
        m_nickCol = CEGUI::colour(0xFFFFFFFF);
        m_timeCol = CEGUI::colour(0xFF3F3F3F);
        m_bWithTime = true;
        m_bForWhisp = false;

        m_pFont = CEGUI::System::getSingleton().getDefaultFont();
        this->buildText();
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
}

/// Default destructor for an item for the channel messages listbox.
/** This is the default destructor for destroying an item that was
 *  in the listbox that contains all messages sent in a channel.
 *
 * \author Pompei2
 */
FTS::ChatMsgListItem::~ChatMsgListItem()
{
}

/// adds this item to a list box rolls the listbox down.
/** This function adds itself into a list box and then
 *  scrolls the listbox down so it is visible.
 *
 * \param in_pCB The list box where to add me
 *
 * \author Pompei2
 */
void FTS::ChatMsgListItem::addLast(CEGUI::Listbox * in_pLB)
{
    try {
        in_pLB->addItem(this);

        // Care to not overfill the listbox.
        while(in_pLB->getItemCount() > D_MAX_CHAT_ENTRIES) {
            in_pLB->removeItem(in_pLB->getListboxItemFromIndex(0));
        }

        in_pLB->ensureItemIsVisible(this);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
}

CEGUI::Size FTS::ChatMsgListItem::getPixelSize() const
{
    CEGUI::Size tmp(0, 0);

    // Get our client area.
    CEGUI::Rect clientRect = this->getOwnerWindow()->getInnerRect();

    // If we're in a listbox (I hope we are), get the exact client area (w/o scrollbars, border, ...)
    const CEGUI::Listbox *pLB = dynamic_cast<const CEGUI::Listbox *>(this->getOwnerWindow());
    if(pLB)
        clientRect = pLB->getListRenderArea();

    size_t nLines = m_pFont->getFormattedLineCount(this->getText(), clientRect,
                                                   CEGUI::WordWrapLeftAligned);
    float fHExtent = m_pFont->getFormattedTextExtent(this->getText(), clientRect,
                                                     CEGUI::WordWrapLeftAligned);

    tmp.d_height = (PixelAligned(m_pFont->getLineSpacing())) * nLines;
    tmp.d_width = PixelAligned(fHExtent);

    return tmp;
}

void FTS::ChatMsgListItem::draw(const CEGUI::Vector3 & in_position,
                                float in_fAlpha,
                                const CEGUI::Rect & in_clipper) const
{
    CEGUI::Vector3 finalPos(in_position);

    finalPos.d_y +=
        PixelAligned((m_pFont->getLineSpacing() -
                      m_pFont->getFontHeight()) * 0.5f);

    m_pFont->drawText(this->getText(), finalPos, in_clipper,
                      getModulateAlphaColourRect(m_textCols, in_fAlpha));
}

void FTS::ChatMsgListItem::draw(CEGUI::RenderCache & in_cache,
                                const CEGUI::Rect & in_targetRect,
                                float in_fZBase,
                                float in_fAlpha,
                                const CEGUI::Rect *in_pClipper) const
{
    CEGUI::Rect finalPos(in_targetRect);

    finalPos.d_top +=
        PixelAligned((m_pFont->getLineSpacing() -
                      m_pFont->getFontHeight()) * 0.5f);

    in_cache.cacheText(this->getText(), m_pFont,
                       CEGUI::WordWrapLeftAligned, finalPos, in_fZBase,
                       getModulateAlphaColourRect(m_textCols, in_fAlpha),
                       in_pClipper);
}

/// Builds the text to be displayed.
/** This function builds the text that will be displayed and sets
 *  it as the current item text.
 *
 * \return the string that should be displayed.
 *
 * \author Pompei2
 */
String FTS::ChatMsgListItem::buildText()
{
    String sText = String::EMPTY;
    String sFormat = String::EMPTY;
    // Time.
    if(m_bWithTime) {
        sText = "|c" + String::sfromHex((unsigned char)(m_timeCol.getRed()*255.0f)) +
                       String::sfromHex((unsigned char)(m_timeCol.getGreen()*255.0f)) +
                       String::sfromHex((unsigned char)(m_timeCol.getBlue()*255.0f)) +
                       m_sTimeStr + "|d";
    }

    sText += "|c" + String::sfromHex((unsigned char)(m_nickCol.getRed()*255.0f)) +
                    String::sfromHex((unsigned char)(m_nickCol.getGreen()*255.0f)) +
                    String::sfromHex((unsigned char)(m_nickCol.getBlue()*255.0f));
    Translation trans("ui");
    switch(m_eUse) {
    case RecvWhisp:
        sFormat = trans.get("Chat_RecvWhisp") + "|d";
        break;
    case SentWhisp:
        sFormat = trans.get("Chat_SentWhisp") + "|d";
        break;
    case System:
        sFormat = "|d";
        break;
    case Normal:
    default:
        sFormat = trans.get("Chat_Says") + "|d";
        break;
    }

    // Message.
    sText += sFormat.fmt(m_sFrom) + m_sMessage;
    this->setText(sText);
    return sText;
}
