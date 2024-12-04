cmake_minimum_required(VERSION 3.10)
project(downloader VERSION 1.0 LANGUAGES CXX)

# Enable C++17 standard
set(CMAKE_CXX_STANDARD 17)

# Include external CMake modules (if any)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

# if(NOT TARGET spdlog)
#     # Stand-alone build
#     find_package(spdlog REQUIRED)
# endif()

# Add include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/submodules/spdlog/include
)

# Set global C++ flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2")

# Set C++ flags for Debug and Release builds
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

# Add subdirectories (for building executable, libraries, and tests)
add_subdirectory(src)
add_subdirectory(submodules/spdlog)
# add_subdirectory(tests)

# Optionally include packaging or installation rules
# install(TARGETS downloader DESTINATION bin)
