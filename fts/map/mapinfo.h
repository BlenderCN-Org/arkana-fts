#ifndef FTS_MAPINFO_H
#define FTS_MAPINFO_H

#include "main.h"

#include "utilities/DateTime.h"
#include "dLib/dString/dString.h"
#include "graphic/graphic.h"
#include "dLib/dConf/configuration.h"

namespace FTS {
    class Packet;

class MapInfo {
private:
    // From info.conf:
    String m_sName;         ///< The name of the map.
    String m_sDesc;         ///< A description of the map.
    uint8_t m_cMinPlayers;   ///< The minimum number of players needed to play this map.
    uint8_t m_cMaxPlayers;   ///< The maximum number of players allowed to play this map.
    String m_sSuggPlayers;  ///< The suggested number of players (for example: "2v2 and 3v3").
    String m_sAuthor;       ///< This is a string containing the name of the author of the map.
    DateTime m_dtLastModif; ///< When this map has been last modified.
    bool m_bStarted;         ///< Whether the game has beens tarted or not.
    Graphic *m_pPreview;     ///< The preview image that is associated with this map.
    Graphic *m_pIcon;        ///< The icon image that is associated with this map.
    bool m_bPressBtnToStart; ///< Whether to press a button after loading the map nor not.

    bool m_bLoaded;          ///< Whether infos are loaded or not.
    class Settings : public DefaultOptions {
    public:
        Settings() {
            add("Name", "N/A");
            add( "Desc", "N/A");
            add( "MinPlayers", 0);
            add( "MaxPlayers", 0);
            add( "SuggPlayers", "N/A");
            add( "Author", "N/A");
            add( "LastModif", "2000-01-01 00:00:00");
            add( "PressButtonToStart", true);
        }
    };

public:
    MapInfo();
    virtual ~MapInfo();

    int load(const String & in_sDirectory = String::EMPTY);
    int loadFromMap(const String & in_sFileName);
    int unload();
    inline bool isLoaded() {return m_bLoaded;};

    int writeToPacket(Packet *out_pPacket) const;
    int readFromPacket(Packet *in_pPacket);

    inline String getName() const { return m_sName; };
    inline String getDesc() const { return m_sDesc; };
    inline uint8_t getMinPlayers() const { return m_cMinPlayers; };
    inline uint8_t getMaxPlayers() const { return m_cMaxPlayers; };
    inline String getSuggPlayers() const { return m_sSuggPlayers; };
    inline String getAuthor() const { return m_sAuthor; };
    inline DateTime getLastModif() const { return m_dtLastModif; };
    inline const Graphic * const getPreview() const { return m_pPreview; };
    inline const Graphic * const getIcon() const { return m_pIcon; };
    inline bool getPressBtn() const { return m_bPressBtnToStart; };
};

}

#endif                          /* FTS_MAPINFO_H */

 /* EOF */
