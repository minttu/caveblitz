#ifndef CAVEBLITZ_COMMON_MAP_H
#define CAVEBLITZ_COMMON_MAP_H

#include <memory>
#include <vector>

#include "Image.h"

class MapLayer {
public:
    const std::string type;
    const std::string file;

    MapLayer(const std::string type, const std::string file);

    Image image();
};

class PlayerSpawn {
public:
    int x;
    int y;

    PlayerSpawn(int x, int y);
};

class Map {
private:
    std::vector<std::shared_ptr<MapLayer>> layers;
    std::vector<std::shared_ptr<PlayerSpawn>> player_spawns;

public:
    const std::string name;

    Map(const std::string &name);
    std::shared_ptr<MapLayer> get_dynamic();
    std::shared_ptr<MapLayer> get_background();
    std::shared_ptr<PlayerSpawn> get_player_spawn(uint16_t seed);
};

#endif // CAVEBLITZ_COMMON_MAP_H
