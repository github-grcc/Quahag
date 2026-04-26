#pragma once

#include "entities/actoritem.h"
#include "core/tickcontext.h"

#include <QPointF>
#include <QColor>
#include <QRectF>

class GameWorld;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

class Particle : public ActorItem
{
    Q_OBJECT
public:
    enum class Type {
        Simple,
        Physical
    };

    // Physical: physics-based firework/bullet-spark
    Particle(QPointF pos, QPointF velocity,
             qreal gravity, qreal lifetime);

    // Simple: interpolating dust particle
    Particle(QPointF startPos, QPointF endPos,
             qreal startSize, qreal endSize,
             qreal duration);

    void tick(const TickContext &ctx) override;
    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

    EntityKind kind() const override { return EntityKind::Effect; }
    Faction faction() const override { return Faction::Neutral; }

    static void fireworks(GameWorld *world, QPointF center,
                          int count, qreal radiusX, qreal radiusY);
    static void dust(GameWorld *world, QPointF pos, int count);

private:
    Type m_type;

    // Shared
    qreal m_alpha{1.0};
    qreal m_size{4.0};

    // Simple
    QPointF m_startPos;
    QPointF m_endPos;
    qreal m_startSize{4.0};
    qreal m_endSize{0.0};
    qreal m_duration{0.0};

    // Physical
    qreal m_gravity{0.0};
    qreal m_lifetime{0.0};
};
