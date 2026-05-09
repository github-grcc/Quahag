#include "entities/hachimi.h"
#include "entities/player.h"
#include "core/tickcontext.h"
#include "world/gameworld.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

Hachimi::Hachimi()
{
    m_sprite.load(QStringLiteral(":/rsc/sprites/hachimi.png"));
    m_sprite = m_sprite.scaled(static_cast<int>(m_spriteSize.width()),
                               static_cast<int>(m_spriteSize.height()),
                               Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

QRectF Hachimi::boundingRect() const
{
    return m_bodyRect;
}

void Hachimi::paint(QPainter *painter,
                   const QStyleOptionGraphicsItem *,
                   QWidget *)
{
    painter->drawPixmap(m_bodyRect.topLeft(), m_sprite);
}

void Hachimi::tick(const TickContext &ctx)
{
    advanceAge(ctx.dt);

    Player *player = ctx.world->player();
    if (!player || player->pendingDestroy())
        return;

    if (sceneBoundingRect().intersects(player->sceneBoundingRect())) {
        player->applyHachimiBoost();
        ctx.world->destroyLater(this);
    }
}
