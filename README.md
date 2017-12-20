# caveblitz

[![CircleCI](https://img.shields.io/circleci/project/github/minttu/caveblitz/master.svg?style=flat&label=linux%20build)](https://circleci.com/gh/minttu/caveblitz/tree/master)
[![AppVeyor](https://img.shields.io/appveyor/ci/minttu/caveblitz.svg?style=flat&label=windows%20build)](https://ci.appveyor.com/project/minttu/caveblitz/branch/master)

Game where you fly and shoot in a cave.

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
