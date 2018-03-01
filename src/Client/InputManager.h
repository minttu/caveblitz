#ifndef CAVEBLITZ_CLIENT_INPUT_MANAGER_H
#define CAVEBLITZ_CLIENT_INPUT_MANAGER_H

#include <cstdint>
#include <unordered_map>
#include <vector>

#include <SDL2/SDL_events.h>

struct Input {
    uint8_t seat;
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
    std::unordered_map<int32_t, bool> keys_held;

    Input keyboard_input();

public:
    InputManager();

    bool should_quit = false;

    void gather_keys_held();

    void player_inputs(std::vector<Input> *inputs);
};

#endif // CAVEBLITZ_CLIENT_INPUT_MANAGER_H
