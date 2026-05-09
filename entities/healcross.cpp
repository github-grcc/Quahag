#include "entities/healcross.h"
#include "world/gameworld.h"

#include <QPainter>
#include <QRandomGenerator>
#include <QStyleOptionGraphicsItem>

HealCross::HealCross(QPointF pos)
{
    setPos(pos);
    setZValue(ZLayer::Particles);

    auto *rng = QRandomGenerator::global();
    m_floatSpeed = 50.0 + rng->bounded(40.0);   // 50-90 px/s
    m_crossSize = 4.0 + rng->bounded(5.0);       // 4-8 px arm half-length
}

QRectF HealCross::boundingRect() const
{
    const qreal half = m_crossSize + 2.0;
    return QRectF(-half, -half, half * 2.0, half * 2.0);
}

void HealCross::paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *,
                       QWidget *)
{
    const qreal opacity = 1.0 - qMin(age() / kLifetime, 1.0);
    painter->setOpacity(opacity);

    const qreal armW = qMax(1.0, m_crossSize * 0.4);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 255, 80)); // fluorescent green
    // horizontal bar
    painter->drawRect(QRectF(-m_crossSize, -armW, m_crossSize * 2.0, armW * 2.0));
    // vertical bar
    painter->drawRect(QRectF(-armW, -m_crossSize, armW * 2.0, m_crossSize * 2.0));
}

void HealCross::tick(const TickContext &ctx)
{
    advanceAge(ctx.dt);

    setPos(x(), y() - m_floatSpeed * ctx.dt);

    if (age() >= kLifetime)
        ctx.world->destroyLater(this);
}
