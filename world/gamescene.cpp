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
    //基类~QGraphicsScene()调用clear()删除所有项目(瓦片层+~GameWorld释放的实体)
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
    clear(); //删除场景拥有的所有项目(瓦片层)

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

    //不调用removeItem()——Qt 6.10可能异步延迟内部清理，渲染管线仍可在待处理索引更新中找到该项目。
    //改为隐藏项目，Qt渲染时会跳过隐藏项目(drawSubtreeRecursive检查isVisible())。
    //实际场景移除在零计时器删除实体时由~QGraphicsItem()完成。
    entity->setVisible(false);
    entity->setEnabled(false);
}
