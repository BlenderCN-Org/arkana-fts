/**
 * \file game_rlv.h
 * \author Pompei2
 * \date 9 December 2008
 * \brief This file implements the game's main runlevel.
 **/

#include "game/loadgame_rlv.h"

#include <CEGUIWindow.h> // For destroying the windows.
#include <CEGUIWindowManager.h> // For destroying the windows.
#include <CEGUIExceptions.h> // For destroying the windows.
#include <CEGUISystem.h> // For destroying the windows.
#include <CEGUIImagesetManager.h> // For the loadscreen image destruction.
#include <elements/CEGUIProgressBar.h> // For the progressbar.

#include "game/game_rlv.h"
#include "graphic/graphic.h" // For the ICP and RB.
#include "ui/ui.h" // For loadlayout.
#include "ui/ui_menu.h" // Go back there if some important thing failed.
#include "dLib/dArchive/dArchive.h"
#include "dLib/dConf/configuration.h"
#include "logging/logger.h"
#include "logging/ftslogger.h" // To suppress all dlgs while in loadscreen.
#include "logging/Chronometer.h"
#include "map/map.h"
#include "map/mapinfo.h"
#include "scripting/DaoVm.h"
#include "input/input.h"
#include "sound/fts_Snd.h"

using namespace FTS;

LoadGameRlv::LoadGameRlv(const Path &in_sMapFile, uint8_t in_nPlayers)
    : m_loadState(new StateLoadBeginning())
    , m_fPercentDone(0.0f)
    , m_pRootWindow(nullptr)
    , m_pGame(new GameRlv)
    , m_sFile(in_sMapFile)
    , m_nPlayers(in_nPlayers)
    , m_uiBeginTime(0)
    , m_pTerrainLoadingInfo(nullptr)
    , m_bWaiting(false)
{
}

LoadGameRlv::~LoadGameRlv()
{
    SAFE_DELETE( m_loadState );
}

/** This method will be called during the loading of this runlevel.\n
 *  Please note that this method does NOT load the map, that is done within the
 *  tick method.\n
 *
 *  This load method rather loads the loadscreen and sets things up.
 *
 *  \return This method should return true only if it successfully loaded
 *          the whole runlevel. If it returns false, the previous runlevel
 *          will be backed up again.
 *  \author Pompei2
 */
bool LoadGameRlv::load()
{
    FTSMSGDBG("Preloading map from file \"" + m_sFile + "\".", 3);

    DefaultLogger *pLog = dynamic_cast<DefaultLogger *>(Logger::getSingletonPtr());

    try {
        pLog->suppressAllDlgs();

        // First extract and load all that is needed to show the loadscreen.
        Archive *a = Archive::loadArchive(m_sFile);

        m_pGame->giveMapArchive(a);
        File::addArchiveToLook(a);

        // If the loading doesn't work, we'll have a black background that's all.
        GraphicManager::getSingleton().getOrLoadGraphic(Path::datadir("Graphics/loadscreen.png"))
                                     ->createCEGUI("loadscreen", true);
        GraphicManager::getSingleton().destroyGraphic(Path::datadir("Graphics/loadscreen.png"));

        // And setup the GUI.
        if(!(m_pRootWindow = GUI::getSingleton().loadLayout("loadscreen")))
            return false;

        // Disable the tooltip.
        try {
            CEGUI::System::getSingleton().setDefaultTooltip("");
        } catch(CEGUI::Exception &) {
        }

        this->setupLoadscreenPlayers();
        ISndSys::getSingleton().pushContext();

    } catch(const ArkanaException& e) {
        // Corrupt map, stop loading, back to main menu.
        pLog->stopSuppressingDlgs();
        e.show();
        return false;
    }

    return true;
}

/** This method will be called during the cleaning of the runlevel (when quitting).\n
 *  Like the load method, this method does NOT unload the map etc. It rather
 *  unloads the loadscreen again.
 *
 *  \return This method should return true if it successfully unloaded
 *          the whole runlevel. If it returns false, nothing special is done
 *          (the runlevel is still unloaded).
 *  \author Pompei2
 */
