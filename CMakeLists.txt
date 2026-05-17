cmake_minimum_required(VERSION 3.10)
project(noctctl C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add the main executable
add_executable(noctctl src/main.c src/ipc.c)

# Include directories
target_include_directories(noctctl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Installation
install(TARGETS noctctl DESTINATION bin)
