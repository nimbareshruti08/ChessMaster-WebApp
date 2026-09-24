cmake_minimum_required(VERSION 3.16)
project(ChessMaster LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# --- SFML from MSYS2/UCRT64 ---
find_package(SFML 3 CONFIG REQUIRED COMPONENTS Graphics Window System)

add_executable(ChessMaster WIN32
    src/main.cpp
    src/ChessGame.cpp
    src/Board.cpp
    src/Piece.cpp
    src/Move.cpp
    src/ChessAI.cpp
    src/MoveLinkedList.cpp
    src/MoveStack.cpp
    src/Timer.cpp
    src/GameUI.cpp
)

target_include_directories(ChessMaster PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(ChessMaster PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)

# Copy assets beside the EXE
add_custom_command(TARGET ChessMaster POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:ChessMaster>/assets
)