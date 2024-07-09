# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic

# Directories
SRC_DIR = headers/src
INC_DIR = headers
BUILD_DIR = build
EXECUTABLE = Litemus

# Source files
SRCS = litemus.cpp \
       $(SRC_DIR)/executeCmd.cpp \
       $(SRC_DIR)/lmus_cache.cpp \
       $(SRC_DIR)/sfml_helpers.cpp \
       $(SRC_DIR)/ncurses_helpers.cpp \
       $(SRC_DIR)/parsers.cpp \
       $(SRC_DIR)/checkSongDir.cpp \
       $(SRC_DIR)/keyHandlers.cpp

# Object files
OBJS = $(SRCS:%.cpp=$(BUILD_DIR)/%.o)

# SFML and ncurses
SFML_LIBS = -lsfml-audio -lsfml-system
NCURSES_LIBS = -lncurses -lmenu

# nlohmann JSON (assuming it's installed globally)
JSON_LIBS = -ljsoncpp

# Includes
INCLUDES = -I$(INC_DIR)

# Targets
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(SFML_LIBS) $(NCURSES_LIBS) $(JSON_LIBS)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(EXECUTABLE)

.PHONY: all clean
