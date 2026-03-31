#ifndef LEVELBUILDER_H
#define LEVELBUILDER_H

#include <QPointF>

class QGraphicsScene;
class Player;
class TileMap;

class LevelBuilder
{
public:
    struct BuildResult {
        Player *player{nullptr};
        QPointF playerSpawn;
    };

    BuildResult build(QGraphicsScene &scene, const TileMap &tileMap) const;
};

#endif // LEVELBUILDER_H
