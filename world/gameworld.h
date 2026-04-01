#pragma once
#include<QObject>
#include<QVector>
#include<QHash>
#include<QPointer>
#include"entities/actoritem.h"
#include"core/tickcontext.h"
#include"core/entitytypes.h"
class ActorItem;
class Player;

class GameWorld:public QObject{
    Q_OBJECT
public:
    explicit GameWorld(QObject *parent=nullptr);
    void step(TickContext &ctx);
    void spawn(ActorItem *entity);
    void destory(ActorItem *entity);
    void destoryLater(ActorItem *entity);

    const TileMap &tileMap()const{return m_tileMap;}
    TileMap &tileMap(){return m_tileMap;}
    const QVector<ActorItem*> &entities()const{return m_entities;}
    QVector<ActorItem*> &entitiesOfKind(EntityKind kind)const;
    QVector<ActorItem*> &entitiesOfFaction(Faction faction)const;
    Player *player()const;
signals:
    void entitySpawned(ActorItem *entity);
    void entityAboutToBeDestroyed(ActorItem *entity);
    
private:
    void flushSpawns();
    void flushDestroys();
    void indexEntity(ActorItem *entity);
    void unindexEntity(ActorItem *entity);

    TileMap m_tileMap;
    
    QVector<ActorItem*> m_entities;
    QVector<ActorItem*> m_pendingSpawn;
    QVector<ActorItem*> m_pendingDestory;
    
    QHash<EntityKind,QVector<ActorItem*>> m_byKind;
    QHash<Faction,QVector<ActorItem*>> m_byFaction;

    QPointer<Player> m_player;


};