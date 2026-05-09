#ifndef HEALCROSS_H
#define HEALCROSS_H

#include "entities/actoritem.h"

class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

class HealCross : public ActorItem
{
    Q_OBJECT
public:
    explicit HealCross(QPointF pos);

    void tick(const TickContext &ctx) override;
    EntityKind kind() const override { return EntityKind::Effect; }
    Faction faction() const override { return Faction::Neutral; }
    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

private:
    static constexpr qreal kLifetime{1.5};
    qreal m_floatSpeed;
    qreal m_crossSize;
};

#endif // HEALCROSS_H
