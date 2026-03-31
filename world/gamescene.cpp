#include "world/gamescene.h"
#include "world/levelbuilder.h"

namespace {
constexpr qreal kGravity = 1800.0;
constexpr qreal kTickIntervalMs = 16.0;
}

GameScene::GameScene(QObject *parent) : QGraphicsScene(parent) {
    setSceneRect(m_tileMap.sceneBounds());
    initWorld();
    connect(&m_timer,&QTimer::timeout,this,&GameScene::tick);
    m_frameTimer.start();
    m_timer.start(static_cast<int>(kTickIntervalMs));
}

void GameScene::setInput(const InputState &input){
    m_input=input;
}

void GameScene::tick(){
    if(!m_player)
        return;

    const qreal dt = m_frameTimer.restart() / 1000.0;
    m_player->setInput(m_input);
    for (ActorItem *actor : std::as_const(m_actors)) {
        actor->tick(dt, m_tileMap, kGravity);
    }
    emit playerMoved(dt);
}

void GameScene::initWorld(){
    const LevelBuilder builder;
    const auto buildResult = builder.build(*this, m_tileMap);
    m_player = buildResult.player;
    registerActor(m_player);
}

void GameScene::registerActor(ActorItem *actor)
{
    if (actor && !m_actors.contains(actor))
        m_actors.append(actor);
}
