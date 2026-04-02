#pragma once

#include "core/inputstate.h"

#include <QtGlobal>

class GameWorld;

struct TickContext
{
    qreal dt{0.0};
    GameWorld *world{nullptr};
    const InputState *input{nullptr};
    qreal gravity{0.0};
};
