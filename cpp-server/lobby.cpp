/**
 * @file lobby.cpp
 * @brief Implementation of lobby management functionality
 */

#include "lobby.hpp"
#include <algorithm>

Lobby::Lobby(const std::string& code) : m_code(code) {}

void Lobby::addPlayer(const Player& player) {
    if (hasPlayer(player.id)) {
        return;
    }
    
    m_players.push_back(player);
    m_playerReady[player.id] = false;
}

void Lobby::removePlayer(const std::string& playerId) {
    m_players.erase(
        std::remove_if(m_players.begin(), m_players.end(),
            [&](const Player& p) { return p.id == playerId; }),
        m_players.end()
    );
    
    m_playerReady.erase(playerId);
    m_playerBoards.erase(playerId);
}

bool Lobby::hasPlayer(const std::string& playerId) {
    return std::any_of(m_players.begin(), m_players.end(),
        [&](const Player& p) { return p.id == playerId; });
}

void Lobby::setPlayerReady(const std::string& playerId, const json& board) {
    if (hasPlayer(playerId)) {
        m_playerReady[playerId] = true;
        m_playerBoards[playerId] = board;
    }
}

bool Lobby::areAllPlayersReady() const {
    if (m_players.size() < 2) return false;
    
    return std::all_of(m_players.begin(), m_players.end(),
        [&](const Player& p) {
            return m_playerReady.count(p.id) && m_playerReady.at(p.id);
        }
    );
}

std::string Lobby::getLobbyCode() const {
    return m_code;
}

size_t Lobby::getPlayerCount() const {
    return m_players.size();
}

std::vector<Player> Lobby::getPlayers() const {
    return m_players;
}

json Lobby::getPlayerBoard(const std::string& playerId) const {
    if (m_playerBoards.count(playerId)) {
        return m_playerBoards.at(playerId);
    }
    return json::object();
}
