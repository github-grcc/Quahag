#pragma once

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QVector>
#include <memory>

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
    ~GameWorld() override;

    void step(const TickContext &ctx);

    template<typename T, typename... Args>
    T *createEntity(Args&&... args) {
        static_assert(std::is_base_of_v<ActorItem, T>, "T must derive from ActorItem");
        auto entity = std::make_unique<T>(std::forward<Args>(args)...);
        T *raw = entity.get();
        m_pendingSpawn.push_back(std::move(entity));
        return raw;
    }

    void destroyLater(ActorItem *entity);

    const TileMap &tileMap() const { return m_tileMap; }
    TileMap &tileMap() { return m_tileMap; }

    QVector<ActorItem *> entities() const;
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
    void clearAllEntities();

    TileMap m_tileMap;

    std::vector<std::unique_ptr<ActorItem>> m_entities;
    std::vector<std::unique_ptr<ActorItem>> m_pendingSpawn;
    QVector<ActorItem *> m_pendingDestroy;

    QHash<EntityKind, QVector<ActorItem *>> m_entitiesByKind;
    QHash<Faction, QVector<ActorItem *>> m_entitiesByFaction;

    QPointer<Player> m_player;
};
