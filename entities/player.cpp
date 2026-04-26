#include "entities/player.h"
#include "entities/enemy.h"
#include "world/gameworld.h"
#include "world/tilemap.h"

#include <QPainter>
#include <QtGlobal>

namespace {
constexpr qreal kRunSpeed = 600.0;
constexpr qreal kJumpImpulse = 820.0;
constexpr qreal kAirDrag = 1000.0;
constexpr qreal kGroundDrag = 3000.0;
constexpr qreal kAirAccel = 2000.0;
constexpr qreal kGroundAccel = 3000.0;

// Coyote time
constexpr qreal kCoyoteTime = 0.1;

// Jump system
constexpr int kMaxJumps = 2;
constexpr qreal kDoubleJumpImpulse = 800.0;

// Wall slide
constexpr qreal kWallSlideSpeed = 100.0;
constexpr qreal kWallJumpHorizontal = 400.0;
constexpr qreal kWallJumpVertical = 700.0;
constexpr qreal kWallJumpLockTime = 0.12;

// Attack
constexpr qreal kAttackRange = 50.0;
constexpr qreal kAttackHeight = 50.0;
constexpr qreal kAttackCooldown = 0.1;
constexpr qreal kAttackVisualDuration = 0.2;
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

    // Attack flash (underneath body)
    if (m_attackTimer > 0.0) {
        painter->setBrush(QColor(255, 255, 255, 120));
        painter->drawRect(m_bodyRect);
    }else{
        // Main body
        painter->setBrush(QColor(253, 184, 39));
        painter->drawRect(m_bodyRect);

    }


    // // Facing indicator
    // painter->setBrush(QColor(200, 140, 20));
    // if (m_facing == 1) {
    //     const QPointF triangle[3] = {{12.0, -4.0}, {12.0, 4.0}, {18.0, 0.0}};
    //     painter->drawPolygon(triangle, 3);
    // } else {
    //     const QPointF triangle[3] = {{-12.0, -4.0}, {-12.0, 4.0}, {-18.0, 0.0}};
    //     painter->drawPolygon(triangle, 3);
    // }
}

void Player::tick(const TickContext &ctx)
{
    if (!ctx.world)
        return;

    advanceAge(ctx.dt);
    const TileMap &tileMap = ctx.world->tileMap();
    const qreal dt = ctx.dt;
    const InputState input = ctx.input ? *ctx.input : InputState{};

    // ---- Edge detection ----
    const bool jumpPressed = input.jump && !m_prevJump;
    const bool attackPressed = input.attack && !m_prevAttack;
    m_prevJump = input.jump;
    m_prevAttack = input.attack;

    // ---- Timers ----
    m_attackCooldown = qMax(0.0, m_attackCooldown - dt);
    m_attackTimer = qMax(0.0, m_attackTimer - dt);
    m_wallJumpLockTimer = qMax(0.0, m_wallJumpLockTimer - dt);

    // Coyote timer
    if (onGround()) {
        m_coyoteTimer = 0.0;
    } else {
        m_coyoteTimer += dt;
    }

    // ---- Facing direction ----
    if (input.moveLeft && !input.moveRight) {
        m_facing = -1;
    } else if (input.moveRight && !input.moveLeft) {
        m_facing = 1;
    }

    // ---- Wall detection ----
    m_wallSide = detectWallSide(tileMap);

    // ---- State machine ----
    updateState(ctx, jumpPressed);

    // ---- Physics ----
    moveHorizontally(dt, tileMap);
    moveVertically(dt, tileMap);

    // ---- Post-collision fixup ----
    if (onGround()) {
        m_jumpsUsed = 0;
        m_coyoteTimer = 0.0;
        if (m_state != PlayerState::Grounded)
            m_state = PlayerState::Grounded;
    } else if (m_state == PlayerState::Grounded) {
        m_state = PlayerState::Airborne;
    }

    // ---- Attack (parallel, independent of state) ----
    processAttack(attackPressed, ctx);
}

