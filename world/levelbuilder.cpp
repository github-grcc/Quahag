#include "world/levelbuilder.h"

#include "entities/player.h"
#include "world/gameworld.h"
#include "world/tilemap.h"

LevelBuilder::BuildResult LevelBuilder::build(GameWorld &world) const
{
    BuildResult result;
    const TileMap &tileMap = world.tileMap();
    result.playerSpawn = tileMap.playerSpawnScenePosition();

    auto *player = new Player();
    player->setPos(result.playerSpawn);
    player->setZValue(2.0);
    world.spawn(player);
    result.player = player;
    return result;
}
