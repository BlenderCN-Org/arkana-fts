#include "main.h"
#include <falagard/CEGUIFalWidgetLookManager.h>
#include <falagard/CEGUIFalWidgetLookFeel.h>
#include <elements/CEGUIProgressBar.h>

#include "ui/cegui_items/progressbar/ArkanaProgressBarWR.h"

// Start of CEGUI namespace section
namespace CEGUI
{
    const utf8 ArkanaProgressBarWR::TypeName[] = "Arkana/ProgressBar";
    ArkanaProgressBarWRProperties::VerticalProgress ArkanaProgressBarWR::d_verticalProperty;
    ArkanaProgressBarWRProperties::ReversedProgress ArkanaProgressBarWR::d_reversedProperty;


    ArkanaProgressBarWR::ArkanaProgressBarWR(const String& type) :
        WindowRenderer(type, "ProgressBar"),
        d_vertical(false),
        d_reversed(false)
    {
        registerProperty(&d_verticalProperty);
        registerProperty(&d_reversedProperty);
    }

    void ArkanaProgressBarWR::render()
    {
        const StateImagery* imagery;

        // get WidgetLookFeel for the assigned look.
        const WidgetLookFeel& wlf = getLookNFeel();
        // try and get imagery for our current state
        imagery = &wlf.getStateImagery(d_window->isDisabled() ? "Disabled" : "Enabled");
        // peform the rendering operation.
        imagery->render(*d_window);

        // get imagery for actual progress rendering
        imagery = &wlf.getStateImagery(d_window->isDisabled() ? "DisabledProgress" : "EnabledProgress");

        // get target rect for this imagery
        Rect progressRect(wlf.getNamedArea("ProgressArea").getArea().getPixelRect(*d_window));

        ProgressBar* w = (ProgressBar*)d_window;
        if (d_vertical) {
            float height = progressRect.getHeight() * w->getProgress();

            if (d_reversed) {
                progressRect.setHeight(height);
            } else {
                progressRect.d_top = progressRect.d_bottom - height;
            }
        } else {
            float width = progressRect.getWidth() * w->getProgress();

            if (d_reversed) {
                progressRect.d_left = progressRect.d_right - width;
            } else {
                progressRect.setWidth(width);
            }
        }

        // calculate a clipper according to the current progress.
        Rect progressClipper(progressRect);

        // peform the rendering operation.
        imagery->render(*d_window, progressRect, 0, &progressClipper);
    }

    bool ArkanaProgressBarWR::isVertical() const
    {
        return d_vertical;
    }

    bool ArkanaProgressBarWR::isReversed() const
    {
        return d_reversed;
    }

    void ArkanaProgressBarWR::setVertical(bool setting)
    {
        d_vertical = setting;
    }

    void ArkanaProgressBarWR::setReversed(bool setting)
    {
        d_reversed = setting;
    }

    CEGUI::WindowRenderer* ArkanaProgressBarWRFactory::create( void )
    {
        return new ArkanaProgressBarWR( ArkanaProgressBarWR::TypeName );
    }

    void ArkanaProgressBarWRFactory::destroy( CEGUI::WindowRenderer* wr )
    {
        delete wr;
    }

    ArkanaProgressBarWRFactory& getArkanaProgressBarWRFactory()
    {
        static ArkanaProgressBarWRFactory s_factory;
        return s_factory;
    }
} // End of  CEGUI namespace section
