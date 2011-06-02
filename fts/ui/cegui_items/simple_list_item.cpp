/**
 * \file cegui_items.cpp
 * \author Pompei2
 * \date unknown (very old)
 * \brief This file implements all possible custom CEGUI items.
 **/

#include <CEGUI.h>

#include "ui/cegui_items/simple_list_item.h"

#include "logging/logger.h"
#include "ui/ui.h"
#include "utilities/utilities.h"

///////////////////////////////////////////////////////////////////////////////
// SIMPLE LIST ITEM - SIMPLE LIST ITEM - SIMPLE LIST ITEM - SIMPLE LIST ITEM //
///////////////////////////////////////////////////////////////////////////////
#include <cmath>

using namespace FTS;

/// Create a simple list item.
/** This is the constructor for creating an item that will go into the
 *  listbox that contains the players that are on a channel.
 *
 * \param in_sName The name of the player to insert.
 * \param in_uiItemID Default CEGUI argument.
 * \param in_pItemData Default CEGUI argument.
 * \param in_bDisabled Default CEGUI argument.
 * \param in_bAutoDelete Default CEGUI argument.
 *
 * \author Pompei2
 */
FTS::SimpleListItem::SimpleListItem(const String& in_sText)
    : ListboxTextItem(in_sText)
{
    try {
        this->setSelectionBrushImage("ArkanaLook", "ListSelectionBrush");
        this->setSelectionColours(CEGUI::colour(0xFFFFFFFF));
        this->setTextColours(CEGUI::colour(0xFF524423));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
}

/// sets this item as default in a combo box.
/** This function sets this list item as the currently selected
 *  one in a combobox.
 *
 * \param in_pCB The comboBox where to select me
 *
 * \author Pompei2
 */
void FTS::SimpleListItem::setAsDefault(CEGUI::Combobox* in_pCB)
{
    try {
        in_pCB->setItemSelectState(this, true);
        in_pCB->setText(this->getText());
        this->setSelected(true);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
}

/// sets this item as default in a list box.
/** This function sets this list item as the currently selected
 *  one in a listbox.
 *
 * \param in_pCB The list box where to select me
 *
 * \author Pompei2
 */
void FTS::SimpleListItem::setAsDefault(CEGUI::Listbox* in_pLB)
{
    try {
        in_pLB->setItemSelectState(this, true);
        in_pLB->setText(this->getText());
        this->setSelected(true);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
}

/// adds this item to a combo box and selects it.
/** This function adds itself into a combo box and then sets
 *  itself as being the currently selected item.
 *
 * \param in_pCB The combo box where to add me
 *
 * \author Pompei2
 */
void FTS::SimpleListItem::addAsDefault(CEGUI::Combobox* in_pCB)
{
    try {
        in_pCB->addItem(this);
        this->setAsDefault(in_pCB);
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
void FTS::SimpleListItem::addAsDefault(CEGUI::Listbox* in_pLB)
{
    try {
        in_pLB->addItem(this);
        this->setAsDefault(in_pLB);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
}

void FTS::SimpleListItem::addAsLast(CEGUI::Listbox * in_pLB)
{
    try {
        in_pLB->addItem(this);

        // Care to not overfill the listbox.
        while(in_pLB->getItemCount() > MaxListItems) {
            in_pLB->removeItem(in_pLB->getListboxItemFromIndex(0));
        }

        in_pLB->ensureItemIsVisible(this);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

}

/// Overloaded function to get the listbox item's size.
CEGUI::Size FTS::SimpleListItem::getPixelSize() const
{
    try {
        CEGUI::Size s = ListboxTextItem::getPixelSize();

        // Get our client area.
        CEGUI::Rect clientRect = this->getOwnerWindow()->getInnerRect();

        // If we're in a listbox, get the exact client area (w/o scrollbars, border, ...)
        const CEGUI::Listbox *pLB = dynamic_cast<const CEGUI::Listbox *>(this->getOwnerWindow());
        if(pLB)
            clientRect = pLB->getListRenderArea();

        if(this->getFont()) {
            size_t nLines = this->getFont()->getFormattedLineCount(this->getText(), clientRect, CEGUI::WordWrapLeftAligned);
            s.d_height = nLines * std::ceil(this->getFont()->getLineSpacing());
        }

        return s;
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return CEGUI::Size();
    }
}

/// Overloaded function to draw the listbox item.
void FTS::SimpleListItem::draw(const CEGUI::Vector3 & in_position,
                               float in_fAlpha,
                               const CEGUI::Rect & in_clipper) const
{
    CEGUI::Vector3 vTxt = in_position;

    try {
        // Draw the selection box first.
        if(d_selected && (d_selectBrush != 0)) {
            d_selectBrush->draw(in_clipper, in_position.d_z, in_clipper, getModulateAlphaColourRect(d_selectCols, in_fAlpha));
        }

        // And the text next.
        CEGUI::Font* pFont = this->getFont();
        if(pFont) {
            vTxt.d_y += PixelAligned((pFont->getLineSpacing() - pFont->getFontHeight()) * 0.5f);
            pFont->drawText(d_itemText, vTxt, in_clipper, getModulateAlphaColourRect(d_textCols, in_fAlpha));
        }
    } catch(CEGUI::Exception &) {
    }
}

/// Overloaded function to draw the listbox item.
void FTS::SimpleListItem::draw(CEGUI::RenderCache & in_cache,
                               const CEGUI::Rect & in_targetRect,
                               float in_fZBase, float in_fAlpha,
                               const CEGUI::Rect *in_pClipper) const
{
    CEGUI::Rect rTxt = in_targetRect;

    try {
        // Draw the selection box first.
        if(d_selected && d_selectBrush != 0) {
            in_cache.cacheImage(*d_selectBrush, in_targetRect, in_fZBase, getModulateAlphaColourRect(d_selectCols, in_fAlpha), in_pClipper);
        }

        // And the text next.
        CEGUI::Font* pFont = this->getFont();
        if(pFont) {
            rTxt.d_top += PixelAligned((pFont->getLineSpacing() - pFont->getFontHeight()) * 0.5f);
            in_cache.cacheText(d_itemText, pFont, CEGUI::LeftAligned, rTxt, in_fZBase, getModulateAlphaColourRect(d_textCols, in_fAlpha), in_pClipper);
        }
    } catch(CEGUI::Exception &) {
    }
}

///////////////////////////////////////////////////////////////////////////////
// SERVER LIST ITEM - SERVER LIST ITEM - SERVER LIST ITEM - SERVER LIST ITEM //
///////////////////////////////////////////////////////////////////////////////

/// Create a server list item.
/** This is the constructor for creating an item that will go into the
 *  combobox that contains the server. This item gets a description and
 *  an address, the description will be displayed first, then the address
 *  is appended, in paranthesis.
 *
 * \param in_sDesc The description (first part of the string)
 * \param in_sAddr The server address (second part, in paranthesis)
 *
 * \author Pompei2
 */
FTS::ServerListItem::ServerListItem(const String& in_sAddr, const String& in_sDesc)
    : SimpleListItem(ServerListItem::calcText(in_sAddr, in_sDesc))
    , m_sDesc(in_sDesc)
    , m_sAddr(in_sAddr)
{
}

String FTS::ServerListItem::calcText(const String& in_sAddr, const String& in_sDesc)
{
    if(in_sDesc.empty()) {
        return in_sAddr;
    } else {
        return in_sDesc + " (" + in_sAddr + ")";
    }
}

 /* EOF */
