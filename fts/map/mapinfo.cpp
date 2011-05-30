#include <CEGUI.h>

#include "mapinfo.h"

#include "dLib/dFile/dFile.h"
#include "dLib/dArchive/dArchive.h"
#include "logging/logger.h"
#include "net/packet.h"
#include "graphic/graphic.h"

using namespace FTS;

MapInfo::MapInfo()
    : m_sName("Yet another map")
    , m_sDesc("Yet another description of yet another map")
    , m_cMinPlayers(0)
    , m_cMaxPlayers(0)
    , m_sSuggPlayers("Very much")
    , m_sAuthor("Yet another mapper")
    , m_bStarted(false)
    , m_pPreview(NULL)
    , m_pIcon(NULL)
    , m_bPressBtnToStart(true)
    , m_bLoaded(false)
{
}

MapInfo::~MapInfo()
{
    this->unload();
    m_bLoaded = false;
}

int MapInfo::load(const String & in_sDirectory)
{
    if(m_bLoaded)
        this->unload();

    String sInfoFile = in_sDirectory + "info.xml";

    FTSMSGDBG("Loading map info from file \"" + sInfoFile + "\".", 3);
    Configuration conf(*File::open(sInfoFile, File::Read), Settings());
    m_sName = conf.get("Name");
    m_sDesc = conf.get("Desc");
    m_cMinPlayers = (uint8_t)conf.getInt("MinPlayers");
    m_cMaxPlayers = (uint8_t)conf.getInt("MaxPlayers");
    m_sSuggPlayers = conf.get("SuggPlayers");
    m_sAuthor = conf.get("Author");
    m_dtLastModif.fromInternStr(conf.get("LastModif"));
    m_bPressBtnToStart = conf.getBool("PressButtonToStart");

    m_pPreview = GraphicManager::getSingleton().getOrLoadGraphic(Path::datadir("Graphics/preview.png"));
    m_pIcon = GraphicManager::getSingleton().getOrLoadGraphic(Path::datadir("Graphics/icon.png"));

    m_bLoaded = true;
    return ERR_OK;
}

int MapInfo::loadFromMap(const String & in_sFileName)
{
    try {
        FTSMSGDBG("Loading map info from map file \"" + in_sFileName + "\".", 3);

        Archive *a = Archive::loadArchive(in_sFileName);
        File::addArchiveToLook(a);

        // Load the informations about the map.
        int iRet = this->load();
        SAFE_DELETE(a);

        return iRet;
    } catch(const ArkanaException& e) {
        e.show();
        return -1;
    }
}

int MapInfo::writeToPacket(Packet *out_pPacket) const
{
    if(!m_bLoaded)
        return -1;

    out_pPacket->append(m_sName);
    out_pPacket->append(m_sDesc);
    out_pPacket->append(m_cMinPlayers);
    out_pPacket->append(m_cMaxPlayers);
    out_pPacket->append(m_sSuggPlayers);
    out_pPacket->append(m_sAuthor);
    out_pPacket->append(m_dtLastModif.toInternStr());
    out_pPacket->append(m_bStarted ? (uint8_t)1 : (uint8_t)0);
    out_pPacket->append(m_bPressBtnToStart ? (uint8_t)1 : (uint8_t)0);
    m_pPreview->writeToPacket(out_pPacket);
    m_pIcon->writeToPacket(out_pPacket);
    return ERR_OK;
}

int MapInfo::readFromPacket(Packet *in_pPacket)
{
    if(m_bLoaded)
        this->unload();

    in_pPacket->get(m_sName);
    in_pPacket->get(m_sDesc);
    in_pPacket->get(m_cMinPlayers);
    in_pPacket->get(m_cMaxPlayers);
    in_pPacket->get(m_sSuggPlayers);
    in_pPacket->get(m_sAuthor);
    m_dtLastModif.fromInternStr(in_pPacket->get_string());
    m_bStarted = in_pPacket->get() == 1;
    m_bPressBtnToStart = in_pPacket->get() == 1;
    m_pPreview = GraphicManager::getSingleton().readGraphicFromPacket(in_pPacket);
    m_pIcon = GraphicManager::getSingleton().readGraphicFromPacket(in_pPacket);

    m_bLoaded = true;
    return ERR_OK;
}

int MapInfo::unload(void)
{
    GraphicManager::getSingleton().destroyGraphic(m_pPreview);
    GraphicManager::getSingleton().destroyGraphic(m_pIcon);
    m_sName = "Yet another map";
    m_sDesc = "Yet another description of yet another map";
    m_cMinPlayers = 0;
    m_cMaxPlayers = 0;
    m_sSuggPlayers = "Very much";
    m_sAuthor = "Yet another mapper";
    m_dtLastModif.empty();
    m_bStarted = false;
    m_bLoaded = false;

    return ERR_OK;
}
