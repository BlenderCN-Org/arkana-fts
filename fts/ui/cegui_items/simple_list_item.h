#ifndef D_CEGUIITEMS_H
#define D_CEGUIITEMS_H

#include "main.h"
#include <elements/CEGUIListboxTextItem.h>

namespace CEGUI {
    class String;
    class Listbox;
    class Combobox;
}

#include "dLib/dString/dString.h"

namespace FTS {

const unsigned int MaxListItems = 200;

class SimpleListItem : public CEGUI::ListboxTextItem {
public:
    SimpleListItem(const String & in_text);
    void setAsDefault(CEGUI::Combobox * in_pCB);
    void setAsDefault(CEGUI::Listbox * in_pLB);
    void addAsDefault(CEGUI::Combobox * in_pCB);
    void addAsDefault(CEGUI::Listbox * in_pLB);
    void addAsLast(CEGUI::Listbox * in_pLB);

    CEGUI::Size getPixelSize() const;
    void draw(const CEGUI::Vector3 & in_position,
              float in_fAlpha, const CEGUI::Rect & in_clipper) const;
    void draw(CEGUI::RenderCache & in_cache,
              const CEGUI::Rect & in_targetRect,
              float in_fZBase,
              float in_fAlpha, const CEGUI::Rect * in_pClipper) const;
};

class ServerListItem : public SimpleListItem {
	String m_sDesc;
	String m_sAddr;
    static String calcText(const String &in_sAddr, const String &in_sDesc);

public:
    ServerListItem(const String &in_sAddr, const String &in_sDesc);

	inline String getDesc() const {return m_sDesc;};
	inline String getAddr() const {return m_sAddr;};
    inline String getText() const {return this->calcText(this->getAddr(), this->getDesc());};
};

} // namespace FTS

#endif                          /* D_CEGUIITEMS_H */

 /* EOF */
