/**
 * @file lobby.hpp
 * @brief Lobby management for player matchmaking and readiness
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>
#include "player.hpp"

using json = nlohmann::json;

/**
 * @class Lobby
 * @brief Manages player matchmaking and ship placement before game starts
 */
class Lobby {
public:
    /**
     * @brief Create a new lobby with given code
     * @param code Unique lobby identifier
     */
    explicit Lobby(const std::string& code);
    
    /**
     * @brief Add a player to the lobby
     * @param player Player to add
     */
    void addPlayer(const Player& player);
    
    /**
     * @brief Remove a player from the lobby
     * @param playerId ID of player to remove
     */
    void removePlayer(const std::string& playerId);
    
    /**
     * @brief Check if player is in this lobby
     * @param playerId Player ID to check
     * @return True if player is in lobby
     */
    bool hasPlayer(const std::string& playerId);
    
    /**
     * @brief Mark player as ready with their ship placement
     * @param playerId Player ID
     * @param board Player's ship placement board
     */
    void setPlayerReady(const std::string& playerId, const json& board);
    
    /**
     * @brief Check if all players are ready to start
     * @return True if all players are ready
     */
    bool areAllPlayersReady() const;
    
    /**
     * @brief Get the lobby code
     * @return Lobby code string
     */
    std::string getLobbyCode() const;
    
    /**
     * @brief Get number of players in lobby
     * @return Player count
     */
    size_t getPlayerCount() const;
    
    /**
     * @brief Get all players in lobby
     * @return Vector of players
     */
    std::vector<Player> getPlayers() const;
    
    /**
     * @brief Get a player's ship placement board
     * @param playerId Player ID
     * @return JSON board configuration
     */
    json getPlayerBoard(const std::string& playerId) const;
    
private:
    std::string m_code;                           ///< Lobby identifier
    std::vector<Player> m_players;                ///< Players in lobby
    std::map<std::string, bool> m_playerReady;    ///< Ready status by player ID
    std::map<std::string, json> m_playerBoards;   ///< Ship boards by player ID
};
