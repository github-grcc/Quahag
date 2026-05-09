#include "world/levelbuilder.h"

#include "entities/player.h"
#include "entities/enemy.h"
#include "entities/throne.h"
#include "entities/hachimi.h"
#include "entities/chutty.h"
#include "world/gameworld.h"
#include "world/tilemap.h"

LevelBuilder::BuildResult LevelBuilder::build(GameWorld &world) const
{
    BuildResult result;
    const TileMap &tileMap = world.tileMap();
    result.playerSpawn = tileMap.playerSpawnScenePosition();

    auto *player = world.createEntity<Player>();
    player->setPos(result.playerSpawn);
    player->setZValue(ZLayer::Player);

    for(int j=0;j<tileMap.mapHeight();j++){
        for(int i=0;i<tileMap.mapWidth();i++){
            if(tileMap.tileAt(j,i)==TileMap::TileType::EnemySpawn){
                auto *enemy = world.createEntity<Enemy>();
                enemy->setPos(tileMap.tileCenterToScene(j,i));
                enemy->setZValue(ZLayer::Enemies);
            }
            if(tileMap.tileAt(j,i)==TileMap::TileType::ThroneSpawn){
                auto *throne = world.createEntity<Throne>();
                throne->setPos(tileMap.tileCenterToScene(j,i));
                throne->setZValue(ZLayer::Enemies);
            }
            if(tileMap.tileAt(j,i)==TileMap::TileType::HachimiSpawn){
                auto *hachimi = world.createEntity<Hachimi>();
                hachimi->setPos(tileMap.tileCenterToScene(j,i));
                hachimi->setZValue(ZLayer::Items);
            }
            if(tileMap.tileAt(j,i)==TileMap::TileType::ChuttySpawn){
                auto *chutty = world.createEntity<Chutty>();
                chutty->setPos(tileMap.tileCenterToScene(j,i));
                chutty->setZValue(ZLayer::Items);
            }
        }
    }
    result.player = player;
    return result;
}
