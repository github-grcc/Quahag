#include "entities/throne.h"
#include "entities/player.h"
#include "core/tickcontext.h"
#include "world/gameworld.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

Throne::Throne()
{
    m_sprite.load(QStringLiteral(":/rsc/sprites/throne.png"));
    m_sprite = m_sprite.scaled(static_cast<int>(m_spriteSize.width()),
                               static_cast<int>(m_spriteSize.height()),
                               Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

QRectF Throne::boundingRect() const
{
    return m_bodyRect;
}

void Throne::paint(QPainter *painter,
                   const QStyleOptionGraphicsItem *,
                   QWidget *)
{
    painter->drawPixmap(m_bodyRect.topLeft(), m_sprite);
}

void Throne::tick(const TickContext &ctx)
{
    advanceAge(ctx.dt);

    Player *player = ctx.world->player();
    if (!player || player->pendingDestroy())
        return;

    if (sceneBoundingRect().intersects(player->sceneBoundingRect())) {
        ctx.world->setVictory(true);
    }
}
