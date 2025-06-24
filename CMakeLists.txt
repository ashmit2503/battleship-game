cmake_minimum_required(VERSION 3.10)
project(battleship-game)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(Threads REQUIRED)

# Add the executable
add_executable(battleship_server 
    cpp-server/battleship_server.cpp
    cpp-server/game.cpp
    cpp-server/lobby.cpp
)

# Add include path for header files
target_include_directories(battleship_server PRIVATE ${PROJECT_SOURCE_DIR}/cpp-server)

# Link libraries
target_link_libraries(battleship_server PRIVATE Threads::Threads)

# Compiler-specific options
if(MSVC)
    target_compile_options(battleship_server PRIVATE /W4)
else()
    target_compile_options(battleship_server PRIVATE -Wall -Wextra -pedantic)
endif()
