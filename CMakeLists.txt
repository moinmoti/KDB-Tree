cmake_minimum_required(VERSION 3.19)
project(KDBTree)

#set(CMAKE_OSX_SYSROOT /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS "-O2")

add_executable(KDBTree
        src/main.cpp
        src/KDBTree.cpp
        src/SuperNode.cpp
        src/OrientSplit.cpp)

target_include_directories(KDBTree PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
