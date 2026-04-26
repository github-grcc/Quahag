#include "world/gameworld.h"

#include "entities/player.h"

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
    if (!m_entitiesDirty)
        return m_cachedEntities;

    m_cachedEntities.clear();
    m_cachedEntities.reserve(static_cast<int>(m_entities.size()));
    for (const auto &ptr : m_entities) {
        if (ptr && !ptr->pendingDestroy())
            m_cachedEntities.append(ptr.get());
    }
    m_entitiesDirty = false;
    return m_cachedEntities;
}

QVector<ActorItem *> GameWorld::entitiesOfKind(EntityKind kind) const
{
    QVector<ActorItem *> result;
    const auto it = m_entitiesByKind.constFind(kind);
    if (it != m_entitiesByKind.cend()) {
        result.reserve(it.value().size());
        for (ActorItem *e : it.value()) {
            if (e && !e->pendingDestroy())
                result.append(e);
        }
    }
    return result;
}

QVector<ActorItem *> GameWorld::entitiesOfFaction(Faction faction) const
{
    QVector<ActorItem *> result;
    const auto it = m_entitiesByFaction.constFind(faction);
    if (it != m_entitiesByFaction.cend()) {
        result.reserve(it.value().size());
        for (ActorItem *e : it.value()) {
            if (e && !e->pendingDestroy())
                result.append(e);
        }
    }
    return result;
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
    Q_ASSERT(!m_inFlushDestroys);
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
    m_entitiesDirty = true;
}

void GameWorld::flushDestroys()
{
    if (m_pendingDestroy.isEmpty())
        return;

    const QVector<ActorItem *> toDestroy = std::move(m_pendingDestroy);
    m_pendingDestroy.clear();

    m_inFlushDestroys = true;
    for (ActorItem *entity : toDestroy) {
        if (!entity)
            continue;

        emit entityAboutToBeDestroyed(entity);
        unindexEntity(entity);

        auto it = std::find_if(m_entities.begin(), m_entities.end(),
                               [entity](const std::unique_ptr<ActorItem> &ptr) {
                                   return ptr.get() == entity;
                               });
        if (it != m_entities.end()) {
            m_entities.erase(it);
        }
    }
    m_entitiesDirty = true;
    m_inFlushDestroys = false;
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
            emit entityAboutToBeDestroyed(ptr.get());
            unindexEntity(ptr.get());
        }
    }

    m_entitiesByKind.clear();
    m_entitiesByFaction.clear();
    m_player.clear();
    m_entities.clear();
    m_entitiesDirty = true;
}
