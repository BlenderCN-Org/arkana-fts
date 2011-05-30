#include "setup.h"

#define WIDTH	640
#define HEIGHT	480

typedef struct _SServerInfo_ {
    std::string sAddr;
    std::string sDir;
    std::string sLogin;
    std::string sPass;
} SServerInfo, *PServerInfo;

void exit_err(int i)
{
#if WINDOOF
    system("pause");
#endif
    exit(i);
}

void init(bool bAdminMode)
{
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        vperror("Unable to initialise SDL: %s", SDL_GetError());
        exit_err(1);
    }

    if(SDL_SetVideoMode(WIDTH, HEIGHT, 0, SDL_OPENGL) == NULL) {
        vperror("Unable to set OpenGL videomode: %s", SDL_GetError());
        SDL_Quit();
        exit_err(1);
    }

    SDL_ShowCursor(SDL_DISABLE);
    SDL_EnableUNICODE(1);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
                        SDL_DEFAULT_REPEAT_INTERVAL);
    SDL_WM_SetCaption("FTS Updater v1.1", NULL);

    glEnable(GL_CULL_FACE);
    glDisable(GL_FOG);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, WIDTH, HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)WIDTH / (float)HEIGHT, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Load up CEGUI.
    try {
        CEGUI::OpenGLRenderer * renderer =
            new CEGUI::OpenGLRenderer(0, WIDTH, HEIGHT);
        new CEGUI::System(renderer);

        // initialise the required dirs for the DefaultResourceProvider
        CEGUI::DefaultResourceProvider * rp =
            static_cast <
            CEGUI::DefaultResourceProvider *
            >(CEGUI::System::getSingleton().getResourceProvider());

        rp->setResourceGroupDirectory("imagesets", UI_IMAGESETS);
        rp->setResourceGroupDirectory("fonts", UI_FONTS);
        rp->setResourceGroupDirectory("layouts", UI_LAYOUTS);
        rp->setResourceGroupDirectory("looknfeels", UI_LOOKNFEEL);
        rp->setResourceGroupDirectory("lua_scripts", UI_LUA);
        rp->setResourceGroupDirectory("schemes", UI_SCHEMES);

        // set the default resource groups to be used
        CEGUI::Imageset::setDefaultResourceGroup("imagesets");
        CEGUI::Font::setDefaultResourceGroup("fonts");
        CEGUI::WindowManager::setDefaultResourceGroup("layouts");
        CEGUI::WidgetLookManager::setDefaultResourceGroup("looknfeels");
        CEGUI::ScriptModule::setDefaultResourceGroup("lua_scripts");
        CEGUI::Scheme::setDefaultResourceGroup("schemes");

        // load in the scheme file, which auto-loads the TaharezLook imageset
        CEGUI::SchemeManager::getSingleton().loadScheme(FTS_UI_SKIN_FILE);

        // load in a font. The first font loaded automatically becomes the default font.
        CEGUI::FontManager::getSingleton().createFont("verdana.font");

        // Set the default cursor and font.
        CEGUI::System::getSingleton().setDefaultMouseCursor(FTS_UI_SKIN,
                                                            "MouseArrow");
        CEGUI::System::getSingleton().setDefaultFont("verdana");

        // Create the root window.
        g_rootWin =
            CEGUI::WindowManager::getSingleton().
            createWindow("DefaultWindow", "root");
        CEGUI::System::getSingleton().setGUISheet(g_rootWin);

        // Load the Main menu
        Window *w =
            CEGUI::WindowManager::getSingleton().
            loadWindowLayout("fts_updater.layout");
        w->setVisible(true);
        g_rootWin->addChildWindow(w);

        if(!bAdminMode) {
            // Disable the make CRC list button.
            w->getChild("btnMkList")->setEnabled(false);
        }
        // Set the version info string.
        char *pszTmp =
            MyAllocSPrintf("%d.%d.%d.%d", g_version[0], g_version[1],
                           g_version[2], g_version[3]);

        w->getChild("curr_ver")->setText(pszTmp);
        free(pszTmp);

        // Fill the mirrors listbox.
        std::ifstream file("v.i5");
        if(!file) {
            vperror("ERROR: The mirrors file v.i5 is missing !\n");
            vperror("You can get one on the FTS Homepage.\n");

            PushButton *pb1 =
                static_cast < PushButton * >(w->getChild("btnUpdate"));
            PushButton *pb2 =
                static_cast < PushButton * >(w->getChild("btnCheck"));
            pb1->setEnabled(false);
            pb2->setEnabled(false);
        }

        while(file) {
            PServerInfo psi = new SServerInfo;

            file >> psi->sAddr;
            file >> psi->sDir;
            file >> psi->sLogin;
            file >> psi->sPass;

            if(psi->sAddr.empty())
                continue;

            ListboxTextItem *lti = new ListboxTextItem(psi->sAddr);

            lti->setSelectionBrushImage("TaharezLook",
                                        "MultiListSelectionBrush");
            lti->setUserData(psi);
            ((Combobox *) w->getChild("Mirrors"))->addItem(lti);
            ((Combobox *) w->getChild("Mirrors"))->setItemSelectState(lti,
                                                                      true);
            ((Combobox *) w->getChild("Mirrors"))->setText(lti->getText());
            lti->setSelected(true);
        }

    }
    catch(CEGUI::UnknownObjectException & ex) {
        vperror("CEGUI_Init: %s\n", ex.getMessage().c_str());
        exit_err(1);
    }
    catch(CEGUI::Exception & e) {
        vperror("CEGUI_Load: %s", e.getMessage().c_str());
        exit_err(1);
    }

    // Init FTP.
    FtpInit();
}

