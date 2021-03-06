/***********************************************************************
    filename:   Sample_ScrollablePane.cpp
    created:    Wed Aug 2 2006
    author:     Tomas Lindquist Olsen
*************************************************************************/
/***************************************************************************
 *   Copyright (C) 2004 - 2006 Paul D Turner & The CEGUI Development Team
 *
 *   Permission is hereby granted, free of charge, to any person obtaining
 *   a copy of this software and associated documentation files (the
 *   "Software"), to deal in the Software without restriction, including
 *   without limitation the rights to use, copy, modify, merge, publish,
 *   distribute, sublicense, and/or sell copies of the Software, and to
 *   permit persons to whom the Software is furnished to do so, subject to
 *   the following conditions:
 *
 *   The above copyright notice and this permission notice shall be
 *   included in all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 *   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 *   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *   OTHER DEALINGS IN THE SOFTWARE.
 ***************************************************************************/
#include "CEGUISystem.h"
#include "CEGUISchemeManager.h"
#include "CEGUIWindowManager.h"
#include "CEGUIFontManager.h"
#include "CEGUIImagesetManager.h"
#include "CEGUIImageset.h"
#include "CEGUIFont.h"
#include "CEGUIWindow.h"
#include "CEGUICoordConverter.h"

#include "elements/CEGUIScrollablePane.h"
#include "elements/CEGUIScrolledContainer.h"

#include "CEGuiSample.h"
#include "CEGuiBaseApplication.h"

/*
This is a demonstration of the ScrollablePane widget
*/

// ScrollablePane demo sample class
class ScrollablePaneSample : public CEGuiSample
{
public:
    // method to initialse the samples windows and events.
    bool initialiseSample();
    // method to perform any required cleanup operations.
    void cleanupSample(void);

private:
    // creates the menubar with content
    void createMenu(CEGUI::Window* bar);

    // quit menu item handler
    bool fileQuit(const CEGUI::EventArgs& e)
    {
        d_sampleApp->setQuitting(true);
        return true;
    }

    // new dialog menu item handler
    bool demoNewDialog(const CEGUI::EventArgs& e);

    // handler for all the global hotkeys
    bool hotkeysHandler(const CEGUI::EventArgs& e);

    // member data
    CEGUI::WindowManager* d_wm; // we will use the window manager alot
    CEGUI::System* d_system;    // the gui system
    CEGUI::Window* d_root;      // the gui sheet
    CEGUI::Font* d_font;        // the font we use
    CEGUI::ScrollablePane* d_pane; // the scrollable pane. center piece of the demo
};

// Sample program entry point
int main(int argc, char *argv[])
{
    // This is a basic start-up for the sample application which is
    // object orientated in nature, so we just need an instance of
    // the CEGuiSample based object and then tell that sample application
    // to run.  All of the samples will use code similar to this in the
    // main/WinMain function.
    ScrollablePaneSample app;
    return app.run();
}

/*************************************************************************
    Sample specific initialisation goes here.
*************************************************************************/
bool ScrollablePaneSample::initialiseSample()
{
    using namespace CEGUI;

    // this sample will use WindowsLook
    SchemeManager::getSingleton().loadScheme("WindowsLook.scheme");

    // load the default font
    d_font = FontManager::getSingleton().createFont("DejaVuSans-10.font");

    // to look more like a real application, we override the autoscale setting
    // for both skin and font
    Imageset* wndlook = ImagesetManager::getSingleton().getImageset("WindowsLook");
    wndlook->setAutoScalingEnabled(false);
    d_font->setProperty("AutoScaled", "false");

    // set the mouse cursor
    d_system = System::getSingletonPtr();
    d_system->setDefaultMouseCursor(&wndlook->getImage("MouseArrow"));

    // set the default tooltip type
    d_system->setDefaultTooltip("WindowsLook/Tooltip");

    // We need the window manager to set up the test interface :)
    d_wm = WindowManager::getSingletonPtr();

    // create a root window
    // this will be a static, to give a nice app'ish background
    d_root = d_wm->createWindow("WindowsLook/Static");
    d_root->setProperty("FrameEnabled", "false");
    // root window will take care of hotkeys
    d_root->subscribeEvent(Window::EventKeyDown, Event::Subscriber(&ScrollablePaneSample::hotkeysHandler, this));
    d_system->setGUISheet(d_root);

    // create a menubar.
    // this will fit in the top of the screen and have options for the demo
    UDim bar_bottom(0,d_font->getLineSpacing(2));

    Window* bar = d_wm->createWindow("WindowsLook/Menubar");
    bar->setArea(UDim(0,0),UDim(0,0),UDim(1,0),bar_bottom);
    bar->setAlwaysOnTop(true); // we want the menu on top
    d_root->addChildWindow(bar);

    // fill out the menubar
    createMenu(bar);

    // create a scrollable pane for our demo content
    d_pane = static_cast<ScrollablePane*>(d_wm->createWindow("WindowsLook/ScrollablePane"));
    d_pane->setArea(URect(UDim(0,0),bar_bottom,UDim(1,0),UDim(1,0)));
    // this scrollable pane will be a kind of virtual desktop in the sense that it's bigger than
    // the screen. 3000 x 3000 pixels
    d_pane->setContentPaneAutoSized(false);
    d_pane->setContentPaneArea(CEGUI::Rect(0,0,3000,3000));
    d_root->addChildWindow(d_pane);

    // add a dialog to this pane so we have something to drag around :)
    Window* dlg = d_wm->createWindow("WindowsLook/FrameWindow");
    dlg->setMinSize(UVector2(UDim(0,250),UDim(0,100)));
    dlg->setText("Drag me around");
    d_pane->addChildWindow(dlg);

    return true;
}

