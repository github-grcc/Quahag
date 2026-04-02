#include "entities/player.h"
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

Player::Player()
{
}

QRectF Player::boundingRect() const
{
    return m_bodyRect;
}

void Player::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(253, 184, 39));
    painter->drawRect(m_bodyRect);
}

void Player::tick(const TickContext &ctx)
{
    if (!ctx.world)
        return;

    const InputState input = ctx.input ? *ctx.input : InputState{};
    const TileMap &tileMap = ctx.world->tileMap();
    const qreal dt = ctx.dt;
    const qreal gravity = ctx.gravity;
    const bool movingLeft = input.moveLeft && !input.moveRight;
    const bool movingRight = input.moveRight && !input.moveLeft;
    const qreal currentDrag = onGround() ? kGroundDrag : kAirDrag;
    const qreal currentAccel = onGround() ? kGroundAccel : kAirAccel;

    if (movingLeft) {
        setVelocityX(velocityX() - currentAccel * dt);
    } else if (movingRight) {
        setVelocityX(velocityX() + currentAccel * dt);
    }
    if (velocityX() > 0) {
        setVelocityX(qMax(0.0, velocityX() - currentDrag * dt));
    } else if (velocityX() < 0) {
        setVelocityX(qMin(0.0, velocityX() + currentDrag * dt));
    }
    setVelocityX(qBound(-kRunSpeed, velocityX(), kRunSpeed));

    if (input.jump && onGround()) {
        setVelocityY(-kJumpImpulse);
        setOnGround(false);
    }

    moveHorizontally(dt, tileMap);
    moveVertically(dt, tileMap, gravity);
}

void Player::moveHorizontally(qreal dt, const TileMap &tileMap)
{
    if (qFuzzyIsNull(velocityX()))
        return;

    moveBy(velocityX() * dt, 0.0);
    resolveTileCollisionsX(tileMap);
}

void Player::moveVertically(qreal dt, const TileMap &tileMap, qreal gravity)
{
    setVelocityY(velocityY() + gravity * dt);
    moveBy(0.0, velocityY() * dt);

    setOnGround(false);
    resolveTileCollisionsY(tileMap);
}

void Player::resolveTileCollisionsX(const TileMap &tileMap)
{
    QRectF playerRect = sceneBoundingRect();
    const auto tiles = tileMap.solidTilesOverlapping(playerRect);
    for (const QPoint &tile : tiles) {
        const QRectF tileRect(tileMap.tileToScene(tile.y(), tile.x()), tileMap.tileSize().toSizeF());
        if (!playerRect.intersects(tileRect))
            continue;

        const QRectF overlap = playerRect.intersected(tileRect);
        if (velocityX() > 0.0) {
            setX(x() - overlap.width());
            setVelocityX(0.0);
        } else if (velocityX() < 0.0) {
            setX(x() + overlap.width());
            setVelocityX(0.0);
        }
        playerRect = sceneBoundingRect();
    }
}

void Player::resolveTileCollisionsY(const TileMap &tileMap)
{
    QRectF playerRect = sceneBoundingRect();
    const auto tiles = tileMap.solidTilesOverlapping(playerRect);
    for (const QPoint &tile : tiles) {
        const QRectF tileRect(tileMap.tileToScene(tile.y(), tile.x()), tileMap.tileSize().toSizeF());
        if (!playerRect.intersects(tileRect))
            continue;

        const QRectF overlap = playerRect.intersected(tileRect);
        if (velocityY() > 0.0) {
            setY(y() - overlap.height());
            setVelocityY(0.0);
            setOnGround(true);
        } else if (velocityY() < 0.0) {
            setY(y() + overlap.height());
            setVelocityY(0.0);
        }
        playerRect = sceneBoundingRect();
    }
}
