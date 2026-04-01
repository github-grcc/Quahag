#pragma once
#include"world/tilemap.h"
#include"core/inputstate.h"
#include<QtGlobal>
struct TickContext{
    qreal dt{0.0};
    const TileMap &tileMap;
    class GameWorld *world{nullptr};
    const InputState *input{nullptr};
    qreal gravity;
};