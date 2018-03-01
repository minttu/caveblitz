#ifndef CAVEBLITZ_CLIENT_SCENE_H
#define CAVEBLITZ_CLIENT_SCENE_H

#include <string>
#include <vector>

#include "FPSManager.h"

class Game;
class Input;

class Scene {
protected:
    Game *game;

public:
    explicit Scene(Game *game) : game(game){};

    virtual std::string name() const = 0;
    virtual bool tick(DeltaTime dt, std::vector<Input> inputs) = 0;
    virtual void switched_to() = 0;
    virtual void switched_from() = 0;
};

#include "Game.h"

#endif // CAVEBLITZ_CLIENT_SCENE_H
