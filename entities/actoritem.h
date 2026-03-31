#ifndef ACTORITEM_H
#define ACTORITEM_H

#include <QGraphicsObject>
#include <QPointF>

class TileMap;

class ActorItem : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit ActorItem(QGraphicsItem *parent = nullptr);

    virtual void tick(qreal dt, const TileMap &tileMap, qreal gravity) = 0;

    QPointF velocity() const { return m_velocity; }
    void setVelocity(const QPointF &velocity) { m_velocity = velocity; }
    bool onGround() const { return m_onGround; }

protected:
    void setOnGround(bool value) { m_onGround = value; }

    QPointF m_velocity{0.0, 0.0};
    bool m_onGround{false};
};

#endif // ACTORITEM_H
