#include "entities/enemy.h"
#include "world/gameworld.h"
#include "world/tilemap.h"

#include <QPainter>
#include <QtGlobal>

namespace {
constexpr qreal kRunSpeed = 420.0;
constexpr qreal kJumpImpulse = 820.0;
constexpr qreal kAirDrag = 10.0;
constexpr qreal kGroundDrag = 400.0;
constexpr qreal kAirAccel = 600.0;
constexpr qreal kGroundAccel = 1200.0;
} // namespace

Enemy::Enemy()
{
}

QRectF Enemy::boundingRect() const
{
    return m_bodyRect;
}

void Enemy::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(75, 207, 89));
    painter->drawRect(m_bodyRect);
}

void Enemy::tick(const TickContext &ctx)
{
    if (!ctx.world)
        return;

}

