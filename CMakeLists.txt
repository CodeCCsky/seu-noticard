cmake_minimum_required(VERSION 3.20)
project(seu-noticard CXX C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_executable(
    seu-noticard
    src/main.cpp
    src/web/server.cpp
)

target_include_directories(seu-noticard PRIVATE src)

if(MSVC)
    target_compile_options(seu-noticard PRIVATE /utf-8 /W4)
endif()

target_compile_definitions(
    seu-noticard PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    UNICODE
    _UNICODE
)

target_link_libraries(seu-noticard PRIVATE ws2_32 shell32)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT seu-noticard)
