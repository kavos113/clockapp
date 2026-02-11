cmake_minimum_required(VERSION 4.1)
project(clockapp)

set(CMAKE_CXX_STANDARD 20)

add_executable(clockapp src/main.cpp
        src/App.cpp
        src/App.h
        src/DWriteEngine.cpp
        src/DWriteEngine.h
        src/DrawInfo.h)

target_link_libraries(clockapp PRIVATE d2d1 d3d11 dxgi dwrite)
target_compile_definitions(clockapp PRIVATE UNICODE WIN32_LEAN_AND_MEAN)
