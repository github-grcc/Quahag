#include "world/gameworld.h"

#include "entities/player.h"
#include <QGraphicsScene>

GameWorld::GameWorld(QObject *parent)
    : QObject(parent)
{
}

GameWorld::~GameWorld()
{
    clearAllEntities();
}

Player *GameWorld::player() const
{
    return m_player;
}

QVector<ActorItem *> GameWorld::entities() const
{
    QVector<ActorItem *> result;
    result.reserve(static_cast<int>(m_entities.size()));
    for (const auto &ptr : m_entities) {
        if (ptr && !ptr->pendingDestroy())
            result.append(ptr.get());
    }
    return result;
}

const QVector<ActorItem *> &GameWorld::entitiesOfKind(EntityKind kind) const
{
    static const QVector<ActorItem *> kEmptyEntities;
    const auto it = m_entitiesByKind.constFind(kind);
    return it != m_entitiesByKind.cend() ? it.value() : kEmptyEntities;
}

const QVector<ActorItem *> &GameWorld::entitiesOfFaction(Faction faction) const
{
    static const QVector<ActorItem *> kEmptyEntities;
    const auto it = m_entitiesByFaction.constFind(faction);
    return it != m_entitiesByFaction.cend() ? it.value() : kEmptyEntities;
}

void GameWorld::step(const TickContext &ctx)
{
    flushSpawns();

    const QVector<ActorItem *> currentEntities = entities();
    for (ActorItem *entity : currentEntities) {
        if (!entity || entity->pendingDestroy())
            continue;
        entity->tick(ctx);
    }

    flushDestroys();
}

void GameWorld::destroyLater(ActorItem *entity)
{
    if (!entity || entity->pendingDestroy())
        return;

    entity->markPendingDestroy();
    m_pendingDestroy.append(entity);
}

void GameWorld::flushSpawns()
{
    for (auto &ptr : m_pendingSpawn) {
        if (!ptr)
            continue;

        ActorItem *raw = ptr.get();
        m_entities.push_back(std::move(ptr));
        indexEntity(raw);
        emit entitySpawned(raw);
    }

    m_pendingSpawn.clear();
}

void GameWorld::flushDestroys()
{
    if (m_pendingDestroy.isEmpty())
        return;

    const QVector<ActorItem *> toDestroy = std::move(m_pendingDestroy);
    m_pendingDestroy.clear();

    for (ActorItem *entity : toDestroy) {
        if (!entity)
            continue;

        emit entityAboutToBeDestroyed(entity);
        unindexEntity(entity);

        QGraphicsScene *scene = entity->scene();
        if (scene) {
            scene->removeItem(entity);
        }

        auto it = std::find_if(m_entities.begin(), m_entities.end(),
                               [entity](const std::unique_ptr<ActorItem> &ptr) {
                                   return ptr.get() == entity;
                               });
        if (it != m_entities.end()) {
            m_entities.erase(it);
        }
    }
}

void GameWorld::indexEntity(ActorItem *entity)
{
    if (!entity)
        return;

    m_entitiesByKind[entity->kind()].append(entity);
    m_entitiesByFaction[entity->faction()].append(entity);

    if (entity->kind() == EntityKind::Player)
        m_player = qobject_cast<Player *>(entity);
}

void GameWorld::unindexEntity(ActorItem *entity)
{
    if (!entity)
        return;

    auto kindIt = m_entitiesByKind.find(entity->kind());
    if (kindIt != m_entitiesByKind.end()) {
        kindIt.value().removeOne(entity);
        if (kindIt.value().isEmpty())
            m_entitiesByKind.erase(kindIt);
    }

    auto factionIt = m_entitiesByFaction.find(entity->faction());
    if (factionIt != m_entitiesByFaction.end()) {
        factionIt.value().removeOne(entity);
        if (factionIt.value().isEmpty())
            m_entitiesByFaction.erase(factionIt);
    }

    if (entity == m_player)
        m_player.clear();
}

void GameWorld::clearAllEntities()
{
    m_pendingSpawn.clear();
    m_pendingDestroy.clear();

    for (auto &ptr : m_entities) {
        if (ptr) {
            QGraphicsScene *scene = ptr->scene();
            if (scene) {
                scene->removeItem(ptr.get());
            }
        }
    }

    m_entitiesByKind.clear();
    m_entitiesByFaction.clear();
    m_player.clear();
    m_entities.clear();
}
