#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>
#include <QPointer>

#include "world/gameworld.h"

class ActorItem;
class Player;
class TileLayerItem;

class GameScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit GameScene(QObject *parent = nullptr);
    ~GameScene() override;

    void attachWorld(GameWorld *world);
    GameWorld *world() const { return m_world; }
    Player *player() const;

    void rebuildScene();

signals:
    void playerMoved(qreal dt);

private:
    void addEntityItem(ActorItem *entity);
    void removeEntityItem(ActorItem *entity);

    QPointer<GameWorld> m_world;
    QGraphicsItem *m_tileLayer{nullptr};
};

#endif // GAMESCENE_H
