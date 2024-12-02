cmake_minimum_required(VERSION 3.10)
project(downloader VERSION 1.0 LANGUAGES CXX)

# Enable C++17 standard
set(CMAKE_CXX_STANDARD 17)

# Include external CMake modules (if any)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

# Add include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add subdirectories (for building executable, libraries, and tests)
add_subdirectory(src)
add_subdirectory(tests)

# Optionally include packaging or installation rules
# install(TARGETS LeonardoMVP DESTINATION bin)
