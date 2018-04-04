#include "InputManager.h"
#include <iostream>

InputManager::InputManager() = default;

void InputManager::player_inputs(std::vector<Input> *inputs) {
    inputs->clear();

    this->gather_keys_held();

    inputs->push_back(this->keyboard_arrow_input());
    inputs->push_back(this->keyboard_wasd_input());
}

Input InputManager::keyboard_arrow_input() {
    Input playerInput{};

    playerInput.seat = 1;

    playerInput.up = this->keys_held[SDLK_UP];
    playerInput.down = this->keys_held[SDLK_DOWN];
    playerInput.right = this->keys_held[SDLK_RIGHT];
    playerInput.left = this->keys_held[SDLK_LEFT];

    playerInput.thrust = static_cast<int8_t>(playerInput.up * 127);
    playerInput.rotation = static_cast<int8_t>(playerInput.left * 127 - playerInput.right * 127);

    playerInput.primary = this->keys_held[SDLK_RSHIFT];
    playerInput.special = this->keys_held[SDLK_DOWN];
    playerInput.back = this->keys_held[SDLK_ESCAPE];

    return playerInput;
}

Input InputManager::keyboard_wasd_input() {
    Input playerInput{};

    playerInput.seat = 2;

    playerInput.up = this->keys_held[SDLK_w];
    playerInput.down = this->keys_held[SDLK_s];
    playerInput.right = this->keys_held[SDLK_d];
    playerInput.left = this->keys_held[SDLK_a];

    playerInput.thrust = static_cast<int8_t>(playerInput.up * 127);
    playerInput.rotation = static_cast<int8_t>(playerInput.left * 127 - playerInput.right * 127);

    playerInput.primary = this->keys_held[SDLK_LSHIFT];
    playerInput.special = this->keys_held[SDLK_s];
    playerInput.back = this->keys_held[SDLK_ESCAPE];

    return playerInput;
}

void InputManager::gather_keys_held() {
    SDL_Event e{};

    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            this->should_quit = true;
        }
        if (e.type == SDL_KEYDOWN) {
            this->keys_held[e.key.keysym.sym] = true;
        }
        if (e.type == SDL_KEYUP) {
            this->keys_held[e.key.keysym.sym] = false;
        }
    }
}
