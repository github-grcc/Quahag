#include "core/gameloop.h"

#include "core/tickcontext.h"
#include "world/gameworld.h"

namespace {
constexpr qreal kGravity = 2000.0;
constexpr int kTickIntervalMs = 16;
}

GameLoop::GameLoop(QObject *parent)
    : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &GameLoop::tick);
}

void GameLoop::setWorld(GameWorld *world)
{
    m_world = world;
}

void GameLoop::setInputState(const InputState *input)
{
    m_input = input;
}

void GameLoop::setWorldPaused(bool paused)
{
    m_worldPaused = paused;
}

void GameLoop::start()
{
    if (!m_elapsedTimer.isValid())
        m_elapsedTimer.start();
    m_timer.start(kTickIntervalMs);
}

void GameLoop::stop()
{
    m_timer.stop();
}

void GameLoop::tick()
{
    if (!m_world)
        return;
    WorldEvents events;
    TickContext ctx;
    ctx.dt = m_elapsedTimer.restart() / 1000.0;
    ctx.world = m_world;
    ctx.input = m_input;
    ctx.gravity = kGravity;
    ctx.events = &events;

    if (!m_worldPaused)
        m_world->step(ctx);

    for (const CameraShakeEvent &shake : events.cameraShakes) {
        emit cameraShakeRequested(shake);
    }
    for (const CameraZoomPulseEvent &zoomPulse : events.cameraZoomPulses) {
        emit cameraZoomPulseRequested(zoomPulse);
        m_zoomPulseActive = true;
    }
    if (events.stopZoomPulseRequested && m_zoomPulseActive) {
        emit cameraZoomPulseStopRequested();
        m_zoomPulseActive = false;
    }
    events.zoomPulseActive = m_zoomPulseActive;
    emit stepped(ctx.dt);
}
