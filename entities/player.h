#ifndef PLAYER_H
#define PLAYER_H

#include "core/inputstate.h"
#include "entities/actoritem.h"

#include <QRectF>

class TileMap;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

class Player : public ActorItem
{
    Q_OBJECT
public:
    Player();

    void setInput(const InputState &input);
    void tick(qreal dt, const TileMap &tileMap, qreal gravity) override;

    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

private:
    void moveHorizontally(qreal dt, const TileMap &tileMap);
    void moveVertically(qreal dt, const TileMap &tileMap, qreal gravity);
    void resolveTileCollisionsX(const TileMap &tileMap);
    void resolveTileCollisionsY(const TileMap &tileMap);

    InputState m_input;
    QRectF m_bodyRect{-12.0, -24.0, 24.0, 48.0};
};

#endif // PLAYER_H
