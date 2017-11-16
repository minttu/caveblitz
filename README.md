# caveblitz

Game where you fly and shoot in a cave.

|   |   |
| - | - |
| linux | [![CircleCI](https://circleci.com/gh/minttu/caveblitz/tree/master.svg?style=svg&circle-token=7eed1db17b9da6e44a085a2de652d81d9d41d50e)](https://circleci.com/gh/minttu/caveblitz/tree/master) |
| windows | [![AppVeyor](https://ci.appveyor.com/api/projects/status/yuahv4fvadkpnh17/branch/master?svg=true)](https://ci.appveyor.com/project/minttu/caveblitz/branch/master)

## requirements

* C++14 capable compiler
* `cmake 3.6`
* `libpng 1.6`
* `SDL2`
* `SDL2_image`
* `SDL2_ttf`
* `SDL2_mixer`
* `enet 1.3`
* `git submodule update --init --recursive`

## building

* `mkdir -p build`
* `cd build`
* `cmake ..`
* `make`

## running

To start a new game & server

    ./caveblitz -s

To join existing game

    ./caveblitz -c 192.168.1.100

## controls

* **Left, right and up arrows**: movement
* **Right shift**: firing the primary weapon
* **Down arrow**: start game (all connected players need to press this), firing the secondary weapon
