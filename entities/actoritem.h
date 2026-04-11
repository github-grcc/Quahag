#ifndef ACTORITEM_H
#define ACTORITEM_H

#include <QGraphicsObject>
#include <QPointF>

#include "core/entitytypes.h"
#include "core/tickcontext.h"

class TileMap;

class ActorItem : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit ActorItem(QGraphicsItem *parent = nullptr);
    virtual void tick(const TickContext &ctx) = 0;

    virtual EntityKind kind() const = 0;
    virtual Faction faction() const { return Faction::Neutral; }

    bool pendingDestroy() const { return m_pendingDestroy; }
    void markPendingDestroy() { m_pendingDestroy = true; }

    QPointF velocity() const { return m_velocity; }
    qreal velocityX() const { return m_velocity.x(); }
    qreal velocityY() const { return m_velocity.y(); }

    void setVelocity(const QPointF &velocity) { m_velocity = velocity; }
    void setVelocityX(qreal x) { m_velocity.setX(x); }
    void setVelocityY(qreal y) { m_velocity.setY(y); }

    bool onGround() const { return m_onGround; }
    qreal age()const{return m_age;}
    
protected:
    void setOnGround(bool value) { m_onGround = value; }
    void setAge(qreal age){m_age=age;}
    void advanceAge(qreal dt){m_age+=dt;}
private:
    QPointF m_velocity{0.0, 0.0};
    bool m_onGround{false};
    bool m_pendingDestroy{false};
    qreal m_age{0.0};
};

#endif // ACTORITEM_H
