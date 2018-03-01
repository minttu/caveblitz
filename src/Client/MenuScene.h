#ifndef CAVEBLITZ_CLIENT_MENU_SCENE_H
#define CAVEBLITZ_CLIENT_MENU_SCENE_H

#include "Scene.h"

class MenuScene : public Scene {
public:
    explicit MenuScene(Game *game);
    std::string name() const override;
    bool tick(DeltaTime dt, std::vector<Input> inputs) override;
    void switched_to() override;
    void switched_from() override;

    static Scene *ref(Game *game) {
        return new MenuScene(game);
    }
};

#endif // CAVEBLITZ_CLIENT_MENU_SCENE_H
