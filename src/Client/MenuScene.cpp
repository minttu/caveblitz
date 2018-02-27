#include "MenuScene.h"

MenuScene::MenuScene(Game *game) : Scene(game) {
}

bool MenuScene::tick(DeltaTime dt) {
    return true;
}

std::string MenuScene::name() const {
    return "menu";
}

void MenuScene::switched_to() {
}

void MenuScene::switched_from() {
}
