#ifndef D_COLOR_LIST_ITEM_H
#define D_COLOR_LIST_ITEM_H

#include "main.h"

#include "graphic/Color.h"

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

class ColorListItem : public CEGUI::ListboxTextItem {
public:
    ColorListItem(const Color& in_col,
                  const CEGUI::String &in_text = "",
                  void *in_pItemData = 0,
                  CEGUI::uint in_uiItemID = 0,
                  bool in_bDisabled = false,
                  bool in_bAutoDelete = true);
    virtual ~ColorListItem();

    CEGUI::Size getPixelSize() const;
    void draw(const CEGUI::Vector3 & in_position, float in_fAlpha, const CEGUI::Rect & in_clipper) const;
    void draw(CEGUI::RenderCache & in_cache, const CEGUI::Rect & in_targetRect, float in_fZBase, float in_fAlpha, const CEGUI::Rect * in_pClipper) const;

    void setAsDefault(CEGUI::Combobox * in_pCB);
    void setAsDefault(CEGUI::Listbox * in_pLB);
    void addAsDefault(CEGUI::Combobox * in_pCB);
    void addAsDefault(CEGUI::Listbox * in_pLB);
private:
    Color m_col;
};

} // namespace FTS

#endif // D_COLOR_LIST_ITEM_H
