#include "input/input.h"
#include "ui/ui.h"
#include "3d/3d.h"
#include "3d/camera.h"
#include "logging/logger.h"
#include "graphic/anim.h"
#include "graphic/graphic.h"
#include "utilities/utilities.h"
#include "utilities/threading.h"
#include "main/runlevels.h"
#include "dLib/dConf/configuration.h"

#include <CEGUI.h>

using namespace FTS;
class CursorSettings : public DefaultOptions {
public:
    CursorSettings() {
        add("Normal","std.png");
        add("Left","std.png");
        add("Middle","std.png");
        add("Right","std.png");
        add("hotspotX",0);
        add("hotspotY",0);
    }
};

/** Reads a cursor's configuration file and then loads it.
 *
 * \param in_pszFile The configuration file of the cursor to load.
 *
 * \author Pompei2
 */
PCursor loadCursor(const String & in_sFile)
{
    PCursor pCur = NULL;

    String sImg[FTS_CURSOR_IMAGES];

    if(!in_sFile) {
        FTS18N("InvParam", MsgType::Horror, "loadCursor");
        return NULL;
    }

    Path sFile = Path::datadir("Graphics/ui/cursors") + Path(in_sFile + ".xml");

    /* Alloc the space for the structure. */
    pCur = new SCursor;

    /* now read all the configuration of the cursor. */
    Configuration conf(sFile, CursorSettings(), false);
    /* now read all the configuration of the cursor. */
    sImg[0] = conf.get("Normal");
    sImg[1] = conf.get("Left");
    sImg[2] = conf.get("Middle");
    sImg[3] = conf.get("Right");

    /* Check wether it's animated or not. */
    for(int i = 0; i < FTS_CURSOR_IMAGES; i++) {
        if(!sImg[i])
            continue;

        pCur->bAnimated[i] = sImg[i].contains(".anim");
    }

    pCur->iXHS = conf.getInt("hotspotX");
    pCur->iYHS = conf.getInt("hotspotY");

    pCur->iX = 0;
    pCur->iY = 0;

    /* Set all the mouse buttons to be not clicked. */
    for(int i = 0; i < MouseButton::NoButton - SpecialKey::NoSpecial; i++) {
        pCur->pbState[i] = false;
    }

    /* And finally load all the graphics or animations. */
    for(int i = 0; i < FTS_CURSOR_IMAGES; i++) {
        if(!sImg[i]) {
            /* If none selected, just take the default one. */
            pCur->bAnimated[i] = pCur->bAnimated[0];
            pCur->pGraph[i] = pCur->pGraph[0];
            pCur->pAnim[i] = pCur->pAnim[0];
        } else {
            if(pCur->bAnimated[i]) {
                pCur->pGraph[i] = NULL;
                pCur->pAnim[i] = new Anim(sFile.sameDir(sImg[i]));
                pCur->pAnim[i]->load();
            } else {
                pCur->pAnim[i] = NULL;
                pCur->pGraph[i] = GraphicManager::getSingleton().getOrLoadGraphic(sFile.sameDir(sImg[i]));
            }
        }
    }

    return pCur;
}

/** Unloads a cursor loaded by loadCursor.
 *
 * \param in_pCursor The cursor to unload.
 *
 * \author Pompei2
 */
int unloadCursor(PCursor in_pCursor)
{
    if(!in_pCursor)
        return ERR_OK;

    /* First delete the last ones, they could point at the first one. Get it ? */
    for(int i = FTS_CURSOR_IMAGES - 1; i >= 0; i--) {
        if(in_pCursor->bAnimated[i]) {
            if(in_pCursor->pAnim[i] != in_pCursor->pAnim[0])
                in_pCursor->pAnim[i]->unload();
        } else {
            if(!(in_pCursor->pGraph[i] == in_pCursor->pGraph[0]))
                GraphicManager::getSingleton().destroyGraphic(in_pCursor->pGraph[i]);
        }
    }

    return ERR_OK;
}

/** Deletes a cursor loaded by loadCursor.
 *
 * \param in_pCursor The cursor to delete.
 *
 * \author Pompei2
 */
int deleteCursor(PCursor in_pCursor)
{
    if(!in_pCursor)
        return ERR_OK;

    /* First delete the last ones, they could point at the first one. Get it ? */
    for(int i = FTS_CURSOR_IMAGES - 1; i >= 0; i--) {
        if(in_pCursor->bAnimated[i]) {
            if(i == 0 || in_pCursor->pAnim[i] != in_pCursor->pAnim[0])
                SAFE_DELETE(in_pCursor->pAnim[i]);
        } else {
            if(i == 0 || in_pCursor->pGraph[i] != in_pCursor->pGraph[0])
                GraphicManager::getSingleton().destroyGraphic(in_pCursor->pGraph[i]);
        }
    }

    SAFE_DELETE(in_pCursor);

    return ERR_OK;
}

/** Draws a cursor, animated if needed.
 *
 * \param in_pCursor The cursor to draw.
 *
 * \author Pompei2
 */
int drawCursor(const Clock& in_c, const PCursor in_pCursor)
{
    int iToDraw = 0;

    /* Do this in reverse order so the default is the standart cursor. */
    // Two is because: left, middle, right.
    for(int i = 2; i >= 0; i--) {
        if(in_pCursor->pbState[i])
            iToDraw = i + 1;
    }

    if(in_pCursor->bAnimated[iToDraw]) {
        in_pCursor->pAnim[iToDraw]->draw(in_pCursor->iX - in_pCursor->iXHS,
                                         in_pCursor->iY - in_pCursor->iYHS);
    } else {
        in_pCursor->pGraph[iToDraw]->draw(in_pCursor->iX - in_pCursor->iXHS,
                                          in_pCursor->iY - in_pCursor->iYHS);
    }

    return ERR_OK;
}

 /* EOF */
