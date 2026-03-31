#include "world/levelbuilder.h"

#include "entities/player.h"
#include "graphics/tilelayeritem.h"
#include "world/tilemap.h"

#include <QGraphicsScene>

LevelBuilder::BuildResult LevelBuilder::build(QGraphicsScene &scene, const TileMap &tileMap) const
{
    auto *tileLayer = new TileLayerItem(tileMap);
    tileLayer->setZValue(0.0);
    scene.addItem(tileLayer);

    BuildResult result;
    result.playerSpawn = tileMap.playerSpawnScenePosition();

    auto *player = new Player();
    player->setPos(result.playerSpawn);
    player->setZValue(2.0);
    scene.addItem(player);
    result.player = player;
    return result;
}
