cmake_minimum_required (VERSION 3.0)
set (CMAKE_CXX_STANDARD 17)
project (socketServer)

find_package(Threads REQUIRED)

add_executable(socketServer socketServer.cpp)
add_subdirectory(Server)
target_link_libraries(socketServer
                        server
                        Threads::Threads)

set(CMAKE_BUILD_TYPE Debug) 