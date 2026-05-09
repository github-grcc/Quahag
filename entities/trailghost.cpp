#include "entities/trailghost.h"
#include "world/gameworld.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

TrailGhost::TrailGhost(QPointF pos, const QPixmap &sprite)
    : m_sprite(sprite)
{
    setPos(pos);
    setZValue(ZLayer::Particles);
}

QRectF TrailGhost::boundingRect() const
{
    return m_bodyRect;
}

void TrailGhost::paint(QPainter *painter,
                        const QStyleOptionGraphicsItem *,
                        QWidget *)
{
    const qreal opacity = 1.0 - qMin(age() / kLifetime, 1.0);
    painter->setOpacity(opacity);
    painter->drawPixmap(m_bodyRect.topLeft(), m_sprite);
}

void TrailGhost::tick(const TickContext &ctx)
{
    advanceAge(ctx.dt);
    if (age() >= kLifetime)
        ctx.world->destroyLater(this);
}
