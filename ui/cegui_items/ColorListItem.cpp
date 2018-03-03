/**
 * \file cegui_items.cpp
 * \author Pompei2
 * \date unknown (very old)
 * \brief This file implements all possible custom CEGUI items.
 **/

#include <CEGUI.h>

#include "ui/cegui_items/ColorListItem.h"

#include "logging/logger.h"

using namespace FTS;

/// Initialises an item of the file dialog listbox.
FTS::ColorListItem::ColorListItem(const FTS::Color& in_col, const CEGUI::String& in_text, void* in_pItemData, CEGUI::uint in_uiItemID, bool in_bDisabled, bool in_bAutoDelete)
    : CEGUI::ListboxTextItem(in_text, in_uiItemID, in_pItemData, in_bDisabled, in_bAutoDelete)
    , m_col(in_col)
{
    try {
        this->setAutoDeleted(true);
        this->setSelectionBrushImage("ArkanaLook", "ListSelectionBrush");
        this->setSelectionColours(CEGUI::colour(0xFFFFFFFF));
        this->setTextColours(CEGUI::colour(0xFF524423));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
}

/// Destroys an item of the file dialog listbox.
FTS::ColorListItem::~ColorListItem()
{
}

/// Overloaded function to get the listbox item's size.
CEGUI::Size FTS::ColorListItem::getPixelSize() const
{
    CEGUI::Size s = ListboxTextItem::getPixelSize();

    // We make the color-box be a square, using the font height.
    s.d_width += s.d_height + 4; // + the place between color box and font.

    return s;
}

/// Overloaded function to draw the listbox item.
void FTS::ColorListItem::draw(const CEGUI::Vector3 & in_position, float in_fAlpha, const CEGUI::Rect & in_clipper) const
{
    CEGUI::Vector3 vTxt = in_position;

    try {
        // Draw the selection box first.
        if(d_selected && (d_selectBrush != 0)) {
            d_selectBrush->draw(in_clipper, in_position.d_z, in_clipper, getModulateAlphaColourRect(d_selectCols, in_fAlpha));
        }
    } catch(CEGUI::Exception &) {
    }

    try {
        // Draw the color and adjust the text's X position.
        vTxt.d_x += 4 + this->getPixelSize().d_height;
        CEGUI::ImagesetManager::getSingleton().getImageset("ArkanaLook")->getImage("OneWhitePixel")
            .draw(in_position, CEGUI::Size(this->getPixelSize().d_height, this->getPixelSize().d_height), in_clipper, getModulateAlphaColourRect(m_col, in_fAlpha));
    } catch(CEGUI::Exception &) {
    }

    try {
        CEGUI::Font* pFont = this->getFont();

        // Draw the text vertically centred.
        if(pFont) {
            vTxt.d_y += PixelAligned((this->getPixelSize().d_height - pFont->getFontHeight()) * 0.5f);
            pFont->drawText(d_itemText, vTxt, in_clipper, getModulateAlphaColourRect(d_textCols, in_fAlpha));
        }
    } catch(CEGUI::Exception &) {
    }
}

/// Overloaded function to draw the listbox item.
void FTS::ColorListItem::draw(CEGUI::RenderCache & in_cache, const CEGUI::Rect & in_targetRect, float in_fZBase, float in_fAlpha, const CEGUI::Rect *in_pClipper) const
{
    CEGUI::Rect rTxt = in_targetRect;
    CEGUI::Rect rImg = in_targetRect;

    try {
        // Draw the selection box first.
        if(d_selected && d_selectBrush != 0) {
            in_cache.cacheImage(*d_selectBrush, in_targetRect, in_fZBase, getModulateAlphaColourRect(d_selectCols, in_fAlpha), in_pClipper);
        }
    } catch(CEGUI::Exception &) {
    }

    try {
        // Draw the color and adjust the text's X position.
        rImg.d_right = rImg.d_left + this->getPixelSize().d_height;
        rTxt.d_left = 4 + rImg.d_right;
        in_cache.cacheImage(CEGUI::ImagesetManager::getSingleton().getImageset("ArkanaLook")->getImage("OneWhitePixel"),
            rImg, in_fZBase, getModulateAlphaColourRect(m_col, in_fAlpha), in_pClipper);
    } catch(CEGUI::Exception &) {
    }

    try {
        CEGUI::Font *pFont = this->getFont();

        // Vertically centre the text.
        if(pFont) {
            rTxt.d_top += PixelAligned((this->getPixelSize().d_height - pFont->getFontHeight()) * 0.5);
            in_cache.cacheText(d_itemText, pFont, CEGUI::LeftAligned, rTxt, in_fZBase, getModulateAlphaColourRect(d_textCols, in_fAlpha), in_pClipper);
        }
    } catch(CEGUI::Exception &) {
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
void FTS::ColorListItem::setAsDefault(CEGUI::Combobox* in_pCB)
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
void FTS::ColorListItem::setAsDefault(CEGUI::Listbox* in_pLB)
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
void FTS::ColorListItem::addAsDefault(CEGUI::Combobox* in_pCB)
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
void FTS::ColorListItem::addAsDefault(CEGUI::Listbox* in_pLB)
{
    try {
        in_pLB->addItem(this);
        this->setAsDefault(in_pLB);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
}
