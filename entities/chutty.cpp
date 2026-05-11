#include "entities/chutty.h"
#include "entities/player.h"
#include "entities/healcross.h"
#include "core/tickcontext.h"
#include "core/sound.h"
#include "world/gameworld.h"

#include <QPainter>
#include <QRandomGenerator>
#include <QStyleOptionGraphicsItem>

Chutty::Chutty()
{
    m_sprite.load(QStringLiteral(":/rsc/sprites/chutty.png"));
    m_sprite = m_sprite.scaled(static_cast<int>(m_spriteSize.width()),
                               static_cast<int>(m_spriteSize.height()),
                               Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

QRectF Chutty::boundingRect() const
{
    return m_bodyRect;
}

void Chutty::paint(QPainter *painter,
                   const QStyleOptionGraphicsItem *,
                   QWidget *)
{
    painter->drawPixmap(m_bodyRect.topLeft(), m_sprite);
}

void Chutty::tick(const TickContext &ctx)
{
    advanceAge(ctx.dt);

    Player *player = ctx.world->player();
    if (!player || player->pendingDestroy() || player->health() <= 0)
        return;

    if (sceneBoundingRect().intersects(player->sceneBoundingRect())) {
        playQrcSound(":/rsc/sounds/chutty.wav");
        player->addHealth(1);

        auto *rng = QRandomGenerator::global();
        const QPointF center = player->sceneBoundingRect().center();
        const int count = 5 + rng->bounded(3); // 5-7 crosses
        for (int i = 0; i < count; ++i) {
            const qreal ox = rng->bounded(60.0) - 30.0;
            const qreal oy = rng->bounded(60.0) - 30.0;
            ctx.world->createEntity<HealCross>(QPointF(center.x() + ox, center.y() + oy));
        }

        ctx.world->destroyLater(this);
    }
}