/*************************************************************************
    Creates the menu bar and fills it up :)
*************************************************************************/
void ScrollablePaneSample::createMenu(CEGUI::Window* bar)
{
    using namespace CEGUI;
    
    // file menu item
    Window* file = d_wm->createWindow("WindowsLook/MenuItem");
    file->setText("File");
    bar->addChildWindow(file);
    
    // file popup
    Window* popup = d_wm->createWindow("WindowsLook/PopupMenu");
    file->addChildWindow(popup);
    
    // quit item in file menu
    Window* item = d_wm->createWindow("WindowsLook/MenuItem");
    item->setText("Quit");
    item->subscribeEvent("Clicked", Event::Subscriber(&ScrollablePaneSample::fileQuit, this));
    popup->addChildWindow(item);

    // demo menu item
    Window* demo = d_wm->createWindow("WindowsLook/MenuItem");
    demo->setText("Demo");
    bar->addChildWindow(demo);

    // demo popup
    popup = d_wm->createWindow("WindowsLook/PopupMenu");
    demo->addChildWindow(popup);

    // demo -> new window
    item = d_wm->createWindow("WindowsLook/MenuItem");
    item->setText("New dialog");
    item->setTooltipText("Hotkey: Space");
    item->subscribeEvent("Clicked", Event::Subscriber(&ScrollablePaneSample::demoNewDialog, this));
    popup->addChildWindow(item);
}

/*************************************************************************
    Cleans up resources allocated in the initialiseSample call.
*************************************************************************/
void ScrollablePaneSample::cleanupSample()
{
    // everything we did is cleaned up by CEGUI
}

/*************************************************************************
    Handler for the 'Demo -> New dialog' menu item
*************************************************************************/
bool ScrollablePaneSample::demoNewDialog(const CEGUI::EventArgs& e)
{
    using namespace CEGUI;
    
    // add a dialog to this pane so we have some more stuff to drag around :)
    Window* dlg = d_wm->createWindow("WindowsLook/FrameWindow");
    dlg->setMinSize(UVector2(UDim(0,200),UDim(0,100)));
    dlg->setText("Drag me around too!");
    
    // we put this in the center of the viewport into the scrollable pane
    UVector2 uni_center(UDim(0.5f,0), UDim(0.5f,0));
    Vector2 center = CoordConverter::windowToScreen(*d_root, uni_center);
    Vector2 target = CoordConverter::screenToWindow(*d_pane->getContentPane(), center);
    dlg->setPosition(UVector2(UDim(0,target.d_x-100), UDim(0,target.d_y-50)));
    
    d_pane->addChildWindow(dlg);
    
    return true;
}

/*************************************************************************
    Handler for global hotkeys
*************************************************************************/
bool ScrollablePaneSample::hotkeysHandler(const CEGUI::EventArgs& e)
{
    using namespace CEGUI;
    const KeyEventArgs& k = static_cast<const KeyEventArgs&>(e);

    switch (k.scancode)
    {
    // space is a hotkey for demo -> new dialog
    case Key::Space:
        // this handler does not use the event args at all so this is fine :)
        // though maybe a bit hackish...
        demoNewDialog(e);
        break;

    // no hotkey found? event not used...
    default:
        return false;
    }

    return true;
}
