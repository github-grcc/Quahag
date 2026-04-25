#include "world/levelbuilder.h"

#include "entities/player.h"
#include"entities/enemy.h"
#include "world/gameworld.h"
#include "world/tilemap.h"

LevelBuilder::BuildResult LevelBuilder::build(GameWorld &world) const
{
    BuildResult result;
    const TileMap &tileMap = world.tileMap();
    result.playerSpawn = tileMap.playerSpawnScenePosition();

    auto *player = world.createEntity<Player>();
    player->setPos(result.playerSpawn);
    player->setZValue(2.0);

    for(int j=0;j<tileMap.mapHeight();j++){
        for(int i=0;i<tileMap.mapWidth();i++){
            if(tileMap.tileAt(j,i)==TileMap::TileType::EnemySpawn){
                auto *enemy = world.createEntity<Enemy>();
                enemy->setPos(tileMap.tileCenterToScene(j,i));
                enemy->setZValue(2.0);
            }
        }
    }
    result.player = player;
    return result;
}
