#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <gsl/gsl>
#include <rapidjson/document.h>

#include "Map.h"

MapLayer::MapLayer(const std::string type, const std::string file) : type(type), file(file) {
}

Image MapLayer::image() {
    return read_png(this->file);
}

PlayerSpawn::PlayerSpawn(int x, int y) : x(x), y(y) {
}

Map::Map(const std::string &name) : name(name) {

    std::string path = "assets/maps/" + name + "/map.json";
    std::ifstream file;
    std::stringstream buffer;
    file.open(path);
    if (!file) {
        throw std::runtime_error("no such map");
    }

    buffer << file.rdbuf();

    rapidjson::Document doc;
    doc.Parse(buffer.str().c_str());

    if (doc.HasParseError()) {
        throw std::runtime_error("bad json");
    }

    {
        const rapidjson::Value &player_spawns = doc["player_spawns"];
        if (!player_spawns.IsArray()) {
            throw std::runtime_error("spawn points should be array");
        }

        for (auto &sub : player_spawns.GetArray()) {
            if (!sub.IsArray() || sub.Size() != 2) {
                continue;
            }

            auto ps = std::make_shared<PlayerSpawn>(PlayerSpawn(sub[0].GetInt(), sub[1].GetInt()));

            this->player_spawns.push_back(ps);
        }
    }

    {
        const rapidjson::Value &layers = doc["layers"];
        if (!layers.IsArray()) {
            throw std::runtime_error("layers should be array");
        }

        for (auto &sub : layers.GetArray()) {
            if (!sub.IsObject()) {
                continue;
            }

            auto ml = std::make_shared<MapLayer>(
                    MapLayer(sub["type"].GetString(),
                             "assets/maps/" + name + "/" + sub["file"].GetString()));

            this->layers.push_back(ml);
        }
    }

    {
        const rapidjson::Value &width = doc["width"];
        if (!width.IsInt()) {
            throw std::runtime_error("width not int");
        }

        this->width = width.GetInt();
    }

    {
        const rapidjson::Value &height = doc["height"];
        if (!height.IsInt()) {
            throw std::runtime_error("height not int");
        }

        this->height = height.GetInt();
    }
}

std::shared_ptr<MapLayer> Map::get_background() {
    for (auto const &layer : this->layers) {
        if (layer->type == "background") {
            return layer;
        }
    }

    throw std::runtime_error("no background layer");
}

std::shared_ptr<MapLayer> Map::get_dynamic() {
    for (auto const &layer : this->layers) {
        if (layer->type == "dynamic") {
            return layer;
        }
    }

    throw std::runtime_error("no dynamic layer");
}

std::shared_ptr<PlayerSpawn> Map::get_player_spawn(uint16_t seed) {
    auto ind = seed % this->player_spawns.size();
    return gsl::at(this->player_spawns, ind);
}

int Map::get_width() {
    return this->width;
}

int Map::get_height() {
    return this->height;
}
