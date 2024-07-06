# LiteMus - Light Music Player

A fast and interactive console music player written in C++ for UNIX Systems.

## Features

### Insanely Fast

* Insanely fast sorting and caching system thanks to Inode data structure of filesystem and C++ libraries (lmus_cache.h)

* Remote Caching of a directory that immediately registers in the current session

### Beautiful TUI

* Ncurses library for making the beautiful and fast TUI

* Dynamic Scrolling, string search, clean status bar and more!

### Smooth Audio

* SFML for the audio API for smooth audio quality and features

* Modular Code for easy usage of all SFML features

### Song Controls

* Pretty much all basic and advanced song controls are implemented with the exception queues

* Songs with or without metadata are all displayed with equal song controls!

## Demo

[![Demo Video](https://github.com/nots1dd/Litemus/assets/140317709/cce9fc82-14f5-4983-bfa8-a5b714d20910)](https://github.com/nots1dd/Litemus/assets/140317709/cce9fc82-14f5-4983-bfa8-a5b714d20910)

![lmusPic1](https://github.com/nots1dd/Litemus/assets/140317709/736e6e8f-af47-4102-9365-187c7ac0f176)
![lmusPic2](https://github.com/nots1dd/Litemus/assets/140317709/a87f3118-ec39-470f-8cd6-06287a4a37dc)


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

> **Tip:**
> 
> A simpler way is to just use the `build.sh` script that does everything for you (including adding the `lmus` alias to your shell rc!)
> 
> NOTE: Run `chmod +x build.sh` in order for it to execute. ALWAYS BE CAREFUL OF WHAT YOU ARE EXECUTING!!
> 

## Installation

There is currently no means of installing this on any Linux distro other than building it from source.

If you encounter any issues with building Litemus from this repository in any distro, feel free to create an issue.

> **Tip:**
> 
> It is advised that you run Litemus in a full window at all times 
> 
> This application still lacks dynamic windowing
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
