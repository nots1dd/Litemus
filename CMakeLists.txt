cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(LiteMusCache VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add include directories
include_directories(headers)

# Add source files
file(GLOB SOURCES "src/*.cpp" "main.cpp")

# Add executable
add_executable(LiteMusCache ${SOURCES})

# Find and link the nlohmann_json library
find_package(nlohmann_json 3.2.0 REQUIRED)
target_link_libraries(LiteMusCache PRIVATE nlohmann_json::nlohmann_json)

# Link necessary libraries
find_package(Threads REQUIRED)
target_link_libraries(LiteMusCache PRIVATE Threads::Threads)

# Optionally link with rt, pthread, and dl if available
find_library(LIB_RT rt)
if(LIB_RT)
    target_link_libraries(LiteMusCache PRIVATE ${LIB_RT})
endif()

find_library(LIB_DL dl)
if(LIB_DL)
    target_link_libraries(LiteMusCache PRIVATE ${LIB_DL})
endif()

# Add the source files for executeCmd and sanitize
target_sources(LiteMusCache PRIVATE
    headers/src/executeCmd.cpp
    headers/src/sanitize.cpp
)
