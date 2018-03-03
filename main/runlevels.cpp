#include "main/runlevels.h"

#include "3d/3d.h" // Only for FTS::Runlevel::renderCEGUI.
#include "ui/ui.h"
#include "ui/ui_menu_online.h"
#include "ui/ui_menu_online_main.h"
#include "input/input.h" // For the cursor loading.
#include "game/player.h"
#include "logging/logger.h"
#include "utilities/console.h"
#include "map/map.h"
#include "mdlviewer/mdlviewer_main.h"
#include "graphic/graphic.h"
#include "3d/Renderer.h" // The default camera.
#include "dLib/dString/dTranslation.h"

#include <CEGUI.h>

using namespace FTS;
Runlevel::Runlevel() 
    : m_pDefCursor(nullptr) 
{
    m_translation = new Translation("ui");
}
Runlevel::~Runlevel() 
{
    SAFE_DELETE(m_translation);
}

String Runlevel::getTranslation(const String& in_String)
{
    return m_translation->get(in_String);
}

/** This renders the whole CEGUI GUI, with an additional blue dotted frame
 *  around the widget that is currently active (for FTS).
 *
 * \author Pompei2
 */
void FTS::Runlevel::renderCEGUI()
{
    verifGL("Runlevel::renderCEGUI start");

    // Make the first texture unit active if it is not yet, so CEGUI gets what
    // it wants.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (GUI::getSingletonPtr() == NULL)
        return ;
    verifGL("Runlevel::renderCEGUI end1");

    // Try to render the GUI if it exists.
    try {
        CEGUI::System *pSys = CEGUI::System::getSingletonPtr();
        if (pSys) {
            pSys->renderGUI();
        }
    } catch (CEGUI::Exception &) { }

    // Deselect any texture they might have kept selected.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    verifGL("Runlevel::renderCEGUI end2");

    // Now we will draw a dotted line around the currently active widget.
    // But do not draw it when a tooltip is currently active.
    if (CEGUI::System::getSingletonPtr()->getDefaultTooltip())
        if (CEGUI::System::getSingletonPtr()->getDefaultTooltip()->isVisible())
            return ;

    CEGUI::Window *pW = GUI::getSingleton().getActiveWidget();
    if (pW == NULL || !GUI::getSingleton().tabbing()) {
        // No need for a dotted line if there is no active widget.
        return ;
    }

    CEGUI::Rect r = pW->getPixelRect();

    // Special case for comboboxes, only surround the editbox+button part.
    CEGUI::Combobox *pCombo = dynamic_cast<CEGUI::Combobox *>(pW);
    if (pCombo) {
        CEGUI::Window *pTmp = pCombo->getEditbox();
        r.d_left = pTmp->getPixelRect().d_left;
        r.d_bottom = pTmp->getPixelRect().d_bottom;
    }

    r.d_left -= 0.5f;
    r.d_top += 0.5f;
    r.d_right += 0.5f;
    r.d_bottom += 1.5f;

    // Get the real left/right, top/bottom.
    int iLeft = (int)std::min(r.d_left, r.d_right);
    int iRight = (int)std::max(r.d_left, r.d_right);
    int iTop = (int)std::min(r.d_top,r.d_bottom);
    int iBottom = (int)std::max(r.d_top,r.d_bottom);

    glBegin(GL_POINTS);
    glColor4f(0.0f,0.0f,1.0f,1.0f);

    // Top and bottom line.
    for(int x = iLeft+1 ; x < iRight ; x += 2) {
        glVertex2i(x, iTop);
        glVertex2i(x, iBottom);
    }
    // Left and right line
    for(int y = iTop+1 ; y < iBottom ; y += 2) {
        glVertex2i(iLeft, y);
        glVertex2i(iRight, y);
    }
    glEnd();

    verifGL("Runlevel::renderCEGUI end3");
}

/** This loads the default cursor object. should be called in your subclass'
 *  load method.
 *
 * \return ERR_OK on success or an error code < 0 on failure.
 *
 * \author Pompei2
 */
int FTS::Runlevel::loadDefaultCursor()
{
    // Try to load the cursor.
    m_pDefCursor = loadCursor("std");
    return m_pDefCursor == NULL ? -1 : ERR_OK;
}

/** This unloads the default cursor object. should be called in your subclass'
 *  unload method.
 *
 * \return ERR_OK.
 *
 * \author Pompei2
 */
int FTS::Runlevel::unloadDefaultCursor()
{
    // Try to load the cursor.
    deleteCursor(m_pDefCursor);
    return ERR_OK;
}

Camera& FTS::Runlevel::getActiveCamera()
{
    return Renderer::getSingleton().getDefault3DCamera();
}

Camera& FTS::Runlevel::getMainCamera()
{
    return Renderer::getSingleton().getDefault3DCamera();
}

