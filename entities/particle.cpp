#include "entities/particle.h"
#include "world/gameworld.h"
#include "core/entitytypes.h"

#include <QPainter>
#include <QRandomGenerator>
#include <QtMath>
#include <QtGlobal>

// --- Physical constructor (fireworks) ---
Particle::Particle(QPointF pos, QPointF velocity,
                   qreal gravity, qreal lifetime)
    : m_type(Type::Physical)
{
    setPos(pos);
    setVelocity(velocity);
    m_gravity = gravity;
    m_lifetime = lifetime;
    m_size = 4.0;
    setZValue(ZLayer::Particles);
}

// --- Simple constructor (dust) ---
Particle::Particle(QPointF startPos, QPointF endPos,
                   qreal startSize, qreal endSize,
                   qreal duration)
    : m_type(Type::Simple)
{
    setPos(startPos);
    m_startPos = startPos;
    m_endPos = endPos;
    m_startSize = startSize;
    m_endSize = endSize;
    m_duration = duration;
    m_size = startSize;
    setZValue(ZLayer::Particles);
}

void Particle::tick(const TickContext &ctx)
{
    advanceAge(ctx.dt);

    switch (m_type) {
    case Type::Physical: {
        // Apply gravity
        setVelocityY(velocityY() + m_gravity * ctx.dt);

        // Integrate position
        prepareGeometryChange();
        setPos(x() + velocityX() * ctx.dt,
               y() + velocityY() * ctx.dt);

        // Check expiry
        const qreal speed = qSqrt(velocityX() * velocityX()
                                + velocityY() * velocityY());
        if (age() >= m_lifetime || speed < 10.0 || y() > 5000.0) {
            if (ctx.world)
                ctx.world->destroyLater(this);
        }
        break;
    }
    case Type::Simple: {
        const qreal t = qMin(1.0, age() / m_duration);
        const qreal cx = m_startPos.x() + (m_endPos.x() - m_startPos.x()) * t;
        const qreal cy = m_startPos.y() + (m_endPos.y() - m_startPos.y()) * t;

        prepareGeometryChange();
        setPos(cx, cy);

        m_size = m_startSize + (m_endSize - m_startSize) * t;
        m_alpha = 1.0 - t;

        if (t >= 1.0) {
            if (ctx.world)
                ctx.world->destroyLater(this);
        }
        break;
    }
    }
}

QRectF Particle::boundingRect() const
{
    const qreal half = (m_type == Type::Physical)
        ? qMax(m_size, qSqrt(velocityX() * velocityX()
                            + velocityY() * velocityY()) / 25.0) / 2.0 + 2.0
        : m_size / 2.0 + 2.0;
    return QRectF(-half, -half, half * 2.0, half * 2.0);
}

void Particle::paint(QPainter *painter,
                     const QStyleOptionGraphicsItem *,
                     QWidget *)
{
    painter->setOpacity(m_alpha);

    if (m_type == Type::Physical) {
        const qreal speed = qSqrt(velocityX() * velocityX()
                                + velocityY() * velocityY());
        const qreal tailLen = qMax(qreal(4.0), speed / 25.0);
        const qreal angle = qAtan2(velocityY(), velocityX());

        painter->save();
        painter->rotate((angle + M_PI) * 180.0 / M_PI);
        painter->setBrush(Qt::white);
        painter->setPen(QPen(Qt::black, 1));
        painter->drawRect(QRectF(0, -2, tailLen, 4));
        painter->restore();
    } else {
        const qreal half = m_size / 2.0;
        painter->setPen(QPen(Qt::black, 1));
        painter->setBrush(Qt::white);
        painter->drawRect(QRectF(-half, -half, m_size, m_size));
    }
}

// --- Factory methods ---

void Particle::fireworks(GameWorld *world, QPointF center,
                         int count, qreal radiusX, qreal radiusY)
{
    auto *rng = QRandomGenerator::global();
    for (int i = 0; i < count; ++i) {
        const qreal angle = -(qreal)M_PI + rng->bounded((qreal)M_PI); // [ -PI, 0 ]
        const qreal speed = 200.0 + rng->bounded(200.0);               // [ 200, 400 ]
        const qreal vx = qCos(angle) * speed;
        const qreal vy = qSin(angle) * speed;

        const qreal ox = rng->bounded(2.0 * radiusX) - radiusX;
        const qreal oy = (radiusY > 0)
            ? rng->bounded(2.0 * radiusY) - radiusY
            : 0.0;

        auto *p = world->createEntity<Particle>(
            QPointF(center.x() + ox, center.y() + oy),
            QPointF(vx, vy),
            800.0,  // gravity
            4.0);   // lifetime
        p->setZValue(ZLayer::Particles);
    }
}

void Particle::dust(GameWorld *world, QPointF pos, int count)
{
    auto *rng = QRandomGenerator::global();
    for (int i = 0; i < count; ++i) {
        const qreal startSize = 5.0 + rng->bounded(5.0);   // [ 5, 10 ]
        const qreal dx = rng->bounded(80.0) - 40.0;        // [ -40, 40 ]
        const qreal dy = -(20.0 + rng->bounded(30.0));     // [ -50, -20 ]
        const qreal duration = 0.2 + rng->bounded(0.3);    // [ 0.2, 0.5 ]

        auto *p = world->createEntity<Particle>(
            pos,
            QPointF(pos.x() + dx, pos.y() + dy),
            startSize, 0.0,
            duration);
        p->setZValue(ZLayer::Particles);
    }
}
