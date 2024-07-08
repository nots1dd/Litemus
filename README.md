# LiteMus - Light Music Player

A fast and interactive console music player written in C++ for UNIX Systems.

Supports any `.mp3`, `.wav` and `.flac` files! (with or without embedded metadata)

## Features

### Insanely Fast

* Insanely fast sorting and caching system thanks to Inode data structure of filesystem and C++ libraries (lmus_cache.h)

* Remote Caching of a directory that immediately registers in the current session

### Beautiful TUI

* Ncurses library for making the beautiful and fast TUI

* Dynamic Scrolling, string search, clean status bar and more!

* Showcases songs sorted by artist, then albums with release years and other metadata in a clean manner

### Smooth Audio

* SFML for the audio API for smooth audio quality and features

* Modular Code for easy usage of all SFML features

### Song Controls

* Pretty much all basic and advanced song controls are implemented with the exception queues

* Songs with or without metadata are all displayed with equal song controls!

## Demo

https://github.com/nots1dd/Litemus/assets/140317709/253cc58f-7034-46ae-9020-5881612d36da

For pictures on the working application, check the `misc/pictures` folder


## Pre-Requisites

-> A UNIX based filesystem (x86_64 arch)

-> A directory with `mp3 / wav / flac` files which have proper *embedded metadata* (without metadata also works fine)

-> Dependencies: `cmake`, `ncurses`, `sfml`, `nlohmann-json` and `ffmpeg` (for building this project)

## Build

### Note:

This has only been tried and tested on Arch Linux 6.9 kernel (x86-64)

### Steps:

1. Clone this repository: `https://github.com/nots1dd/Litemus.git`
2. Install the required C++ libraries: `ncurses`, `SFML`, `nlohmann-json` (and `ffmpeg in your system`)
3. Run `cmake -S . -B build/`
4. Run `cmake --build build/`

This will generate a `./build/Litemus` executable. Run it to get the Litemus experience!

> <span style="color: green;"><strong>Tip:</strong></span>
> 
> <span style="color: white;">A simpler way is to just use the <code>build.sh</code> script that does everything for you (including adding the <code>lmus</code> alias to your shell rc!)</span>
> 
> <span style="color: white;">NOTE: Run <code>chmod +x build.sh</code> in order for it to execute. ALWAYS BE CAREFUL OF WHAT YOU ARE EXECUTING!!</span>


## Installation

There is currently no means of installing this on any Linux distro other than building it from source.

If you encounter any issues with building Litemus from this repository in any distro, feel free to create an issue.

> **Tip:**
> 
> LiteMus can only dynamically window based on the initial window sizes 
> 
> Hence, it is noted that it still lacks some major dynamic resizing issues
> 
> BEST RESULTS are seen on: 1920x1080 screen size 

## Future

- [x] Have clean windows for showing session details, lyrics view
- [ ] Possibly have an audio visualizer (text based) integrated
- [x] Make the code more modular and easy to read, modify
- [x] Add other audio formats (.wav, .flac)
- [ ] Have custom keybinds

## LICENSE

This project is licensed under `GNU GPL v3`
