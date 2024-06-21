# LiteMus in C++

## C++ version of LiteMus, written with ncurses, SFLW

### FEATURES:

1. Insanely fast sorting and caching system thanks to Inode data structure of filesystem and C++ libraries
2. Ncurses library for making the beautiful TUI
3. SFLW for the audio API for smooth audio quality and FEATURES
4. Basic song controls are implemented

### BUILD:
Currently, `main.cpp` and `ncurses_lmus.cpp` are NOT connected, this is still a massive work in progress.

-> Firstly, clone this repository

-> To test `main.cpp`, just run

`mkdir build && cd build && cmake ..`

`cmake --build build/`

Then run the executable `./build/LiteMusCache`

-> To test `ncurses_lmus.cpp`,

`g++ -o ncurses_lmus ncurses_lmus.cpp -lsfml-system -lsfml-audio -lsfml-network -lsfml-graphics -lncurses -lmenu`

Then run the executable, `./ncurses_lmus`


### FUTURE:
1. Actual Integration of the cache system and `ncurses_lmus.cpp`
2. Parsing of the cached information for song display and lyrics
