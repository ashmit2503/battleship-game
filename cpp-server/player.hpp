/**
 * @file player.hpp
 * @brief Player structure for game sessions
 */

#pragma once

#include <string>
#include <websocketpp/connection.hpp>

typedef websocketpp::connection_hdl connection_hdl;

/**
 * @struct Player
 * @brief Represents a player in the game with connection information
 */
struct Player {
    std::string id;        ///< Unique player identifier
    std::string username;  ///< Player's display name
    connection_hdl hdl;    ///< WebSocket connection handle
    
    Player() = default;
    
    Player(const std::string& id, const std::string& username, connection_hdl hdl)
        : id(id), username(username), hdl(hdl) {}
};
