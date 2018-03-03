/**
 * \file cegui_items.cpp
 * \author Pompei2
 * \date unknown (very old)
 * \brief This file implements all possible custom CEGUI items.
 **/

#include <CEGUI.h>

#include "ui/cegui_items/imaged_list_item.h"

#include "logging/logger.h"
#include "ui/ui.h"
#include "graphic/graphic.h"
#include "utilities/utilities.h"

using namespace FTS;

/** Initialises an item of the file dialog listbox. */
FTS::ImagedListItem::ImagedListItem(const CEGUI::String & in_text,
                                    const CEGUI::String &in_sImageset,
                                    const CEGUI::String &in_sImage,
                                    void *in_pItemData,
                                    CEGUI::uint in_uiItemID,
                                    bool in_bDisabled,
                                    bool in_bAutoDelete)
    : ListboxTextItem(in_text, in_uiItemID, in_pItemData, in_bDisabled, in_bAutoDelete)
{
    try {
        m_Img = CEGUI::ImagesetManager::getSingleton()
                                       .getImageset(in_sImageset)
                                      ->getImage(in_sImage);
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    try {
        this->setAutoDeleted(true);
        this->setSelectionBrushImage("ArkanaLook", "ListSelectionBrush");
        this->setSelectionColours(CEGUI::colour(0xFFFFFFFF));
        this->setTextColours(CEGUI::colour(0xFF524423));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    m_pImageset = NULL;
}

FTS::ImagedListItem::ImagedListItem(const CEGUI::String &in_text,
                                    Graphic *in_pGraphic,
                                    void *in_pItemData,
                                    CEGUI::uint in_uiItemID,
                                    bool in_bDisabled,
                                    bool in_bAutoDelete)
    : ListboxTextItem(in_text, in_uiItemID, in_pItemData, in_bDisabled, in_bAutoDelete)
{

    try {
        // Find a disponible random name for the imageset.
        m_pImageset = in_pGraphic->createCEGUI();
        this->setAutoDeleted(true);
        this->setSelectionBrushImage("ArkanaLook", "ListSelectionBrush");
        this->setSelectionColours(CEGUI::colour(0xFFFFFFFF));
        this->setTextColours(CEGUI::colour(0xFF524423));
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
}

/** Destroys an item of the file dialog listbox. */
FTS::ImagedListItem::~ImagedListItem()
{
    CEGUI::ImagesetManager::getSingleton().destroyImageset(m_pImageset);
}

/** Overloaded function to get the listbox item's size. */
CEGUI::Size FTS::ImagedListItem::getPixelSize() const
{
    CEGUI::Size s(1.0f, 1.0f);

    try {
        s = ListboxTextItem::getPixelSize();

        if(m_pImageset) {
            s.d_height = std::max( s.d_height, m_pImageset->getImage("image").getHeight() );
            s.d_width += m_pImageset->getImage("image").getWidth();
        } else {
            s.d_height = std::max( s.d_height, m_Img.getHeight() );
            s.d_width += m_Img.getWidth();
        }
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }

    return s;
}

/** Overloaded function to draw the listbox item. */
void FTS::ImagedListItem::draw(const CEGUI::Vector3 & in_position,
                               float in_fAlpha,
                               const CEGUI::Rect & in_clipper) const
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
        // Draw the image and adjust the text's X position.
        if(m_pImageset) {
            vTxt.d_x += m_pImageset->getImage("image").getWidth();
            m_pImageset->getImage("image").draw(in_position, in_clipper);
        } else {
            vTxt.d_x += m_Img.getWidth();
            m_Img.draw(in_position, in_clipper);
        }
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

/** Overloaded function to draw the listbox item. */
void FTS::ImagedListItem::draw(CEGUI::RenderCache & in_cache,
                               const CEGUI::Rect & in_targetRect,
                               float in_fZBase, float in_fAlpha,
                               const CEGUI::Rect *in_pClipper) const
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

    CEGUI::ColourRect cr(CEGUI::colour(1.0f, 1.0f, 1.0f, in_fAlpha));

    try {
        // Draw the image and adjust the text's X position.
        if(m_pImageset) {
            rTxt.d_left += m_pImageset->getImage("image").getWidth();
            rImg.d_right = rImg.d_left + m_pImageset->getImage("image").getWidth();
            in_cache.cacheImage(m_pImageset->getImage("image"), rImg, in_fZBase,
                                getModulateAlphaColourRect(cr, in_fAlpha),
                                in_pClipper);
        } else {
            rTxt.d_left += m_Img.getWidth();
            rImg.d_right = rImg.d_left + m_Img.getWidth();
            in_cache.cacheImage(m_Img, rImg, in_fZBase,
                                getModulateAlphaColourRect(cr, in_fAlpha),
                                in_pClipper);
        }
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

 /* EOF */
