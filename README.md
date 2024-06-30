# LiteMus - Light Music Player

A fast and interactive music player built with SFML, ncurses, and nlohmann json libraries in C++.

## Features

### Insanely Fast

* Insanely fast sorting and caching system thanks to Inode data structure of filesystem and C++ libraries (lmus_cache.h)

### Beautiful TUI

* Ncurses library for making the beautiful TUI

### Smooth Audio

* SFML for the audio API for smooth audio quality and features

### Basic Song Controls

* Basic song controls are implemented (play, pause, seek forward/backward, next song, prev song)

## Demo

[![Demo Video](https://github.com/nots1dd/Litemus/assets/140317709/cce9fc82-14f5-4983-bfa8-a5b714d20910)](https://github.com/nots1dd/Litemus/assets/140317709/cce9fc82-14f5-4983-bfa8-a5b714d20910)

![litemusreadme1](https://github.com/nots1dd/Litemus/assets/140317709/c270df7d-af84-4cf9-a0b7-b475f27a2081)
![litemusreadme2](https://github.com/nots1dd/Litemus/assets/140317709/56a5010b-d71e-4a4a-ac27-0ec880970052)

## Build

### Note:

This has only been tried and tested on Arch Linux 6.9 kernel (x86-64)

### Steps:

1. Clone this repository: `https://github.com/nots1dd/Litemus.git`
2. Install the required C++ libraries: `ncurses`, `SFML`, `nlohmann-json`
3. Run `cmake -S . -B build/`
4. Run `cmake --build build/`

This will generate a `./build/Litemus` executable. Run it to get the Litemus experience!

> **Important Note**:
> 
> Change the `songsDirectory` variable in `litemus.cpp` in order for the project to work for you.
> 
> A fix for this is coming very soon.

## Installation

There is currently no means of installing this on any Linux distro other than building it from source.

If you encounter any issues with building Litemus from this repository in any distro, feel free to create an issue.

## Future

- [ ] Have clean windows for showing session details, lyrics view
- [ ] Possibly have an audio visualizer (text based) integrated
- [ ] Make the code more modular and easy to read, modify
- [ ] Add other audio formats (.wav, .flac)
- [ ] Have custom keybinds
