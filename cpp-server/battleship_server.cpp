/**
 * @file battleship_server.cpp
 * @brief WebSocket server for real-time Battleship game communication
 * 
 * Handles player connections, lobby management, and game state coordination
 * between clients using WebSocket protocol.
 */

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <memory>
#include <map>
#include <string>
#include <functional>
#include "game.hpp"
#include "lobby.hpp"

using json = nlohmann::json;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef websocketpp::server<websocketpp::config::asio> server;
typedef server::message_ptr message_ptr;
typedef websocketpp::connection_hdl connection_hdl;

/**
 * @class BattleshipServer
 * @brief Main WebSocket server class for handling Battleship game sessions
 */
class BattleshipServer {
public:
    /**
     * @brief Constructor - initializes WebSocket server and event handlers
     */
    BattleshipServer() {
        m_server.init_asio();
        m_server.clear_access_channels(websocketpp::log::alevel::all);
        m_server.set_access_channels(websocketpp::log::alevel::app);
        
        m_server.set_open_handler(bind(&BattleshipServer::on_open, this, _1));
        m_server.set_close_handler(bind(&BattleshipServer::on_close, this, _1));
        m_server.set_message_handler(bind(&BattleshipServer::on_message, this, _1, _2));
    }

    /**
     * @brief Start the server on specified port
     * @param port Port number to listen on
     */
    void run(uint16_t port) {
        m_server.listen(port);
        std::cout << "Battleship server listening on port " << port << std::endl;
        m_server.start_accept();
        m_server.run();
    }

private:
    server m_server;
    std::map<connection_hdl, std::string, std::owner_less<connection_hdl>> m_connections;
    std::map<std::string, std::unique_ptr<Lobby>> m_lobbies;
    std::map<std::string, std::unique_ptr<Game>> m_games;

    /**
     * @brief Handle new WebSocket connection
     */
    void on_open(connection_hdl hdl) {
        std::cout << "Connection opened" << std::endl;
    }

    /**
     * @brief Handle WebSocket connection closure and cleanup
     */
    void on_close(connection_hdl hdl) {
        std::cout << "Connection closed" << std::endl;
        
        auto it = m_connections.find(hdl);
        if (it != m_connections.end()) {
            std::string userId = it->second;
            
            for (auto& lobby_pair : m_lobbies) {
                Lobby& lobby = *lobby_pair.second;
                if (lobby.hasPlayer(userId)) {
                    lobby.removePlayer(userId);
                    notifyLobbyUpdate(lobby);
                    break;
                }
            }
            
            for (auto& game_pair : m_games) {
                Game& game = *game_pair.second;
                if (game.hasPlayer(userId)) {
                    game.playerDisconnected(userId);
                    
                    std::string opponentId = game.getOpponentId(userId);
                    if (!opponentId.empty()) {
                        connection_hdl opponent_hdl = getConnectionByUserId(opponentId);
                        if (opponent_hdl.lock()) {
                            json msg = {
                                {"type", "opponentDisconnected"},
                                {"message", "Your opponent has disconnected from the game."}
                            };
                            send(opponent_hdl, msg);
                        }
                    }
                    break;
                }
            }
            
            m_connections.erase(it);
        }
    }

    /**
     * @brief Process incoming WebSocket messages
     */
    void on_message(connection_hdl hdl, message_ptr msg) {
        try {
            json data = json::parse(msg->get_payload());
            std::string messageType = data.value("type", "");
            
            if (messageType == "join") {
                handleJoinMessage(hdl, data);
            }
            else if (messageType == "ready") {
                handleReadyMessage(hdl, data);
            }
            else if (messageType == "attack") {
                handleAttackMessage(hdl, data);
            }
            else {
                std::cout << "Unknown message type: " << messageType << std::endl;
            }
        } 
        catch (const std::exception& e) {
            std::cerr << "Error processing message: " << e.what() << std::endl;
        }
    }

    /**
     * @brief Handle player joining a lobby
     */
    void handleJoinMessage(connection_hdl hdl, const json& data) {
        std::string lobbyCode = data.value("lobby", "");
        std::string userId = data.value("user", "");
        std::string username = data.value("username", "");
        
        std::cout << "Player " << username << " (ID: " << userId << ") joining lobby " << lobbyCode << std::endl;
        
        m_connections[hdl] = userId;
        
        if (m_lobbies.find(lobbyCode) == m_lobbies.end()) {
            m_lobbies[lobbyCode] = std::make_unique<Lobby>(lobbyCode);
        }
        
        Lobby& lobby = *m_lobbies[lobbyCode];
        Player player{userId, username, hdl};
        lobby.addPlayer(player);
        
        if (lobby.getPlayerCount() == 2) {
            Player firstPlayer = lobby.getPlayers()[0];
            json notification = {
                {"type", "opponentJoined"},
                {"username", username}
            };
            send(firstPlayer.hdl, notification);
        }
        
        json confirmation = {
            {"type", "joinConfirmed"},
            {"message", "Successfully joined lobby " + lobbyCode}
        };
        send(hdl, confirmation);
    }

