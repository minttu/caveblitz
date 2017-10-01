#include <SDL2/SDL.h>

#undef main

#include "Client/Game.h"

int main() {
    Game game;
    game.run();

    return 0;
}
