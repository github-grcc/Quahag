#include "world/gamescene.h"

#include "entities/actoritem.h"
#include "graphics/tilelayeritem.h"
#include "world/gameworld.h"
#include "world/levelbuilder.h"

GameScene::GameScene(QObject *parent)
    : QGraphicsScene(parent)
{
}

GameScene::~GameScene()
{
    if (m_world) {
        disconnect(m_world, nullptr, this, nullptr);
    }
    // Base ~QGraphicsScene() calls clear() which deletes all items
    // (tile layer + any entities released by ~GameWorld).
}

void GameScene::attachWorld(GameWorld *world)
{
    if (m_world == world)
        return;

    if (m_world) {
        disconnect(m_world, nullptr, this, nullptr);
    }

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
    clear(); // Deletes all scene-owned items (tile layer)

    if (!m_world)
        return;

    setSceneRect(m_world->tileMap().sceneBounds());

    m_tileLayer = new TileLayerItem(&m_world->tileMap());
    m_tileLayer->setZValue(ZLayer::Background);
    addItem(m_tileLayer);

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
    if (!entity)
        return;

    if (entity->scene() == this)
        removeItem(entity);
}