void Player::updateState(const TickContext &ctx, bool jumpPressed)
{
    switch (m_state) {
    case PlayerState::Grounded:
        behaveGrounded(ctx, jumpPressed);
        break;
    case PlayerState::Airborne:
        behaveAirborne(ctx, jumpPressed);
        break;
    case PlayerState::WallSliding:
        behaveWallSliding(ctx, jumpPressed);
        break;
    }
}

void Player::behaveGrounded(const TickContext &ctx, bool jumpPressed)
{
    const InputState input = ctx.input ? *ctx.input : InputState{};
    const qreal dt = ctx.dt;
    const bool movingLeft = input.moveLeft && !input.moveRight;
    const bool movingRight = input.moveRight && !input.moveLeft;

    // Apply ground accel/drag
    if (movingLeft)
        setVelocityX(velocityX() - kGroundAccel * dt);
    else if (movingRight)
        setVelocityX(velocityX() + kGroundAccel * dt);

    if (velocityX() > 0.0)
        setVelocityX(qMax(0.0, velocityX() - kGroundDrag * dt));
    else if (velocityX() < 0.0)
        setVelocityX(qMin(0.0, velocityX() + kGroundDrag * dt));

    setVelocityX(qBound(-kRunSpeed, velocityX(), kRunSpeed));

    // Coyote jump: allow jump just after leaving ground
    if (jumpPressed && (onGround() || m_coyoteTimer < kCoyoteTime)) {
        setVelocityY(-kJumpImpulse);
        m_jumpsUsed = 1;
        m_state = PlayerState::Airborne;
    }
}

void Player::behaveAirborne(const TickContext &ctx, bool jumpPressed)
{
    const InputState input = ctx.input ? *ctx.input : InputState{};
    const qreal dt = ctx.dt;
    const qreal gravity = ctx.gravity;
    const bool movingLeft = input.moveLeft && !input.moveRight;
    const bool movingRight = input.moveRight && !input.moveLeft;

    // Apply air accel/drag
    if (movingLeft)
        setVelocityX(velocityX() - kAirAccel * dt);
    else if (movingRight)
        setVelocityX(velocityX() + kAirAccel * dt);

    if (velocityX() > 0.0)
        setVelocityX(qMax(0.0, velocityX() - kAirDrag * dt));
    else if (velocityX() < 0.0)
        setVelocityX(qMin(0.0, velocityX() + kAirDrag * dt));

    setVelocityX(qBound(-kRunSpeed, velocityX(), kRunSpeed));

    // Apply gravity
    setVelocityY(velocityY() + gravity * dt);

    // Double jump
    if (jumpPressed && m_jumpsUsed < kMaxJumps) {
        setVelocityY(-kDoubleJumpImpulse);
        m_jumpsUsed++;
    }

    // Wall slide transition: must be falling, against a wall, pressing into it,
    // and not in wall-jump lockout
    if (m_wallSide != 0 && velocityY() >= 0.0 && m_wallJumpLockTimer <= 0.0) {
        const bool pressingTowardWall = (m_wallSide == -1 && movingLeft)
                                     || (m_wallSide == 1 && movingRight);
        if (pressingTowardWall) {
            m_state = PlayerState::WallSliding;
        }
    }
}

