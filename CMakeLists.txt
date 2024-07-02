cmake_minimum_required(VERSION 3.10)

# Project name and version
project(Litemus VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
add_executable(
  Litemus 
  litemus.cpp 
  headers/src/executeCmd.cpp 
  headers/src/lmus_cache.cpp 
  headers/src/sfml_helpers.cpp 
  headers/src/ncurses_helpers.cpp 
  headers/src/parsers.cpp
  headers/src/checkSongDir.cpp
  headers/src/keyHandlers.cpp
)

# Find and include SFML
find_package(SFML 2.5 COMPONENTS audio REQUIRED)
if (SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    target_link_libraries(Litemus sfml-audio sfml-system)
else()
    message(FATAL_ERROR "SFML library not found")
endif()

# Include nlohmann JSON library
# Assumes nlohmann_json is installed and findable by CMake
find_package(nlohmann_json 3.2.0 REQUIRED)
target_link_libraries(Litemus nlohmann_json::nlohmann_json)

# Include ncurses and menu libraries
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})
target_link_libraries(Litemus ${CURSES_LIBRARIES})
target_link_libraries(Litemus menu)

# Include custom headers
include_directories(${CMAKE_SOURCE_DIR}/headers)

# Add additional source files if needed
# For example:
# target_sources(Litemus PRIVATE src/exitError.cpp src/executeCmd.cpp src/sanitize.cpp src/directoryUtils.cpp)

# For custom headers (e.g., lmus_cache.hpp), ensure they're in the correct directory
target_include_directories(Litemus PRIVATE headers)

# Set compile options
target_compile_options(Litemus PRIVATE -Wall -Wextra -pedantic)
