# Battleship Game

A modern web-based implementation of the classic Battleship game featuring real-time multiplayer gameplay with WebSocket communication.

## 🎮 Features

- **Real-time Multiplayer**: Play against other players in real-time using WebSocket connections
- **Lobby System**: Create and join game lobbies with unique 6-character codes
- **User Authentication**: Secure user registration and login system
- **Interactive Game Board**: Drag-and-drop ship placement and click-to-attack gameplay
- **Game State Management**: Complete game flow from lobby creation to game completion
- **Cross-platform**: Web-based interface accessible from any modern browser

## 🏗️ Architecture

This project uses a multi-service architecture:

- **Django Backend** (Python): REST API for user management and lobby operations
- **C++ WebSocket Server**: Real-time game communication and game logic
- **MongoDB**: Database for user data and lobby information
- **Docker Compose**: Orchestrates all services for easy deployment

## 📋 Prerequisites

- **Docker** and **Docker Compose**
- **CMake** (for local C++ development)
- **Python 3.11+** (for local Django development)
- **Modern web browser** with WebSocket support

## 🚀 Quick Start

### Using Docker (Recommended)

1. **Clone the repository**
   ```bash
   git clone <repository-url>
   cd battleship-game
   ```

2. **Start all services**
   ```bash
   docker-compose up --build
   ```

3. **Access the application**
   - Web interface: http://localhost:8000
   - WebSocket server: ws://localhost:9002
   - MongoDB: localhost:27017

### Local Development

#### Django Backend Setup

1. **Navigate to backend directory**
   ```bash
   cd backend
   ```

2. **Install Python dependencies**
   ```bash
   pip install -r requirements.txt
   ```

3. **Run database migrations**
   ```bash
   python manage.py migrate
   ```

4. **Start Django development server**
   ```bash
   python manage.py runserver
   ```

#### C++ WebSocket Server Setup

1. **Install dependencies** (Ubuntu/Debian)
   ```bash
   sudo apt-get install libwebsocketpp-dev nlohmann-json3-dev libasio-dev libboost-system-dev libboost-thread-dev
   ```

2. **Build the server**
   ```bash
   cmake -S . -B build
   cmake --build build
   ```

3. **Run the WebSocket server**
   ```bash
   ./build/battleship_server
   ```

#### MongoDB Setup

Start MongoDB locally or use the Docker container:
```bash
docker run -d -p 27017:27017 --name battleship-mongodb mongo:8.0
```

## 🎯 How to Play

1. **Register/Login**: Create an account or log in to an existing one
2. **Create/Join Lobby**: Either create a new lobby or join an existing one with a 6-character code
3. **Ship Placement**: Place your ships on the game board by dragging and dropping
4. **Ready Up**: Mark yourself as ready when ship placement is complete
5. **Battle**: Take turns attacking your opponent's board
6. **Victory**: First player to sink all opponent ships wins!

## 🔧 Configuration

### Environment Variables

The application supports the following environment variables:

- `MONGO_URI`: MongoDB connection string (default: `mongodb://localhost:27017`)
- `MONGO_DB_NAME`: MongoDB database name (default: `battleship_db`)
- `WEBSOCKET_HOST`: WebSocket server host (default: `localhost`)
- `WEBSOCKET_PORT`: WebSocket server port (default: `9002`)

### Django Settings

Key settings in `backend/backend/settings.py`:
- `DEBUG`: Development mode (set to `False` for production)
- `ALLOWED_HOSTS`: Allowed hostnames
- `SECRET_KEY`: Django secret key (change for production)

## 📡 API Endpoints

### Authentication
- `POST /register/` - User registration
- `POST /login/` - User login
- `GET /logout/` - User logout

### Game Management
- `GET /dashboard/` - User dashboard
- `POST /create_lobby/` - Create new game lobby
- `POST /join_lobby/` - Join existing lobby
- `POST /start_game/` - Start game when ready
- `GET /lobby_status/` - Get lobby status
- `GET /game/<lobby_code>/` - Game interface

## 🔌 WebSocket Events

### Client to Server
- `join` - Join a lobby
- `ready` - Mark player as ready with ship board
- `attack` - Attack opponent's position

### Server to Client
- `joinConfirmed` - Lobby join confirmation
- `playerJoined` - Another player joined
- `gameStart` - Game started
- `attackResult` - Result of attack
- `gameOver` - Game finished

## 🏗️ Project Structure

```
battleship-game/
├── backend/                 # Django REST API
│   ├── app/                # Main Django app
│   ├── backend/            # Django project settings
│   ├── static/             # Static files (CSS, JS)
│   ├── templates/          # HTML templates
│   └── requirements.txt    # Python dependencies
├── cpp-server/             # C++ WebSocket server
│   ├── battleship_server.cpp
│   ├── game.cpp/.hpp       # Game logic
│   ├── lobby.cpp/.hpp      # Lobby management
│   └── player.hpp          # Player structures
├── build/                  # CMake build output
├── CMakeLists.txt          # CMake configuration
├── docker-compose.yml      # Multi-service orchestration
├── Dockerfile              # C++ server container
└── README.md               # This file
```

## 🧪 Development

### Building C++ Server

Use the VS Code task or run manually:
```bash
cmake -S . -B build && cmake --build build
```

### Running Tests

```bash
# Django tests
cd backend
python manage.py test

# C++ tests (if implemented)
cd build
./battleship_server_tests
```

## 🐳 Docker Services

- **mongodb**: MongoDB 8.0 database
- **cpp-server**: C++ WebSocket server for real-time communication
- **django-backend**: Django REST API server

All services communicate through the `battleship-network` Docker network.