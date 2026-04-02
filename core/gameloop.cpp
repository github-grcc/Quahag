#include "core/gameloop.h"

#include "core/tickcontext.h"
#include "world/gameworld.h"

namespace {
constexpr qreal kGravity = 1800.0;
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

    TickContext ctx;
    ctx.dt = m_elapsedTimer.restart() / 1000.0;
    ctx.world = m_world;
    ctx.input = m_input;
    ctx.gravity = kGravity;
    m_world->step(ctx);
    emit stepped(ctx.dt);
}