/// Default constructor, does nothing great.
FTS::RunlevelManager::RunlevelManager()
{
    m_pCurrRunlevel = NULL;
    m_pRunlevelToEnter = NULL;
}

/// Default destructor, leaves and destroys the currently active runlevel too.
FTS::RunlevelManager::~RunlevelManager()
{
    // If there still is a runlevel, we may leave it and destroy it.
    if (m_pCurrRunlevel) {
        m_pCurrRunlevel->unload();
        SAFE_DELETE(m_pCurrRunlevel);
    }

    // Gotta remove it from the updateables, it no more exists!
    UpdateableManager::getSingleton().rem("Current Runlevel");

    // If we got this object, delete it too as it won't enter anymore.
    SAFE_DELETE(m_pRunlevelToEnter);
}

/** This method sets up everything so that the specified runlevel \a in_pRlv
 *  be entered upon the next game tick. Thus at the time this method returns,
 *  the game still is in its old runlevel. The new runlevel will only be loaded
 *  and activated later on.\n
 *  This is to avoid issues where changing the runlevel within a callback would
 *  corrupt one of the stack's spaces.
 *
 * \param in_pRlv The runlevel object you want to enter soon.
 *
 * \author Pompei2
 */
void FTS::RunlevelManager::prepareRunlevelEntrance(FTS::Runlevel *in_pRlv)
{
    assert(in_pRlv != NULL);
    m_pRunlevelToEnter = in_pRlv;
}

/** If a runlevel was prepared to entrance using the prepareRunlevelEntrance
 *  method, this method will really enter the runlevel that had been prepared.\n
 *  It will first unload the current runlevel. Then it tries to load the new
 *  runlevel. If that succeeds, it returns true. But if that failed, it will
 *  unload the (new) runlevel again (Thus cleanup code of your runlevel has to
 *  be solid), load the old level back again and return false.
 *
 * \return The runlevel that is currently active (after this method is done).
 *
 * \author Pompei2
 */
FTS::Runlevel *FTS::RunlevelManager::realEnterRunlevel()
{
    // Check if we need to enter a new runlevel first.
    if (m_pRunlevelToEnter == NULL)
        return m_pCurrRunlevel;

    Runlevel* pOldRLV = m_pCurrRunlevel;

    try {
        // Say that we prepare to enter a runlevel.
        FTS18NDBG("EnterRlv", 1);
        Console::Attr(Console::ATTRIBUTE::CHANGEFG, Console::COLOR::BLUE);
        FTSMSG(m_pRunlevelToEnter->getName(), MsgType::Raw);
        Console::Attr(Console::ATTRIBUTE::NORMAL);
        FTSMSG("  ...\n", MsgType::Raw);

        // Unfortunately, we need to unload the current runlevel even
        // before we load the new one, because else it might release
        // some resources the new runlevel has created (by name).
        int iCursorX = 0, iCursorY = 0;
        if (m_pCurrRunlevel) {
            // First, takeover the cursor position.
            PCursor pCursor = m_pCurrRunlevel->getActiveCursor();
            if (pCursor) {
                iCursorX = pCursor->iX;
                iCursorY = pCursor->iY;
            }

            // Then unload the old level.
            m_pCurrRunlevel->unload();
        }

        // Try to load the needed runlevel:
        m_pRunlevelToEnter->load();

        PCursor pCursor = m_pRunlevelToEnter->getActiveCursor();
        if (pCursor) {
            pCursor->iX = iCursorX;
            pCursor->iY = iCursorY;
        }

        // And register its update function over the old one.
        UpdateableManager::getSingleton().add("Current Runlevel", m_pRunlevelToEnter);

        // Here we make the switch over to the new runlevel active.
        // Nothing really fatal should be possible to happen here.
        m_pCurrRunlevel = m_pRunlevelToEnter;
        m_pRunlevelToEnter = NULL;
        SAFE_DELETE(pOldRLV);
        FTSMSGDBG("Runlevel {1} OK", 2, m_pCurrRunlevel->getName());

    } catch (const ArkanaException& e) {
        // Here, we still can backup to the old runlevel.
        e.show();

        // Nothing has really been setup yet, just unload the bad runlevel.
        m_pRunlevelToEnter->unload();
        SAFE_DELETE(m_pRunlevelToEnter);

        // If this was the first runlevel (LoadFTS) that failed, quit.
        if (pOldRLV == NULL)
            exit(0);

        // We can try to switch back to the old runlevel.
        if (!pOldRLV->load())
            exit(0);

        // And register its update function over the old one.
        UpdateableManager::getSingleton().add("Current Runlevel", pOldRLV);

        return m_pCurrRunlevel = pOldRLV;
    }

    return m_pCurrRunlevel;
}