bool LoadGameRlv::unload()
{
    SAFE_DELETE(m_pTerrainLoadingInfo);

    // On a successful load, the game should be NULL so that nothing is done
    // here. But on a failure, the game is not NULL and will be deleted.
    SAFE_DELETE(m_pGame);

    // Two separate try/catch blocks to still do the second even if the first fails.
    try {
        CEGUI::ImagesetManager::getSingleton().destroyImageset("loadscreen");
    } catch(CEGUI::Exception &) {
    }

    try {
        if(m_pRootWindow != nullptr)
            CEGUI::WindowManager::getSingleton().destroyWindow(m_pRootWindow);

        // Set back the tooltip.
        CEGUI::System::getSingleton().setDefaultTooltip("ArkanaLook/Tooltip");
    } catch(CEGUI::Exception &) {
    }

    DefaultLogger *pLog = dynamic_cast<DefaultLogger *>(Logger::getSingletonPtr());
    pLog->stopSuppressingDlgs();

    return true;
}

/// This sets up the other players in the loadscreen.
/** The loadscreen must already be loaded for this method to do something.\n
 *  It creates the progressbars for the other users in the loadscreen.
 *
 * \return this
 *
 * \author Pompei2
 */
void LoadGameRlv::setupLoadscreenPlayers()
{
    CEGUI::WindowManager *pWM = nullptr;
    CEGUI::Window *wContainer = nullptr;

    float fCurrY = 0.0f;   // The y position of the current player's progressbar.
    float fHeight = 34.0f; // The height of one element (Label and progressbar).

    try {
        pWM = CEGUI::WindowManager::getSingletonPtr();
        if(pWM == nullptr)
            return;

        wContainer = pWM->getWindow("loadscreen/progs");

        // We don't want to detect the height automatically anymore because it
        // looks better with fixed height.
        // fHeight = wParent->getPixelSize().d_height / (float)nPlayers;
    } catch(CEGUI::Exception &) {
        // If there is no container for the progressbars, don't create them.
        return;
    }

    for(int i = 0 ; i < m_nPlayers ; i++) {
        String sPlayer = getTranslation("General_Player");
        String sPlayerName = String("{1} {2}").fmt(sPlayer, String::nr(i));
        String sProgressName = "loadscreen/progs/pg" + sPlayerName;
        String sLabelName = "loadscreen/progs/lbl" + sPlayerName;

        try {
            // Create the progressbar with all its settings.
            float fProgY = fCurrY + ((fHeight-24.0f) / 2.0f);
            CEGUI::Window *wProg = pWM->createWindow("ArkanaLook/Progressbar", sProgressName);
            wProg->setPosition(CEGUI::UVector2(CEGUI::UDim(0.0f,0.0f), CEGUI::UDim(0.0f,fProgY)));
            wProg->setSize(CEGUI::UVector2(CEGUI::UDim(0.0f,124.0f), CEGUI::UDim(0.0f,24.0f)));

            // Create the label with the player name that comes into the progressbars.
            CEGUI::Window *wLbl = pWM->createWindow("ArkanaLook/ShadowedLabel", sLabelName);
            wLbl->setPosition(CEGUI::UVector2(CEGUI::UDim(0.0f,130.0f), CEGUI::UDim(0.0f,fCurrY)));
            wLbl->setSize(CEGUI::UVector2(CEGUI::UDim(1.0f,-130.0f), CEGUI::UDim(0.0f,fHeight)));
            wLbl->setText(sPlayerName);

            wContainer->addChildWindow(wProg);
            wContainer->addChildWindow(wLbl);
        } catch(CEGUI::Exception &) {
            // On errors, just go to the next player.
        }

        // Go under the progressbar.
        fCurrY += fHeight;
    }
}

/// This sets up the loadscreen's detailed informations.
/** The loadscreen and the MapInfo must already be loaded for this method to
 *  work. It sets the labels to the corresponding map infos on the loadscreen.
 *
 * \param in_pDetails The infos about the map.
 *
 * \author Pompei2
 */
void LoadGameRlv::setupLoadscreenDetails(MapInfo *in_pDetails)
{
    if(in_pDetails == nullptr)
        return ;

    CEGUI::WindowManager *pWM = CEGUI::WindowManager::getSingletonPtr();
    if(pWM == nullptr) {
        return ;
    }

    // Set the title of the map.
    try {
        if(pWM->isWindowPresent("loadscreen/lblTitle")) {
            pWM->getWindow("loadscreen/lblTitle")
               ->setText(in_pDetails->getName());
        }
    } catch(CEGUI::Exception &) { }

    // Set the Author of the map.
    try {
        if(pWM->isWindowPresent("loadscreen/lblAuthor")) {
            String sFmt = getTranslation("Loadscr_Author");
            String sAuthor = sFmt.fmt(in_pDetails->getAuthor());
            pWM->getWindow("loadscreen/lblAuthor")->setText(sAuthor);
        }
    } catch(CEGUI::Exception &) { }

    // Set the map's last modification date.
    try {
        if(pWM->isWindowPresent("loadscreen/lblModif")) {
            String sFmt = getTranslation("Loadscr_Lastmodif");
            String sModif = sFmt.fmt(in_pDetails->getLastModif().toStr());
            pWM->getWindow("loadscreen/lblModif")->setText(sModif);
        }
    } catch(CEGUI::Exception &) { }

    // Set the map's description.
    try {
        if(pWM->isWindowPresent("loadscreen/lblDescription")) {
            pWM->getWindow("loadscreen/lblDescription")
               ->setText(in_pDetails->getDesc());
        }
    } catch(CEGUI::Exception &) { }
}

