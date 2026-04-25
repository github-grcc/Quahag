#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>
#include <QPointer>
#include <memory>

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

signals:
    void playerMoved(qreal dt);

private:
    void rebuildScene();
    void addEntityItem(ActorItem *entity);
    void removeEntityItem(ActorItem *entity);

    QPointer<GameWorld> m_world;
    std::unique_ptr<QGraphicsItem> m_tileLayer;
};

#endif // GAMESCENE_H
