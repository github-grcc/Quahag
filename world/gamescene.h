#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>
#include <QPointer>

#include "world/gameworld.h"

class ActorItem;
class Player;

class GameScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit GameScene(QObject *parent = nullptr);

    void attachWorld(GameWorld *world);
    GameWorld *world() const { return m_world; }
    Player *player() const;

signals:
    void playerMoved(qreal dt);

private:
    void rebuildScene();
    void addEntityItem(ActorItem *entity);
    void removeEntityItem(ActorItem *entity);

    QPointer<GameWorld> m_world;
};

#endif // GAMESCENE_H