/// Adds a bit to the main progressbar.
/** The loadscreen must already be loaded for this method to do something.\n
 *  It updates the progress of the current player (me), that is the progressbar
 *  and different info texts that may be displayed.
 *
 * \param in_fPercentForStage How many percent to add to the current stage's
 *                            progress, in the range [0.0 ; 1.0].
 * \param in_sDetail Some detailed textual info about what will be loaded next.
 *
 * \note if too many percents get added for the current stage (that means the
 *       total percent of the current stage would get > 1.0), the percent that
 *       are too much get discarded.
 * \note if the detail is String::EMPTY, the previous detail is kept.
 *
 * \author Pompei2
 */
void LoadGameRlv::addToProgress(float in_fPercentForStage, const String &in_sDetail)
{

    // Every stage may have a different weight.

    // Now, get the amount of % that are already done in the current stage.
    float fPercentDoneInCurrStage = 0.0f;
    try {
        FTSGetConvertWinMacro(CEGUI::ProgressBar, pPB, "loadscreen/pgMe");
        fPercentDoneInCurrStage = pPB->getProgress() - m_fPercentDone;
    } catch(...) { }

    // Calculate the amount of % the current stage should have after the update.
    float fPercentForCurrStage = fPercentDoneInCurrStage + in_fPercentForStage * m_loadState->getStatePercentage();
    if(fPercentForCurrStage > m_loadState->getStatePercentage()) {
        fPercentForCurrStage = m_loadState->getStatePercentage();
    }

    // And this is finally the current position of the progressbar.
    float fNow = m_fPercentDone + fPercentForCurrStage;

    try {
        FTSGetConvertWinMacro(CEGUI::ProgressBar, pPB, "loadscreen/pgMe");
        pPB->setProgress(fNow);
    } catch(...) { }

    if(!in_sDetail.empty()) {
        try {
            CEGUI::WindowManager::getSingleton()
                .getWindow("loadscreen/lblState")->setText(in_sDetail);
        } catch(...) { }
    }
    // Calculate how much % are already done with the states that are already done.
    m_fPercentDone += m_loadState->getStatePercentage();
    assert(m_fPercentDone < 1.0001f);
}

/** This method will be called once every frame, after the 3D rendering is done
 *  and all matrices are setup to render in two dimensions now.\n
 *  Thus you should use the commands glVertex2i(x,y) to draw something, x
 *  and y being screen-space pixels (from 0 to w, 0 to h).\n
 *
 *  Actually, this only draws the gui.
 *
 *  \author Pompei2
 */
void LoadGameRlv::render2D(const Clock&)
{
    // Try to render the whole GUI.
    try {
        CEGUI::System::getSingletonPtr()->renderGUI();
    } catch(CEGUI::Exception &) { }
}

/** This method needs to be overloaded. It has to return a unique name for
 *  the runlevel.
 *
 *  \author Pompei2
 */
String LoadGameRlv::getName()
{
    return String("Load Game");
}

/** This method will be called once every game tick, that is usually once every
 *  frame, right before the rendering is set up and done.\n
 *  This method will do the actual loading. It will always setup something, save
 *  its current state then return, letting FTS refresh the screen
 *  (progressbar and stuff), continuing its loading work next time it's called.
 *
 *  \author Pompei2
 */
bool LoadGameRlv::update(const Clock&)
{
    m_loadState->doLoad(this);
    return true;
}

bool LoadGameRlv::cbAnyKeyPressed(const CEGUI::EventArgs &)
{
    // Unregister the press any key callback
    if(m_bWaiting) {
        InputManager::getSingleton().delShortcut("blah_load_done_bleh");
        InputManager::getSingleton().delShortcut("blah_load_done_bleh2");

        // Enter the game as soon as a key is pressed.
        RunlevelManager::getSingleton().prepareRunlevelEntrance(m_pGame);
        m_pGame = nullptr;
        m_bWaiting = false;
    }

    return true;
}

