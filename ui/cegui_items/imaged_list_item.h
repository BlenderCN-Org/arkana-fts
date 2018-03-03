#ifndef D_CEGUIITEMS_H
#define D_CEGUIITEMS_H

#include "main.h"
#include <CEGUIcolour.h>
#include <elements/CEGUIListboxTextItem.h>

namespace CEGUI {
    class String;
    class Listbox;
    class Combobox;
    class Size;
    class Image;
    class Imageset;
    class Vector3;
    class Rect;
    class RenderCache;
}

namespace FTS {
    class Graphic;

class ImagedListItem : public CEGUI::ListboxTextItem {
public:
    ImagedListItem(const CEGUI::String &in_text,
                   const CEGUI::String &in_sImageset,
                   const CEGUI::String &in_sImage,
                   void *in_pItemData = 0,
                   CEGUI::uint in_uiItemID = 0,
                   bool in_bDisabled = false,
                   bool in_bAutoDelete = true);
    ImagedListItem(const CEGUI::String &in_text,
                   FTS::Graphic *in_pGraphic,
                   void *in_pItemData = 0,
                   CEGUI::uint in_uiItemID = 0,
                   bool in_bDisabled = false,
                   bool in_bAutoDelete = true);
    virtual ~ImagedListItem();

    CEGUI::Size getPixelSize() const;
    void draw(const CEGUI::Vector3 & in_position,
              float in_fAlpha, const CEGUI::Rect & in_clipper) const;
    void draw(CEGUI::RenderCache & in_cache,
              const CEGUI::Rect & in_targetRect,
              float in_fZBase,
              float in_fAlpha, const CEGUI::Rect * in_pClipper) const;

private:
    CEGUI::Imageset *m_pImageset;
    CEGUI::Image m_Img;
};

} // namespace FTS

#endif                          /* D_CEGUIITEMS_H */

 /* EOF */
