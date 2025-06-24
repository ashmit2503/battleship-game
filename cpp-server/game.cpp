/**
 * @file game.cpp
 * @brief Implementation of game logic and state management
 */

#include "game.hpp"
#include <random>

Game::Game(const std::string& lobbyCode, const Player& player1, const Player& player2,
           const json& board1, const json& board2)
    : m_lobbyCode(lobbyCode), m_player1(player1), m_player2(player2), 
      m_board1(board1), m_board2(board2) {
    
    m_remainingShips[player1.id] = board1.size();
    m_remainingShips[player2.id] = board2.size();
    
    for (auto& [shipId, positions] : board1.items()) {
        m_shipHits[player1.id][shipId] = false;
    }
    
    for (auto& [shipId, positions] : board2.items()) {
        m_shipHits[player2.id][shipId] = false;
    }
    
    m_playerHits[player1.id] = std::set<int>();
    m_playerHits[player2.id] = std::set<int>();
}

int Game::decideFirstPlayer() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 1);
    
    int firstPlayer = distrib(gen);
    m_currentTurnId = (firstPlayer == 0) ? m_player1.id : m_player2.id;
    
    return firstPlayer;
}

AttackResult Game::processAttack(const std::string& attackerId, int x, int y) {
    AttackResult result = {};
    
    if (attackerId != m_currentTurnId) {
        result.nextPlayerId = m_currentTurnId;
        return result;
    }
    
    std::string defenderId = getOpponentId(attackerId);
    json& defenderBoard = (defenderId == m_player1.id) ? m_board1 : m_board2;
    
    int index = y * 10 + x;
    
    if (m_playerHits[defenderId].find(index) != m_playerHits[defenderId].end()) {
        result.nextPlayerId = m_currentTurnId;
        return result;
    }
    
    m_playerHits[defenderId].insert(index);
    
    result.hit = isShipHit(defenderBoard, x, y);
    
    if (result.hit) {
        std::string hitShipId = getShipAtPosition(defenderBoard, x, y);
        result.shipId = hitShipId;
        
        if (!hitShipId.empty()) {
            bool allPositionsHit = true;
            
            for (const auto& pos : defenderBoard[hitShipId]) {
                if (!pos.is_number()) continue;
                
                int posIdx = pos.get<int>();
                if (m_playerHits[defenderId].find(posIdx) == m_playerHits[defenderId].end()) {
                    allPositionsHit = false;
                    break;
                }
            }
            
            if (allPositionsHit) {
                result.shipSunk = true;
                m_shipHits[defenderId][hitShipId] = true;
                m_remainingShips[defenderId]--;
            }
        }
    }
    
    if (m_remainingShips[defenderId] == 0) {
        result.gameOver = true;
        result.winnerId = attackerId;
    }

    m_currentTurnId = defenderId;
    result.nextPlayerId = m_currentTurnId;

    return result;
}

bool Game::hasPlayer(const std::string& userId) {
    return userId == m_player1.id || userId == m_player2.id;
}

std::string Game::getOpponentId(const std::string& userId) {
    if (userId == m_player1.id) return m_player2.id;
    if (userId == m_player2.id) return m_player1.id;
    return "";
}

void Game::playerDisconnected(const std::string& userId) {
    // Handle player disconnection - could implement game abandonment logic here
}

std::string Game::getLobbyCode() const {
    return m_lobbyCode;
}

bool Game::isShipHit(const json& board, int x, int y) {
    int index = y * 10 + x;
    
    for (auto& [shipId, positions] : board.items()) {
        if (positions.is_array()) {
            for (const auto& pos : positions) {
                if (pos.is_number() && pos.get<int>() == index) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

std::string Game::getShipAtPosition(const json& board, int x, int y) {
    int index = y * 10 + x;
    
    for (auto& [shipId, positions] : board.items()) {
        if (positions.is_array()) {
            for (const auto& pos : positions) {
                if (pos.is_number() && pos.get<int>() == index) {
                    return shipId;
                }
            }
        }
    }
    
    return "";
}

bool Game::isShipSunk(const std::string& playerId, const std::string& shipId) {
    return m_shipHits[playerId][shipId];
}
