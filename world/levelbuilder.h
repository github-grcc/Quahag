#ifndef LEVELBUILDER_H
#define LEVELBUILDER_H

#include <QPointF>

class GameWorld;
class Player;

class LevelBuilder
{
public:
    struct BuildResult {
        Player *player{nullptr};
        QPointF playerSpawn;
    };

    BuildResult build(GameWorld &world) const;
};

#endif // LEVELBUILDER_H
