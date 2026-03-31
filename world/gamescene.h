#ifndef GAMESCENE_H
#define GAMESCENE_H
#include <QElapsedTimer>
#include <QGraphicsScene>
#include <QTimer>

#include "core/inputstate.h"
#include "entities/actoritem.h"
#include "entities/player.h"
#include "world/tilemap.h"

class GameScene:public QGraphicsScene{
    Q_OBJECT
public:
    explicit GameScene(QObject *parent=nullptr);
    void setInput(const InputState &input);
    Player *player()const{return m_player;}
signals:
    void playerMoved(qreal dt);
private slots:
    void tick();

private:
    void initWorld();
    void registerActor(ActorItem *actor);

    Player *m_player{nullptr};
    TileMap m_tileMap;
    QList<ActorItem *> m_actors;
    InputState m_input;
    QTimer m_timer;
    QElapsedTimer m_frameTimer;
};

#endif // GAMESCENE_H
