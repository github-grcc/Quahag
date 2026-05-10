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

    // Don't call removeItem() — in Qt 6.10 it may defer internal
    // cleanup asynchronously, and the rendering pipeline can still
    // find the item in pending index updates.
    //
    // Instead, just hide the item. Qt will skip hidden items during
    // rendering (drawSubtreeRecursive checks isVisible()). The actual
    // scene removal happens in ~QGraphicsItem() when the zero-timer
    // deletes the entity.
    entity->setVisible(false);
    entity->setEnabled(false);
}
