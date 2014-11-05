
#include "anim.h"
#include "graphic.h"
#include "logging/logger.h"
#include "dLib/dConf/configuration.h"
#include "dLib/dArchive/dArchive.h"

using namespace FTS;

FTS::Anim::Anim(const Path &in_sFile, bool in_bShow)
    : m_pGraphs(NULL)
    , m_iLenght(0)
    , m_nFrames(0)
    , m_bStarted(false)
    , m_bLoop(false)
    , m_sFile(in_sFile)
    , m_bLoaded(false)
    , m_bShow(true)
{
    m_timeStart = std::chrono::steady_clock::now();
    if(!in_sFile) {
        FTS18N("InvParam", MsgType::Horror, "Anim::Anim");
        return;
    }
}

FTS::Anim::~Anim()
{
    if(m_bLoaded)
        this->unload();
}

int FTS::Anim::load()
{
    try {
        FTSMSGDBG("Loading animation from file '"+m_sFile+"'", 2);

        // First, if there's still something loaded, unload it.
        if(m_bLoaded)
            this->unload();

        // Load the archive and get the configuration file out of the archive.
        Archive::Ptr pArch(Archive::loadArchive(m_sFile));
        File &confFile = pArch->getFile("a.xml");

        Configuration conf(confFile, Settings());

        // Read all info from the anim's config file.
        m_nFrames = conf.getInt ("Frames");
        m_iLenght = conf.getInt("Time");
        m_bLoop   = conf.getBool("Loop");

        // Load all those images that form the animation.
        m_pGraphs = new Graphic*[m_nFrames];
        for(uint64_t i = 0 ; i < m_nFrames ; ++i) {
            File &imageFile = pArch->getFile(String::nr(i+1) + ".png");
            m_pGraphs[i] = GraphicManager::getSingleton().getOrLoadGraphic(imageFile);
        }

        m_bLoaded = true;
        return ERR_OK;
    } catch(const ArkanaException& e) {
        e.show();
        return -1;
    }
}

int FTS::Anim::unload()
{
    if(!m_bLoaded) {
        FTS18N("InvParam", MsgType::Horror, "CAnim::unload");
        return -1;
    }

    for(uint64_t i = 0 ; i < m_nFrames ; ++i) {
        GraphicManager::getSingleton().destroyGraphic(m_pGraphs[i]);
    }

    SAFE_DELETE_ARR(m_pGraphs);

    m_bLoaded = false;
    m_bLoop   = false;
    m_bShow   = false;

    return ERR_OK;
}

int FTS::Anim::drawEx(int in_iX, int in_iY,
                      int in_iSubX, int in_iSubY, int in_iSubW, int in_iSubH,
                      float in_fRotate,
                      float in_fZoomX, float in_fZoomY,
                      float in_fR, float in_fG, float in_fB, float in_fA)
{
    if(!m_bLoaded) {
//        FTS18N( "InvParam", MsgType::Warning, "drawGraphicEx" );
        return -1;
    }

    // Do we have the right to show that graphic ?
    if(!m_bShow)
        return ERR_OK;

    if(!m_bStarted) {
        m_timeStart = std::chrono::steady_clock::now();
        m_bStarted = true;
    }

    return m_pGraphs[this->getCurrPic()]
                ->drawEx(in_iX, in_iY, in_iSubX, in_iSubY, in_iSubW, in_iSubH,
                         in_fRotate, in_fZoomX, in_fZoomY,
                         in_fR, in_fG, in_fB, in_fA);
}

int FTS::Anim::draw(int in_iX, int in_iY)
{
    if(!m_bLoaded)
        return -1;

    // Do we have the right to show that graphic ?
    if(!m_bShow)
        return ERR_OK;

    if(!m_bStarted) {
        m_timeStart = std::chrono::steady_clock::now();
        m_bStarted = true;
    }
    
    m_pGraphs[this->getCurrPic()]->draw(in_iX, in_iY);

    return ERR_OK;
}

uint64_t FTS::Anim::getCurrPic()
{
    uint64_t currpic = 0;

    double fFPS = (double)m_nFrames / (double)m_iLenght;
    auto timeDiff = std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::steady_clock::now() - m_timeStart ).count();
    currpic = (uint64_t)(timeDiff * fFPS);

    if(m_bLoop)
        return currpic < m_nFrames ? currpic : currpic % m_nFrames;
    else
        return currpic < m_nFrames ? currpic : m_nFrames - 1;
}
