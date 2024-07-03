# LiteMus - Light Music Player

A fast and interactive console music player written in C++ for UNIX Systems.

## Features

### Insanely Fast

* Insanely fast sorting and caching system thanks to Inode data structure of filesystem and C++ libraries (lmus_cache.h)

* Remote Caching of a directory that immediately registers in the current session

### Beautiful TUI

* Ncurses library for making the beautiful TUI

### Smooth Audio

* SFML for the audio API for smooth audio quality and features

### Song Controls

* Pretty much all basic and advanced song controls are implemented with the exception queues

## Demo

[![Demo Video](https://github.com/nots1dd/Litemus/assets/140317709/cce9fc82-14f5-4983-bfa8-a5b714d20910)](https://github.com/nots1dd/Litemus/assets/140317709/cce9fc82-14f5-4983-bfa8-a5b714d20910)

![lmusPic1](https://github.com/nots1dd/Litemus/assets/140317709/736e6e8f-af47-4102-9365-187c7ac0f176)
![lmusPic2](https://github.com/nots1dd/Litemus/assets/140317709/a87f3118-ec39-470f-8cd6-06287a4a37dc)


## Pre-Requisites

-> A UNIX based filesystem (x86_64 arch)

-> A directory with `mp3` files which have proper *embedded metadata* (random mp3 files will not work and may lead to some unexepcted results)

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

- [ ] Have clean windows for showing session details, lyrics view
- [ ] Possibly have an audio visualizer (text based) integrated
- [ ] Make the code more modular and easy to read, modify
- [ ] Add other audio formats (.wav, .flac)
- [ ] Have custom keybinds

## LICENSE

This project is licensed under `GNU GPL v3`
