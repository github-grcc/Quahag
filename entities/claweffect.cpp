#include "entities/claweffect.h"
#include "world/gameworld.h"
#include "core/entitytypes.h"

#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QtMath>
#include <QtGlobal>

ClawEffect::ClawEffect(QPointF pos)
{
    setPos(pos);
    auto *rng = QRandomGenerator::global();
    m_angle = rng->bounded(2.0 * M_PI);          // seed * TWO_PI
    m_scale = 1.0 + rng->bounded(0.5);           // 1 + seed * 0.5
    setZValue(ZLayer::Effects);
}

void ClawEffect::tick(const TickContext &ctx)
{
    advanceAge(ctx.dt);
    if (age() >= m_lifetime) {
        if (ctx.world)
            ctx.world->destroyLater(this);
    }
}

QRectF ClawEffect::boundingRect() const
{
    constexpr qreal maxExtent = 40.0 * 1.5 + 10.0; // length * maxScale + yOffsets
    return QRectF(-maxExtent, -maxExtent, maxExtent * 2.0, maxExtent * 2.0);
}

void ClawEffect::paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *,
                       QWidget *)
{
    // Fade out in last 0.25s
    constexpr qreal kFadeDuration = 0.25;
    const qreal fadeProgress = (age() - (m_lifetime - kFadeDuration)) / kFadeDuration;
    const qreal alpha = 1.0 - qBound(0.0, fadeProgress, 1.0);
    painter->setOpacity(alpha);

    painter->save();

    // Apply random rotation and scale
    painter->rotate(m_angle * 180.0 / M_PI);
    painter->scale(m_scale, m_scale);

    // Extend animation: length grows from 0 to full in 0.1s
    constexpr qreal kLength = 40.0;
    const qreal l = kLength * qMin(1.0, age() / 0.1);

    // Translate so origin is at center-left of claw
    painter->translate(-kLength / 2.0, 0);

    // Draw 3 claw marks: center (scale 1.0), top and bottom (scale 0.8)
    struct ClawLine { qreal yOffset; qreal scale; };
    static const ClawLine claws[] = {
        {  0.0, 1.0 },
        { -5.0, 0.8 },
        {  5.0, 0.8 },
    };

    for (const auto &claw : claws) {
        painter->save();
        painter->translate(0, claw.yOffset);
        painter->scale(claw.scale, claw.scale);
        drawClaw(painter, l * (1.0 / claw.scale)); // compensate scale so l is consistent
        painter->restore();
    }

    painter->restore();
}

void ClawEffect::drawClaw(QPainter *painter, qreal l) const
{
    constexpr qreal kThickness = 5.0;

    QPainterPath path;
    path.moveTo(0, 0);
    // Top bezier curve
    path.cubicTo(l / 2.0, -kThickness / 2.0,
                 l / 2.0, -kThickness / 2.0,
                 l, 0);
    // Bottom bezier curve back
    path.cubicTo(l / 2.0,  kThickness / 2.0,
                 l / 2.0,  kThickness / 2.0,
                 0, 0);
    path.closeSubpath();

    painter->setPen(QPen(Qt::black, 1));
    painter->setBrush(Qt::white);
    painter->drawPath(path);
}