bool connect_to_server(void)
{
    PServerInfo psi = NULL;

    if(g_connection != NULL)
        return true;

    // Get the selected mirror.
    try {
        CEGUI::Combobox * pCB =
            static_cast < CEGUI::Combobox * >(g_rootWin->getChild("vers")
                                              ->getChild("Mirrors"));
        CEGUI::ListboxTextItem * pLTI =
            static_cast <
            CEGUI::ListboxTextItem * >(pCB->getSelectedItem());
        psi = (PServerInfo) pLTI->getUserData();
    }
    catch(CEGUI::Exception & e) {
        vperror("CEGUI: %s", e.getMessage().c_str());
        return false;
    }

    // When the username or password is a 'x', it means none is required.
    if(psi->sLogin == "x")
        psi->sLogin = "";
    if(psi->sPass == "x")
        psi->sPass = "";

    // Say something.
    vprint("Connecting to %s ... ", psi->sAddr.c_str());

    // Connect to the server ^^
    if(FtpConnect(psi->sAddr.c_str(), &g_connection) == 0) {
        vperror("could not connect to host '%s': '%s'\n",
                psi->sAddr.c_str(), FtpLastResponse(g_connection));
        return false;
    }
    // Login to the remote server.
    if(FtpLogin(psi->sLogin.c_str(), psi->sPass.c_str(), g_connection) ==
       0) {
        vperror("could not connect user '%s': '%s'\n", psi->sLogin.c_str(),
                FtpLastResponse(g_connection));
        FtpClose(g_connection);
        g_connection = NULL;
        return false;
    }
    // Maybe go to the FTS directory.
    if(psi->sDir != "x") {
        if(FtpChdir(psi->sDir.c_str(), g_connection) == 0) {
            vperror("could not change to directory '%s': '%s'\n",
                    psi->sDir.c_str(), FtpLastResponse(g_connection));
            FtpClose(g_connection);
            g_connection = NULL;
            return false;
        }
    }
    // Say something again.
    vprint("connected !\n");

    return true;
}

void inject_time_pulse(double &last_time_pulse)
{
    // get current "run-time" in seconds
    double t = 0.001 * dGetTicks();

    // inject the time that passed since the last call
    CEGUI::System::getSingleton().
        injectTimePulse(float (t - last_time_pulse));

    // store the new time as the last time
    last_time_pulse = t;
}

void inject_input(bool & must_quit)
{
    SDL_Event e;

    // go through all available events
    while(SDL_PollEvent(&e)) {
        // we use a switch to determine the event type
        switch (e.type) {
            // mouse motion handler
        case SDL_MOUSEMOTION:
            // we inject the mouse position directly.
            CEGUI::System::getSingleton().injectMousePosition((float)e.
                                                              motion.x,
                                                              (float)e.
                                                              motion.y);
            break;

            // mouse down handler
        case SDL_MOUSEBUTTONDOWN:
            // let a special function handle the mouse button down event
            handle_mouse_down(e.button.button);
            break;

            // mouse up handler
        case SDL_MOUSEBUTTONUP:
            // let a special function handle the mouse button up event
            handle_mouse_up(e.button.button);
            break;

            // key down
        case SDL_KEYDOWN:
            // to tell CEGUI that a key was pressed, we inject the scancode.
            CEGUI::System::getSingleton().injectKeyDown(e.key.keysym.
                                                        scancode);

            // as for the character it's a litte more complicated. we'll use for translated unicode value.
            // this is described in more detail below.
            if((e.key.keysym.unicode & 0xFF80) == 0) {
                CEGUI::System::getSingleton().injectChar(e.key.keysym.
                                                         unicode & 0x7F);
            }
            break;

            // key up
        case SDL_KEYUP:
            // like before we inject the scancode directly.
            CEGUI::System::getSingleton().injectKeyUp(e.key.keysym.
                                                      scancode);
            break;

            // WM quit event occured
        case SDL_QUIT:
            must_quit = true;
            break;
        }

    }
}

void handle_mouse_down(Uint8 button)
{
    switch (button) {
        // handle real mouse buttons
    case SDL_BUTTON_LEFT:
        CEGUI::System::getSingleton().
            injectMouseButtonDown(CEGUI::LeftButton);
        break;
    case SDL_BUTTON_MIDDLE:
        CEGUI::System::getSingleton().
            injectMouseButtonDown(CEGUI::MiddleButton);
        break;
    case SDL_BUTTON_RIGHT:
        CEGUI::System::getSingleton().
            injectMouseButtonDown(CEGUI::RightButton);
        break;
        // handle the mouse wheel
    case SDL_BUTTON_WHEELDOWN:
        CEGUI::System::getSingleton().injectMouseWheelChange(-1);
        break;
    case SDL_BUTTON_WHEELUP:
        CEGUI::System::getSingleton().injectMouseWheelChange(+1);
        break;
    }
}

void handle_mouse_up(Uint8 button)
{
    switch (button) {
    case SDL_BUTTON_LEFT:
        CEGUI::System::getSingleton().
            injectMouseButtonUp(CEGUI::LeftButton);
        break;
    case SDL_BUTTON_MIDDLE:
        CEGUI::System::getSingleton().
            injectMouseButtonUp(CEGUI::MiddleButton);
        break;
    case SDL_BUTTON_RIGHT:
        CEGUI::System::getSingleton().
            injectMouseButtonUp(CEGUI::RightButton);
        break;
    }
}


void render_gui(void)
{
    // clear the colour buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // render the GUI :)
    CEGUI::System::getSingleton().renderGUI();

    // Update the screen
    SDL_GL_SwapBuffers();
}
