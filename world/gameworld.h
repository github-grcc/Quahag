#pragma once

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QVector>

#include "core/entitytypes.h"
#include "core/tickcontext.h"
#include "entities/actoritem.h"
#include "world/tilemap.h"

class ActorItem;
class Player;

class GameWorld : public QObject
{
    Q_OBJECT
public:
    explicit GameWorld(QObject *parent = nullptr);
    void step(const TickContext &ctx);

    void spawn(ActorItem *entity);
    void destroyLater(ActorItem *entity);

    const TileMap &tileMap() const { return m_tileMap; }
    TileMap &tileMap() { return m_tileMap; }
    const QVector<ActorItem *> &entities() const { return m_entities; }
    const QVector<ActorItem *> &entitiesOfKind(EntityKind kind) const;
    const QVector<ActorItem *> &entitiesOfFaction(Faction faction) const;
    Player *player() const;

signals:
    void entitySpawned(ActorItem *entity);
    void entityAboutToBeDestroyed(ActorItem *entity);

private:
    void flushSpawns();
    void flushDestroys();
    void indexEntity(ActorItem *entity);
    void unindexEntity(ActorItem *entity);

    TileMap m_tileMap;

    QVector<ActorItem *> m_entities;
    QVector<ActorItem *> m_pendingSpawn;
    QVector<ActorItem *> m_pendingDestroy;

    QHash<EntityKind, QVector<ActorItem *>> m_entitiesByKind;
    QHash<Faction, QVector<ActorItem *>> m_entitiesByFaction;

    QPointer<Player> m_player;
};
