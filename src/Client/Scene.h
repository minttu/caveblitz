#ifndef CAVEBLITZ_CLIENT_SCENE_H
#define CAVEBLITZ_CLIENT_SCENE_H

#include "FPSManager.h"

class Scene {
public:
    virtual bool tick(DeltaTime dt) = 0;
};

#endif // CAVEBLITZ_CLIENT_SCENE_H
