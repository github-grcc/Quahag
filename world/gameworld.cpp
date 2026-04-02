#include "world/gameworld.h"

#include "entities/player.h"

GameWorld::GameWorld(QObject *parent)
    : QObject(parent)
{
}

Player *GameWorld::player() const
{
    return m_player;
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

    const QVector<ActorItem *> currentEntities = m_entities;
    for (ActorItem *entity : currentEntities) {
        if (!entity || entity->pendingDestroy())
            continue;
        entity->tick(ctx);
    }

    flushDestroys();
}

void GameWorld::spawn(ActorItem *entity)
{
    if (!entity)
        return;

    m_pendingSpawn.append(entity);
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
    for (ActorItem *entity : std::as_const(m_pendingSpawn)) {
        if (!entity)
            continue;

        m_entities.append(entity);
        indexEntity(entity);
        emit entitySpawned(entity);
    }

    m_pendingSpawn.clear();
}

void GameWorld::flushDestroys()
{
    for (ActorItem *entity : std::as_const(m_pendingDestroy)) {
        if (!entity)
            continue;

        emit entityAboutToBeDestroyed(entity);
        m_entities.removeOne(entity);
        unindexEntity(entity);
        delete entity;
    }

    m_pendingDestroy.clear();
}

void GameWorld::indexEntity(ActorItem *entity)
{
    m_entitiesByKind[entity->kind()].append(entity);
    m_entitiesByFaction[entity->faction()].append(entity);

    if (entity->kind() == EntityKind::Player)
        m_player = qobject_cast<Player *>(entity);
}

void GameWorld::unindexEntity(ActorItem *entity)
{
    m_entitiesByKind[entity->kind()].removeOne(entity);
    m_entitiesByFaction[entity->faction()].removeOne(entity);

    if (entity == m_player)
        m_player = nullptr;
}
