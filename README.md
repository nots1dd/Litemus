# LiteMus - Light Music Player

A fast and interactive console music player written in C++ for UNIX Systems.

Supports any `.mp3`, `.wav` and `.flac` files! (with or without embedded metadata)

Great TUI with great metadata parsing along with custom keybinds all wrapped in one!

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

* Custom keybinds that fit ever user's needs!!

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

#### Building with CMAKE:

If you want to do this the hard way (without `build.sh`):

1. Clone this repository: `https://github.com/nots1dd/Litemus.git`
2. Install the required C++ libraries: `ncurses`, `SFML`, `nlohmann-json` (and `ffmpeg` in your system)
3. Run `cmake -S . -B build/`
4. Run `cmake --build build/`
5. If the build is successful, copy `keybinds.json` to `$HOME/.config/litemus/` directory (create it if needed)

This will generate a `./build/Litemus` executable and will be able to read your custom keybinds. Run it to get the Litemus experience!

#### Building with MAKE:

If for some reason you do not like or have cmake, there is a `Makefile` for this project

-> In the current directory run: `make` (`sudo make` should NOT be necessary)

This should compile all the files and link them to a `Litemus` executable. Simply run it!

-> To clean up, run: `make clean`

> <span style="color: green;"><strong>Tip:</strong></span>
> 
> <span style="color: white;">A simpler way is to just use the <code>build.sh</code> script that does everything for you (including adding the <code>lmus</code> alias to your shell rc!)</span>
> 
> <span style="color: white;">NOTE: Run <code>chmod +x build.sh</code> in order for it to execute. ALWAYS BE CAREFUL OF WHAT YOU ARE EXECUTING!!</span>


## Configuration

### Keybinds 

Custom keybinds are fully implemented within LiteMus, with an efficient system of parsing them (via `nlohmann-json` of course)

-> When building using `build.sh`, a `$HOME/.config/litemus/` directory is created where the current directory's `keybinds.json` is copied to

-> The `$HOME/.config/litemus/keybinds.json` file can be updated as you need!

-> There is verbose output as well that lets the user know if anything went wrong while parsing

> **NOTE:**
> 
> The code checks for the TAB, ESCAPE and ENTER special characters only!
> 
> Adding unknown fields or any special characters that might pose a security risk while parsing are taken care of
> 
> However, if there are still any form of vulnerabilities or unexpected results, feel free to open an issue!

## Installation

There is currently no means of installing this on any Linux distro other than building it from source.

If you encounter any issues with building Litemus from this repository in any distro, feel free to create an issue.

> **Tip:**
> 
> LiteMus can only dynamically window based on the initial window sizes 
> 
> Hence, it is noted that it still has some major dynamic resizing issues
> 
> BEST RESULTS are seen on: 1920x1080 screen size 

## Future

- [x] Have clean windows for showing session details, lyrics view
- [ ] Possibly have an audio visualizer (text based) integrated (POSSIBLY ANOTHER PROJECT)
- [x] Make the code more modular and easy to read, modify
- [x] Add other audio formats (.wav, .flac)
- [x] Have custom keybinds

## LICENSE

This project is licensed under `GNU GPL v3`
