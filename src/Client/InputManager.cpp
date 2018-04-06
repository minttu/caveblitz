#include "InputManager.h"
#include <iostream>

InputManager::InputManager() {
    InputSeat seat1{};
    seat1.seat_type = INPUT_SEAT_KEYBOARD;
    seat1.keyboard_layout = KEYBOARD_ARROWS;
    seat1.active = true;

    this->seats.push_back(seat1);

    InputSeat seat2{};
    seat2.seat_type = INPUT_SEAT_KEYBOARD;
    seat2.keyboard_layout = KEYBOARD_WASD;

    this->seats.push_back(seat2);
}

void InputManager::player_inputs(std::vector<Input> *inputs) {
    inputs->clear();

    this->gather_keys_held();

    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i) && this->opened_controllers.count(i) == 0) {
            SDL_GameController *controller = SDL_GameControllerOpen(i);
            if (controller != nullptr) {
                this->opened_controllers.emplace(i);
                InputSeat seat{};
                seat.seat_type = INPUT_SEAT_CONTROLLER;
                seat.controller = controller;

                this->seats.push_back(seat);
            }
        }
    }

    for (InputSeat &seat : this->seats) {
        if (seat.active) {
            inputs->push_back(this->seat_input(seat));
        } else {
            seat.active = this->seat_activate(seat);
        }
    }
}

Input InputManager::keyboard_input(const SDL_Scancode layout[6]) {
    Input playerInput{};

    playerInput.up = this->keys_held[layout[0]];
    playerInput.down = this->keys_held[layout[1]];
    playerInput.right = this->keys_held[layout[2]];
    playerInput.left = this->keys_held[layout[3]];

    playerInput.thrust = static_cast<int8_t>(playerInput.up * 127);
    playerInput.rotation = static_cast<int8_t>(playerInput.left * 127 - playerInput.right * 127);

    playerInput.primary = this->keys_held[layout[4]];
    playerInput.special = this->keys_held[layout[5]];
    playerInput.back = this->keys_held[SDL_SCANCODE_BACKSPACE];

    return playerInput;
}

Input InputManager::controller_input(SDL_GameController *controller) {
    Input playerInput{};

    playerInput.up = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
    playerInput.down = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
    playerInput.right = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
    playerInput.left = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);

    playerInput.thrust = static_cast<int8_t>(
            SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A) * 127);
    playerInput.rotation = static_cast<int8_t>(
            SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX) / 256);

    playerInput.primary =
            SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X) |
            SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
    playerInput.special =
            SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y) |
            SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
    playerInput.back = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B);

    return playerInput;
}

bool InputManager::keyboard_activate(const SDL_Scancode layout[6]) {
    return this->keys_held[layout[4]];
}

bool InputManager::controller_activate(SDL_GameController *controller) {
    return SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X);
}

bool InputManager::seat_activate(InputSeat seat) {
    return (seat.seat_type == INPUT_SEAT_KEYBOARD
                    ? keyboard_activate(KEYBOARD_LAYOUTS[seat.keyboard_layout])
                    : controller_activate(seat.controller));
}

Input InputManager::seat_input(InputSeat seat) {
    return (seat.seat_type == INPUT_SEAT_KEYBOARD
                    ? keyboard_input(KEYBOARD_LAYOUTS[seat.keyboard_layout])
                    : controller_input(seat.controller));
}

void InputManager::gather_keys_held() {
    SDL_Event e{};

    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            this->should_quit = true;
        }
        if (e.type == SDL_KEYDOWN) {
            this->keys_held[e.key.keysym.scancode] = true;
        }
        if (e.type == SDL_KEYUP) {
            this->keys_held[e.key.keysym.scancode] = false;
        }
    }
}
