#pragma once

#include "entities/actoritem.h"
#include "core/tickcontext.h"

#include <QPointF>
#include <QRectF>

class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

class ClawEffect : public ActorItem
{
    Q_OBJECT
public:
    explicit ClawEffect(QPointF pos);

    void tick(const TickContext &ctx) override;
    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

    EntityKind kind() const override { return EntityKind::Effect; }
    Faction faction() const override { return Faction::Neutral; }

private:
    void drawClaw(QPainter *painter, qreal length) const;

    qreal m_angle{0.0};
    qreal m_scale{1.0};
    qreal m_lifetime{1.0};
};