void LoadGameRlv::finishStage(const String &in_sProgress, const String &in_sStat)
{
    static Chronometer chron;
    double dSecElapsed = chron.reset();

    this->addToProgress(1.0f, in_sProgress);

    if(!in_sStat.empty()) {
        try {
            CEGUI::String sWin = ("loadscreen/" + in_sStat);
            CEGUI::Window *pWin=CEGUI::WindowManager::getSingleton().getWindow(sWin);
            String sTxt = getTranslation(in_sStat);
            pWin->setText(String("{1} {2} s").fmt(sTxt, String::nr(dSecElapsed, 2)));
            pWin->show();
        } catch(...) {}
    }
}

void FTS::LoadGameRlv::setState( ILoadGameState* state )
{
    delete m_loadState;
    m_loadState = state;
}

void FTS::LoadGameRlv::StateLoadBeginning::doLoad( LoadGameRlv * context )
{
    // Do nothing in this stage, only update the screen.

    // This stage is done, show the user what will be done in the next one.
    context->finishStage(context->getTranslation("Loadscr_Stage_GetFL"));
    context->setState(new StateLoadMapInfo());
}

void FTS::LoadGameRlv::StateLoadMapInfo::doLoad( LoadGameRlv * context )
{
    context->m_pGame->getMap()->getInfo()->load();
    context->setupLoadscreenDetails(context->m_pGame->getMap()->getInfo());

    // This stage is done, show the user what will be done in the next one.
    String sTxt = context->getTranslation("Loadscr_Stage_TerrInfo");
    context->finishStage(sTxt);
    context->setState(new StateLoadTerrainInfo());
}

void FTS::LoadGameRlv::StateLoadTerrainInfo::doLoad( LoadGameRlv * context )
{
    // This stage loads the various informations about the terrain.
    context->m_pTerrainLoadingInfo = new Terrain::SLoadingInfo(context->m_pGame->getMap()
        ->getInfo()
        ->getName());
    Terrain *pT = context->m_pGame->getMap()->m_pTerrain;
    if(ERR_OK != pT->loadInfo("terrain.ftst", *context->m_pTerrainLoadingInfo)) {
        // On an error, we go back to the main menu.
        RunlevelManager::getSingleton().prepareRunlevelEntrance(new MainMenuRlv);
    }

    // This stage is done, show the user what will be done in the next one.
    int nQuads = context->m_pGame->getMap()->m_pTerrain->getW() *
        context->m_pGame->getMap()->m_pTerrain->getH();
    String sFmt =context->getTranslation("Loadscr_Stage_TerrQuads");
    context->finishStage(sFmt.fmt(String::nr(nQuads)));
    context->setState(new StateLoadTerrainQuads());

}

void FTS::LoadGameRlv::StateLoadTerrainQuads::doLoad( LoadGameRlv * context )
{
    // This stage loads the quads of the terrain.
    Terrain *pT = context->m_pGame->getMap()->m_pTerrain;
    if(ERR_OK != pT->loadQuads(*context->m_pTerrainLoadingInfo)) {
        // On an error, we go back to the main menu.
        RunlevelManager::getSingleton().prepareRunlevelEntrance(new MainMenuRlv);
    }

    // This stage is done, show the user what will be done in the next one.
    int nTiles = (context->m_pGame->getMap()->m_pTerrain->getW()+1) *
        (context->m_pGame->getMap()->m_pTerrain->getH()+1);
    String sFmt = context->getTranslation("Loadscr_Stage_TerrLoTi");
    context->finishStage(sFmt.fmt(String::nr(nTiles)), "lblTime_quads");
    context->setState(new StateLoadTerrainLoadLowerTiles());

}

void FTS::LoadGameRlv::StateLoadTerrainLoadLowerTiles::doLoad( LoadGameRlv * context )
{
    // This stage loads the lower tiles of the terrain.
    Terrain *pT = context->m_pGame->getMap()->m_pTerrain;
    if(ERR_OK != pT->loadLowerTiles(*context->m_pTerrainLoadingInfo)) {
        // On an error, we go back to the main menu.
        RunlevelManager::getSingleton().prepareRunlevelEntrance(new MainMenuRlv);
    }

    // This stage is done, show the user what will be done in the next one.
    String sTxt = context->getTranslation("Loadscr_Stage_TerrCompLoTi");
    context->finishStage(sTxt, "lblTime_lowertiles");
    context->setState(new StateLoadTerrainCompileLowerTileset());
}

