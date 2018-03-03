#include "main.h"
#include <CEGUIPropertyHelper.h>

#include "ui/cegui_items/progressbar/ArkanaProgressBarWRProperties.h"
#include "ui/cegui_items/progressbar/ArkanaProgressBarWR.h"

// Start of CEGUI namespace section
namespace CEGUI
{

namespace ArkanaProgressBarWRProperties
{
    String VerticalProgress::get(const PropertyReceiver* receiver) const
    {
        ArkanaProgressBarWR* pb = static_cast<ArkanaProgressBarWR*>(
            static_cast<const Window*>(receiver)->getWindowRenderer());
        return PropertyHelper::boolToString(pb->isVertical());
    }

    void VerticalProgress::set(PropertyReceiver* receiver, const String& value)
    {
        ArkanaProgressBarWR* pb = static_cast<ArkanaProgressBarWR*>(
            static_cast<Window*>(receiver)->getWindowRenderer());
        pb->setVertical(PropertyHelper::stringToBool(value));
    }

    String ReversedProgress::get(const PropertyReceiver* receiver) const
    {
        ArkanaProgressBarWR* pb = static_cast<ArkanaProgressBarWR*>(
            static_cast<const Window*>(receiver)->getWindowRenderer());
        return PropertyHelper::boolToString(pb->isReversed());
    }

    void ReversedProgress::set(PropertyReceiver* receiver, const String& value)
    {
        ArkanaProgressBarWR* pb = static_cast<ArkanaProgressBarWR*>(
            static_cast<Window*>(receiver)->getWindowRenderer());
        pb->setReversed(PropertyHelper::stringToBool(value));
    }
}

} // End of  CEGUI namespace section
