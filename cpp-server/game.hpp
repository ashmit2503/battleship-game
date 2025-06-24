/**
 * @file game.hpp
 * @brief Game logic and state management for Battleship matches
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <websocketpp/connection.hpp>
#include <nlohmann/json.hpp>
#include "player.hpp"

using json = nlohmann::json;

/**
 * @struct AttackResult
 * @brief Result of an attack containing hit information and game state
 */
struct AttackResult {
    bool hit;              ///< Whether the attack hit a ship
    bool shipSunk;         ///< Whether the hit sunk a ship
    bool gameOver;         ///< Whether the game has ended
    std::string nextPlayerId;  ///< ID of the next player to move
    std::string winnerId;      ///< ID of the winning player (if game over)
    std::string shipId;        ///< ID of the ship that was hit
};

/**
 * @class Game
 * @brief Manages game state and logic for a Battleship match between two players
 */
class Game {
public:
    /**
     * @brief Constructor for a new game session
     * @param lobbyCode Unique identifier for the game lobby
     * @param player1 First player information
     * @param player2 Second player information
     * @param board1 First player's ship placement board
     * @param board2 Second player's ship placement board
     */
    Game(const std::string& lobbyCode, const Player& player1, const Player& player2,
         const json& board1, const json& board2);
    
    /**
     * @brief Randomly determine which player goes first
     * @return Index (0 or 1) of the first player
     */
    int decideFirstPlayer();
    
    /**
     * @brief Process an attack from a player
     * @param attackerId ID of the attacking player
     * @param x X coordinate of the attack
     * @param y Y coordinate of the attack
     * @return AttackResult containing hit information and game state
     */
    AttackResult processAttack(const std::string& attackerId, int x, int y);
    
    /**
     * @brief Check if a player is in this game
     * @param userId Player ID to check
     * @return True if player is in this game
     */
    bool hasPlayer(const std::string& userId);
    
    /**
     * @brief Get the opponent's ID for a given player
     * @param userId Player ID
     * @return Opponent's player ID
     */
    std::string getOpponentId(const std::string& userId);
    
    /**
     * @brief Handle player disconnection
     * @param userId ID of the disconnected player
     */
    void playerDisconnected(const std::string& userId);
    
    /**
     * @brief Get the lobby code for this game
     * @return Lobby code string
     */
    std::string getLobbyCode() const;
    
private:
    std::string m_lobbyCode;       ///< Unique lobby identifier
    Player m_player1;              ///< First player
    Player m_player2;              ///< Second player
    json m_board1;                 ///< First player's board
    json m_board2;                 ///< Second player's board
    std::string m_currentTurnId;   ///< Current player's turn
    
    /// Track which parts of each ship have been hit
    std::map<std::string, std::map<std::string, bool>> m_shipHits;
    /// Count remaining ships for each player
    std::map<std::string, int> m_remainingShips;
    /// Track all hit positions for each player
    std::map<std::string, std::set<int>> m_playerHits;
    
    /**
     * @brief Check if there's a ship at the given coordinates
     * @param board Player's board to check
     * @param x X coordinate
     * @param y Y coordinate
     * @return True if ship exists at coordinates
     */
    bool isShipHit(const json& board, int x, int y);
    
    /**
     * @brief Check if a ship has been completely sunk
     * @param playerId Owner of the ship
     * @param shipId Ship identifier
     * @return True if ship is completely sunk
     */
    bool isShipSunk(const std::string& playerId, const std::string& shipId);
    
    /**
     * @brief Get the ship ID at given coordinates
     * @param board Player's board to check
     * @param x X coordinate
     * @param y Y coordinate
     * @return Ship ID if found, empty string otherwise
     */
    std::string getShipAtPosition(const json& board, int x, int y);
};