void FTS::LoadGameRlv::StateLoadTerrainCompileLowerTileset::doLoad( LoadGameRlv * context )
{
    // This stage loads compiles the lower tileset of the terrain.
    Terrain *pT = context->m_pGame->getMap()->m_pTerrain;
    if(ERR_OK != pT->compileLowerTiles(*context->m_pTerrainLoadingInfo)) {
        // On an error, we go back to the main menu.
        RunlevelManager::getSingleton().prepareRunlevelEntrance(new MainMenuRlv);
    }

    // This stage is done, show the user what will be done in the next one.
    int nTiles = (context->m_pGame->getMap()->m_pTerrain->getW()+1) *
        (context->m_pGame->getMap()->m_pTerrain->getH()+1);
    String sFmt = context->getTranslation("Loadscr_Stage_TerrUpTi");
    context->finishStage(sFmt.fmt(String::nr(nTiles)), "lblTime_complowtil");
    context->setState(new StateLoadTerrainUpperTiles());
}

void FTS::LoadGameRlv::StateLoadTerrainUpperTiles::doLoad( LoadGameRlv * context )
{
    // This stage loads the upper tiles of the terrain.
    Terrain *pT = context->m_pGame->getMap()->m_pTerrain;
    if(ERR_OK != pT->loadUpperTiles(*context->m_pTerrainLoadingInfo)) {
        // On an error, we go back to the main menu.
        RunlevelManager::getSingleton().prepareRunlevelEntrance(new MainMenuRlv);
    }

    // This stage is done, show the user what will be done in the next one.
    String sTxt = context->getTranslation("Loadscr_Stage_Precalc");
    context->finishStage(sTxt, "lblTime_uppertiles");
    context->setState(new StateLoadTerrainPrecalc());
}

void FTS::LoadGameRlv::StateLoadTerrainPrecalc::doLoad( LoadGameRlv * context )
{
    // This stage pre-calculates the texture coordinates and normals.
    Terrain *pT = context->m_pGame->getMap()->m_pTerrain;
    pT->precalcTexCoords(*context->m_pTerrainLoadingInfo);
    pT->precalcNormals();

    // This stage is done, show the user what will be done in the next one.
    String sTxt = context->getTranslation("Loadscr_Stage_Forests");
    context->finishStage(sTxt, "lblTime_precalc");
    context->setState(new StateLoadForests());
}

void FTS::LoadGameRlv::StateLoadForests::doLoad( LoadGameRlv * context )
{
    // This stage load the forests (if there are some)
    //         if(FileUtils::fileExists(TEMP "forests") && FileUtils::fileExists(TEMP "forests.conf")) {
    //             m_pGame->getMap()->loadForests(TEMP "forests", TEMP "forests.conf");
    //         }

    // This stage is done, show the user what will be done in the next one.
    context->finishStage(String::EMPTY, "lblTime_forests");
    context->setState(new StateLoadScripts());

}

void FTS::LoadGameRlv::StateLoadScripts::doLoad( LoadGameRlv * context )
{
    DaoVm::getSingleton().pushContext(); // Start a new context in the vm.
    DaoVm::getSingleton().execute(Path("MapLoad.dao"));
    // This stage is done, show the user what will be done in the next one.
    context->finishStage(String::EMPTY, "lblTime_scripts");
    context->setState(new StateLoadFinalize());
}

void FTS::LoadGameRlv::StateLoadFinalize::doLoad( LoadGameRlv * context )
{
    // This stage is done, show the user what will be done in the next one.
    String sTxt = context->getTranslation("Loadscr_Stage_Starting");
    if(context->m_pGame->getMap()->getInfo()->getPressBtn()) {
        sTxt += " " + context->getTranslation("PressEnterToCont");

        // Wait for a button to be pressed? The register a callback.
        InputManager *pMgr = InputManager::getSingletonPtr();
        if(pMgr) {
            pMgr->add("blah_load_done_bleh", SpecialKey::Any, new CallbackCommand(FTS_SUBS_THIS(LoadGameRlv::cbAnyKeyPressed, context)));
            pMgr->add("blah_load_done_bleh2", MouseButton::Any, new CallbackCommand(FTS_SUBS_THIS(LoadGameRlv::cbAnyKeyPressed, context)));
        }

        context->m_bWaiting = true;
    }
    context->finishStage(sTxt, String::EMPTY);
    context->setState(new StateLoadDone());
}

void FTS::LoadGameRlv::StateLoadDone::doLoad( LoadGameRlv * context )
{

}
