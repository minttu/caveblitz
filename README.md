# caveblitz

[![CircleCI](https://circleci.com/gh/minttu/caveblitz/tree/master.svg?style=svg&circle-token=7eed1db17b9da6e44a085a2de652d81d9d41d50e)](https://circleci.com/gh/minttu/caveblitz/tree/master)

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
* `git submodule init; git submodule update`

## building

* `mkdir -p build`
* `cd build`
* `cmake ..`
* `make`
* `cp ../assets assets`
