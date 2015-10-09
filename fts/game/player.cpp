/**
 * \file player.cpp
 * \author Pompei2
 * \date 03 May 2006
 * \brief This file implements the class that represents a player,
 *        online or single players are the same !
 **/

#include "game/player.h"
#include "logging/logger.h"
#include "connection.h"

using namespace FTS;

Player* FTS::g_pMeHacky = new Player();

/// Default constructor.
/** Creates the player object.
 *
 * \author Pompei2
 */
FTS::Player::Player()
{
    m_bOnline = false;
    m_pcMasterServer = NULL;
    m_pcHost = NULL;
    m_uiColour = 0xFF0000FF;
}

/// Default destructor
/** Destroys the player object.
 *
 * \author Pompei2
 */
FTS::Player::~Player()
{
    if(m_bOnline)
        this->og_logout();

    SAFE_DELETE(m_pcMasterServer);
    SAFE_DELETE(m_pcHost);
}

/// Sets the name of the player.
/** This sets the name of the player. This is done when logging in
 *  for a online player, or when starting a game for local/LAN players.
 *
 * \param in_sName The name of the player.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      Error code < 0
 *
 * \note One can only call this once, and one can't join/host
 *       a game 'till this is done.
 *
 * \author Pompei2
 */
void FTS::Player::setName( const String &in_sName )
{
    // If the name already exists, skip.
//     if( !m_sName.isEmpty( ) ) {
//         FTS18N("InvParam", FTS_WARNING,
//                "FTS::Player::setName called more then one time !");
//         return;
//     }

    m_sName = in_sName;
}

/// Gets the name of the player.
/** This gets the name of the player.
 *
 * \return The name of the player.
 *
 * \author Pompei2
 */
String FTS::Player::getName()
{
    return m_sName;
}
