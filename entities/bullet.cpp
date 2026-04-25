#include "bullet.h"
#include"world/tilemap.h"
#include"world/gameworld.h"
#include"entities/player.h"
#include"entities/enemy.h"
#include<QPainter>
#include<QtGlobal>

namespace {
constexpr qreal kMaxBulletLifetime = 5.0;
}

Bullet::Bullet(ActorItem *owner, QPointF spawnPosition, qreal shootAngle)
{
    m_owner = owner;
    m_shootAngle = shootAngle;
    m_spawnPosition = spawnPosition;
    setPos(spawnPosition);
    setRotation(m_shootAngle * 180.0 / M_PI);
    setVelocityX(m_speed * cos(m_shootAngle));
    setVelocityY(m_speed * sin(m_shootAngle));
}
void Bullet::tick(const TickContext &ctx)
{
    advanceAge(ctx.dt);
    if (age() > kMaxBulletLifetime) {
        if (ctx.world)
            ctx.world->destroyLater(this);
        return;
    }
    if (checkCollision(ctx))
        return;
    setPos(pos() + velocity() * ctx.dt);
}

QRectF Bullet::boundingRect() const
{
    return m_bodyRect;
}

void Bullet::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(252, 248, 248));
    painter->drawRect(m_bodyRect);
}

bool Bullet::checkCollision(const TickContext &ctx)
{
    if (!ctx.world)
        return false;

    QRectF bulletRect = sceneBoundingRect();

    const auto entities = ctx.world->entities();
    for (ActorItem *entity : entities) {
        if (entity == m_owner)
            continue;
        if (entity == this)
            continue;

        EntityKind kind = entity->kind();
        if (kind != EntityKind::Player && kind != EntityKind::Enemy)
            continue;

        if (bulletRect.intersects(entity->sceneBoundingRect())) {
            if (auto *player = qobject_cast<Player *>(entity)) {
                player->takeDamage(ctx);
            } else if (auto *enemy = qobject_cast<Enemy *>(entity)) {
                enemy->takeDamage(ctx);
            }
            ctx.world->destroyLater(this);
            return true;
        }
    }

    const auto tiles = ctx.world->tileMap().solidTilesOverlapping(bulletRect);
    for (const QPoint &tile : tiles) {
        const QRectF tileRect(ctx.world->tileMap().tileToScene(tile.y(), tile.x()), ctx.world->tileMap().tileSize().toSizeF());
        if (!bulletRect.intersects(tileRect))
            continue;
        ctx.world->destroyLater(this);
        return true;
    }
    return false;
}
