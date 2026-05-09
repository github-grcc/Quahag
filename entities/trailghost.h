#ifndef TRAILGHOST_H
#define TRAILGHOST_H

#include "entities/actoritem.h"
#include <QPixmap>

class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

class TrailGhost : public ActorItem
{
    Q_OBJECT
public:
    TrailGhost(QPointF pos, const QPixmap &sprite);

    void tick(const TickContext &ctx) override;
    EntityKind kind() const override { return EntityKind::Effect; }
    Faction faction() const override { return Faction::Neutral; }
    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

private:
    QPixmap m_sprite;
    QRectF m_bodyRect{-25, -25, 50, 50};
    static constexpr qreal kLifetime{0.7};
};

#endif // TRAILGHOST_H