    /**
     * @brief Handle player ready status with ship placement
     */
    void handleReadyMessage(connection_hdl hdl, const json& data) {
        std::string userId = data.value("user", "");
        json board = data.value("board", json::object());
        
        std::cout << "Player " << userId << " is ready with " << board.size() << " ships" << std::endl;
        
        for (auto& lobby_pair : m_lobbies) {
            Lobby& lobby = *lobby_pair.second;
            if (lobby.hasPlayer(userId)) {
                std::cout << "Found player in lobby " << lobby.getLobbyCode() << std::endl;
                
                lobby.setPlayerReady(userId, board);
                
                std::cout << "Player count: " << lobby.getPlayerCount() 
                         << ", All ready: " << lobby.areAllPlayersReady() << std::endl;
                
                if (lobby.areAllPlayersReady()) {
                    std::cout << "Starting game for lobby " << lobby.getLobbyCode() << std::endl;
                    startGame(lobby);
                    return;
                } else {
                    std::cout << "Not all players ready yet" << std::endl;
                }
                break;
            }
        }
    }

    /**
     * @brief Handle attack messages during gameplay
     */
    void handleAttackMessage(connection_hdl hdl, const json& data) {
        auto conn_it = m_connections.find(hdl);
        if (conn_it == m_connections.end()) return;
        
        std::string userId = conn_it->second;
        int x = data.value("x", -1);
        int y = data.value("y", -1);
        
        if (x < 0 || y < 0 || x >= 10 || y >= 10) {
            std::cerr << "Invalid attack coordinates: " << x << "," << y << std::endl;
            return;
        }
        
        for (auto& game_pair : m_games) {
            Game& game = *game_pair.second;
            if (game.hasPlayer(userId)) {
                AttackResult result = game.processAttack(userId, x, y);
                
                json attackerMsg = {
                    {"type", "attackResult"},
                    {"x", x},
                    {"y", y},
                    {"hit", result.hit},
                    {"sunk", result.shipSunk},
                    {"nextPlayer", result.nextPlayerId},
                    {"gameOver", result.gameOver},
                    {"winner", result.winnerId}
                };
                send(hdl, attackerMsg);
                
                std::string defenderId = game.getOpponentId(userId);
                connection_hdl defender_hdl = getConnectionByUserId(defenderId);
                
                json defenderMsg = {
                    {"type", "attacked"},
                    {"x", x},
                    {"y", y},
                    {"hit", result.hit},
                    {"sunk", result.shipSunk},
                    {"nextPlayer", result.nextPlayerId},
                    {"gameOver", result.gameOver},
                    {"winner", result.winnerId}
                };
                send(defender_hdl, defenderMsg);
                
                if (result.gameOver) {
                    json gameOverMsg = {
                        {"type", "gameOver"},
                        {"winner", result.winnerId}
                    };
                    
                    send(hdl, gameOverMsg);
                    send(defender_hdl, gameOverMsg);
                    
                    std::string lobbyCode = game.getLobbyCode();
                    m_games.erase(lobbyCode);
                }
                break;
            }
        }
    }

    /**
     * @brief Initialize a new game from a ready lobby
     */
    void startGame(Lobby& lobby) {
        std::string lobbyCode = lobby.getLobbyCode();
        std::vector<Player> players = lobby.getPlayers();
        
        std::cout << "Starting game with " << players.size() << " players in lobby " << lobbyCode << std::endl;
        
        if (players.size() != 2) {
            std::cerr << "Cannot start game without exactly 2 players" << std::endl;
            return;
        }
        
        m_games[lobbyCode] = std::make_unique<Game>(lobbyCode, players[0], players[1], 
            lobby.getPlayerBoard(players[0].id), lobby.getPlayerBoard(players[1].id));
        
        Game& game = *m_games[lobbyCode];
        int firstPlayerIdx = game.decideFirstPlayer();
        std::string firstPlayerId = players[firstPlayerIdx].id;
        
        std::cout << "First player: " << firstPlayerId << " (index " << firstPlayerIdx << ")" << std::endl;
        
        for (const Player& player : players) {
            json startMsg = {
                {"type", "gameStart"},
                {"firstPlayer", firstPlayerId}
            };
            std::cout << "Sending gameStart message to player " << player.id << std::endl;
            send(player.hdl, startMsg);
        }
        
        m_lobbies.erase(lobbyCode);
        std::cout << "Game started successfully, lobby removed" << std::endl;
    }

    /**
     * @brief Notify players of lobby changes (placeholder for future use)
     */
    void notifyLobbyUpdate(const Lobby& lobby) {
        // Future implementation for lobby updates
    }

    /**
     * @brief Find connection handle by user ID
     */
    connection_hdl getConnectionByUserId(const std::string& userId) {
        connection_hdl empty;
        for (const auto& conn : m_connections) {
            if (conn.second == userId) {
                return conn.first;
            }
        }
        return empty;
    }

    /**
     * @brief Send JSON message to a specific connection
     */
    void send(connection_hdl hdl, const json& data) {
        try {
            m_server.send(hdl, data.dump(), websocketpp::frame::opcode::text);
        } catch (const std::exception& e) {
            std::cerr << "Error sending message: " << e.what() << std::endl;
        }
    }
};

/**
 * @brief Main entry point for the Battleship WebSocket server
 */
int main() {
    try {
        BattleshipServer server;
        server.run(9002);
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
