#ifndef CAVEBLITZ_CLIENT_INPUT_MANAGER_H
#define CAVEBLITZ_CLIENT_INPUT_MANAGER_H

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <SDL2/SDL_events.h>

enum KeyboardLayoutName { KEYBOARD_NONE = -1, KEYBOARD_ARROWS = 0, KEYBOARD_WASD = 1 };

const static SDL_Scancode KEYBOARD_LAYOUTS[][6] = {{SDL_SCANCODE_UP,
                                                    SDL_SCANCODE_DOWN,
                                                    SDL_SCANCODE_RIGHT,
                                                    SDL_SCANCODE_LEFT,
                                                    SDL_SCANCODE_RSHIFT,
                                                    SDL_SCANCODE_DOWN},
                                                   {SDL_SCANCODE_W,
                                                    SDL_SCANCODE_S,
                                                    SDL_SCANCODE_D,
                                                    SDL_SCANCODE_A,
                                                    SDL_SCANCODE_LSHIFT,
                                                    SDL_SCANCODE_S}};

enum InputSeatType { INPUT_SEAT_KEYBOARD, INPUT_SEAT_CONTROLLER };

struct InputSeat {
    InputSeatType seat_type;
    SDL_GameController *controller = nullptr;
    KeyboardLayoutName keyboard_layout = KEYBOARD_NONE;
    bool active = false;
};

struct Input {
    bool up;
    bool down;
    bool right;
    bool left;
    int8_t rotation;
    int8_t thrust;
    bool primary;
    bool special;
    bool back;
};

class InputManager {
private:
    std::vector<InputSeat> seats;
    std::unordered_map<int32_t, bool> keys_held;
    std::unordered_set<int> opened_controllers;

    Input keyboard_input(const SDL_Scancode layout[6]);

    Input controller_input(SDL_GameController *controller);

    bool keyboard_activate(const SDL_Scancode layout[6]);

    bool controller_activate(SDL_GameController *controller);

    bool seat_activate(InputSeat seat);

    Input seat_input(InputSeat seat);

public:
    InputManager();

    bool should_quit = false;

    void gather_keys_held();

    void player_inputs(std::vector<Input> *inputs);
};

#endif // CAVEBLITZ_CLIENT_INPUT_MANAGER_H
