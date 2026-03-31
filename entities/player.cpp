#include "entities/player.h"
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

void Player::setInput(const InputState &input)
{
    m_input = input;
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

void Player::tick(qreal dt, const TileMap &tileMap, qreal gravity)
{
    const bool movingLeft = m_input.moveLeft && !m_input.moveRight;
    const bool movingRight = m_input.moveRight && !m_input.moveLeft;
    const qreal currentDrag = m_onGround ? kGroundDrag : kAirDrag;
    const qreal currentAccel = m_onGround ? kGroundAccel : kAirAccel;

    if (movingLeft) {
        m_velocity.setX(m_velocity.x()-currentAccel*dt);
    } else if (movingRight) {
        m_velocity.setX(m_velocity.x()+currentAccel*dt);
    }
    if(m_velocity.x()>0){
        m_velocity.setX(qMax(0.0,m_velocity.x()-currentDrag*dt));
    }else if(m_velocity.x()<0){
        m_velocity.setX(qMin(0.0,m_velocity.x()+currentDrag*dt));
    }
    m_velocity.setX(qBound(-kRunSpeed,m_velocity.x(),kRunSpeed));

    if (m_input.jump && m_onGround) {
        m_velocity.setY(-kJumpImpulse);
        m_onGround = false;
    }

    moveHorizontally(dt, tileMap);
    moveVertically(dt, tileMap, gravity);
}

void Player::moveHorizontally(qreal dt, const TileMap &tileMap)
{
    if (qFuzzyIsNull(m_velocity.x()))
        return;

    moveBy(m_velocity.x() * dt, 0.0);
    resolveTileCollisionsX(tileMap);
}

void Player::moveVertically(qreal dt, const TileMap &tileMap, qreal gravity)
{
    m_velocity.setY(m_velocity.y() + gravity * dt);
    moveBy(0.0, m_velocity.y() * dt);

    m_onGround = false;
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
        if (m_velocity.x() > 0.0) {
            setX(x() - overlap.width());
            m_velocity.setX(0.0);
        } else if (m_velocity.x() < 0.0) {
            setX(x() + overlap.width());
            m_velocity.setX(0.0);
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
        if (m_velocity.y() > 0.0) {
            setY(y() - overlap.height());
            m_velocity.setY(0.0);
            m_onGround = true;
        } else if (m_velocity.y() < 0.0) {
            setY(y() + overlap.height());
            m_velocity.setY(0.0);
        }
        playerRect = sceneBoundingRect();
    }
}
