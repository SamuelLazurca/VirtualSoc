cmake_minimum_required(VERSION 3.24)
project(VirtualSoc1)

set(CMAKE_CXX_STANDARD 17)

add_executable(server server.cpp Server.h Server.cpp SocketAction.cpp SocketAction.h Client.cpp Client.h DataBaseAction.cpp DataBaseAction.h DataBase.cpp DataBase.h)

add_executable(client client.cpp ClientAction.cpp ClientAction.h)

add_executable(db_script db_script.cpp)

find_package(SFML 2.5 COMPONENTS graphics audio REQUIRED)

target_link_libraries(server LINK_PUBLIC sqlite3)
target_link_libraries(db_script LINK_PUBLIC sqlite3)
target_link_libraries(client LINK_PUBLIC sfml-graphics sfml-audio)