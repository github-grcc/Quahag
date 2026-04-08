#pragma once

#include "core/inputstate.h"
#include<QVector>
#include <QtGlobal>

class GameWorld;
struct CameraShakeEvent{
    qreal amplitude{20.0};
    qreal duration{0.5};
    qreal frequency{20.0};
};
struct CameraZoomPulseEvent{
    qreal amplitude{0.5};
    qreal duration{2.4};
    qreal cycles{2.0};
    qreal center{1.5};
    qreal initialPhase{1.5*M_PI};
};

struct WorldEvents{
    QVector<CameraShakeEvent> cameraShakes;
    QVector<CameraZoomPulseEvent> cameraZoomPulses;
};
struct TickContext
{
    qreal dt{0.0};
    GameWorld *world{nullptr};
    const InputState *input{nullptr};
    qreal gravity{0.0};
    WorldEvents *events{nullptr};
};
