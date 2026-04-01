#include"world/gameworld.h"
#include"entities/player.h"
GameWorld::GameWorld(QObject *parent){

}
Player *GameWorld::player()const{
    return m_player;
}

const QVector<ActorItem*> &GameWorld::entitiesOfKind(EntityKind kind) const
{
    static const QVector<ActorItem*> kEmptyEntities;
    const auto it = m_entitiesByKind.constFind(kind);
    return it != m_entitiesByKind.cend() ? it.value() : kEmptyEntities;
}

const QVector<ActorItem*> &GameWorld::entitiesOfFaction(Faction faction) const
{
    static const QVector<ActorItem*> kEmptyEntities;
    const auto it = m_entitiesByFaction.constFind(faction);
    return it != m_entitiesByFaction.cend() ? it.value() : kEmptyEntities;
}

void GameWorld::spawn(ActorItem *entity){
    m_pendingSpawn.append(entity);
}
void GameWorld::flushSpawns(){
    for(ActorItem *entity:std::as_const(m_pendingSpawn)){
        m_entities.append(entity);
        m_entitiesByKind[entity->kind()].append(entity);
        m_entitiesByFaction[entity->faction()].append(entity);

        if(entity->kind()==EntityKind::Player){
            m_player=static_cast<Player*>(entity);
        }
    }
    m_pendingSpawn.clear();
}
void GameWorld::flushDestroys(){
    for(ActorItem *entity:std::as_const(m_pendingDestroy)){
        m_entities.removeOne(entity);
        m_entitiesByKind[entity->kind()].removeOne(entity);
        m_entitiesByFaction[entity->faction()].removeOne(entity);
        
        if(entity==m_player){
            m_player=nullptr;
        }
    }
    m_pendingDestroy.clear();
}