void Player::behaveWallSliding(const TickContext &ctx, bool jumpPressed)
{
    const InputState input = ctx.input ? *ctx.input : InputState{};
    const qreal dt = ctx.dt;
    const qreal gravity = ctx.gravity;

    // Exit wall slide if no wall or on ground
    if (m_wallSide == 0 || onGround()) {
        m_state = onGround() ? PlayerState::Grounded : PlayerState::Airborne;
        return;
    }

    // Face away from wall
    m_facing = -m_wallSide;

    // Apply gravity with slide-speed cap
    setVelocityY(velocityY() + gravity * dt);
    if (velocityY() > kWallSlideSpeed)
        setVelocityY(kWallSlideSpeed);

    // Allow movement away from wall using air accel
    const bool pressingAway = (m_wallSide == -1 && input.moveRight)
                           || (m_wallSide == 1 && input.moveLeft);
    const bool pressingToward = (m_wallSide == -1 && input.moveLeft)
                             || (m_wallSide == 1 && input.moveRight);

    if (pressingAway) {
        const qreal dir = (m_wallSide == -1) ? 1.0 : -1.0;
        setVelocityX(velocityX() + dir * kAirAccel * dt);
        // Apply air drag
        if (velocityX() * dir > 0.0)
            setVelocityX(qMax(0.0, qAbs(velocityX()) - kAirDrag * dt) * dir);
    } else if (!pressingToward) {
        // Not pressing anything horizontally: apply drag toward zero
        if (velocityX() > 0.0)
            setVelocityX(qMax(0.0, velocityX() - kAirDrag * dt));
        else if (velocityX() < 0.0)
            setVelocityX(qMin(0.0, velocityX() + kAirDrag * dt));
    } else {
        // Pressing toward wall: stick, zero horizontal velocity
        setVelocityX(0.0);
    }

    setVelocityX(qBound(-kRunSpeed, velocityX(), kRunSpeed));

    // Wall jump
    if (jumpPressed) {
        setVelocityY(-kWallJumpVertical);
        setVelocityX(-m_wallSide * kWallJumpHorizontal);
        m_jumpsUsed = 1;
        m_wallJumpLockTimer = kWallJumpLockTime;
        m_state = PlayerState::Airborne;
    }
}

void Player::moveHorizontally(qreal dt, const TileMap &tileMap)
{
    if (qFuzzyIsNull(velocityX()))
        return;

    moveBy(velocityX() * dt, 0.0);
    resolveTileCollisionsX(tileMap);
}

void Player::moveVertically(qreal dt, const TileMap &tileMap)
{
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

int Player::detectWallSide(const TileMap &tileMap) const
{
    const QRectF r = sceneBoundingRect();
    const int tileW = tileMap.tileWidth();
    const int tileH = tileMap.tileHeight();
    const int topRow = static_cast<int>(r.top() / tileH);
    const int botRow = static_cast<int>((r.bottom() - 1.0) / tileH);

    // Check left side
    const int leftCol = static_cast<int>((r.left() - 1.0) / tileW);
    for (int row = topRow; row <= botRow; ++row) {
        if (tileMap.isSolidTile(row, leftCol))
            return -1;
    }

    // Check right side
    const int rightCol = static_cast<int>((r.right() + 1.0) / tileW);
    for (int row = topRow; row <= botRow; ++row) {
        if (tileMap.isSolidTile(row, rightCol))
            return 1;
    }

    return 0;
}

void Player::processAttack(bool attackPressed, const TickContext &ctx)
{
    if (!attackPressed || m_attackCooldown > 0.0)
        return;

    const QRectF playerRect = sceneBoundingRect();
    QRectF hitbox;
    if (m_facing == 1) {
        hitbox = QRectF(playerRect.right(),
                        playerRect.center().y() - kAttackHeight / 2.0,
                        kAttackRange, kAttackHeight);
    } else {
        hitbox = QRectF(playerRect.left() - kAttackRange,
                        playerRect.center().y() - kAttackHeight / 2.0,
                        kAttackRange, kAttackHeight);
    }

    for (auto *entity : ctx.world->entitiesOfKind(EntityKind::Enemy)) {
        if (!entity->sceneBoundingRect().intersects(hitbox))
            continue;

        if (auto *enemy = qobject_cast<Enemy *>(entity))
            enemy->takeDamage(ctx);
    }

    m_attackCooldown = kAttackCooldown;
    m_attackTimer = kAttackVisualDuration;
}

void Player::takeDamage(const TickContext &ctx)
{
    if (--m_health <= 0) {
        if (ctx.world) {
            ctx.world->destroyLater(this);
        }
    }
}
