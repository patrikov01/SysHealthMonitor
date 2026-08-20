cmake_minimum_required(VERSION 3.16)

project(SysHealthMonitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(sys_health_monitor
    src/main.cpp
    src/healthmonitor.cpp
    src/logger.cpp
    src/systemstats.cpp
)

target_include_directories(sys_health_monitor PRIVATE include)

target_compile_options(sys_health_monitor PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
