#include "world/gamescene.h"

#include "entities/actoritem.h"
#include "graphics/tilelayeritem.h"
#include "world/gameworld.h"
#include "world/levelbuilder.h"

GameScene::GameScene(QObject *parent)
    : QGraphicsScene(parent)
{
}

void GameScene::attachWorld(GameWorld *world)
{
    if (m_world == world)
        return;

    if (m_world)
        disconnect(m_world, nullptr, this, nullptr);

    m_world = world;
    rebuildScene();

    if (!m_world)
        return;

    connect(m_world, &GameWorld::entitySpawned, this, &GameScene::addEntityItem);
    connect(m_world, &GameWorld::entityAboutToBeDestroyed, this, &GameScene::removeEntityItem);
}

Player *GameScene::player() const
{
    return m_world ? m_world->player() : nullptr;
}

void GameScene::rebuildScene()
{
    clear();
    if (!m_world)
        return;

    setSceneRect(m_world->tileMap().sceneBounds());

    auto *tileLayer = new TileLayerItem(m_world->tileMap());
    tileLayer->setZValue(0.0);
    addItem(tileLayer);

    for (ActorItem *entity : m_world->entities())
        addEntityItem(entity);
}

void GameScene::addEntityItem(ActorItem *entity)
{
    if (!entity || entity->scene() == this)
        return;

    addItem(entity);
}

void GameScene::removeEntityItem(ActorItem *entity)
{
    if (!entity || entity->scene() != this)
        return;

    removeItem(entity);
}
