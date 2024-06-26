# LiteMus - Light Music Player written in C++

## Fast and Interactive music player that uses SLFW, ncurses, nlohmann json libraries

### FEATURES:

1. Insanely fast sorting and caching system thanks to Inode data structure of filesystem and C++ libraries (lmus_cache.h)
2. Ncurses library for making the beautiful TUI
3. SFLW for the audio API for smooth audio quality and FEATURES
4. Basic song controls are implemented (play, pause, seek forward/backward, next song , prev song)

![litemusreadme1](https://github.com/nots1dd/Litemus/assets/140317709/c270df7d-af84-4cf9-a0b7-b475f27a2081)
![litemusreadme2](https://github.com/nots1dd/Litemus/assets/140317709/56a5010b-d71e-4a4a-ac27-0ec880970052)


### BUILD:

#### NOTE: This has only been tried and tested on Arch Linux 6.9 kernel (x86-64)

->  Clone this repository `https://github.com/nots1dd/Litemus.git`

-> Install the required C++ libraries `ncurses`, `SFLW`, `nlohmann-json`

->  Then run `cmake -S .-B build/`

-> Finally run `cmake --build build/`

This will generate a `./build/Litemus` executable. Run it to get the Litemus experience!

#### CAUTION: Running the executable might not work as there are some directories that are HARDCODED in this project. Change them to as you see fit in order for it to work. Will come up with a clean way of handling this issue in the near future.

### INSTALLATION:

There is currently no means of installing this on any Linux distro other than building it from source.

-> As such, if there are any issues with building Litemus from this repository in any distro, feel free to care an issue.

### FUTURE:
1. Parsing of the cached information for song display and lyrics
2. CMUS like two window system (left for artists, right for their songs sorted by album)
3. Multiple windows like song display, lyrics display and litemus session details (maybe even an audio visualizer)
