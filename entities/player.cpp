#include "entities/player.h"
#include "entities/enemy.h"
#include "entities/particle.h"
#include "entities/claweffect.h"
#include "entities/trailghost.h"
#include "world/gameworld.h"
#include "world/tilemap.h"

#include <QPainter>
#include <QPixmap>
#include "core/sound.h"
#include <QRandomGenerator>
#include <QtGlobal>
#include <QDebug>

namespace
{
    constexpr qreal kRunSpeed = 600.0;
    constexpr qreal kJumpImpulse = 820.0;
    constexpr qreal kAirDrag = 1000.0;
    constexpr qreal kGroundDrag = 3000.0;
    constexpr qreal kAirAccel = 2000.0;
    constexpr qreal kGroundAccel = 3000.0;

    //柯基时间
    constexpr qreal kCoyoteTime = 0.1;

    //跳跃系统
    constexpr int kMaxJumps = 2;
    constexpr qreal kDoubleJumpImpulse = 800.0;

    //滑墙
    constexpr qreal kWallSlideSpeed = 100.0;
    constexpr qreal kWallJumpHorizontal = 400.0;
    constexpr qreal kWallJumpVertical = 700.0;
    constexpr qreal kWallJumpLockTime = 0.12;

    //攻击
    constexpr qreal kAttackRange = 50.0;
    constexpr qreal kAttackHeight = 50.0;
    constexpr qreal kAttackCooldown = 0.1;
    constexpr qreal kAttackVisualDuration = 0.2;
} //命名空间

Player::Player()
{
    QPixmap originalIdleSprite;
    QPixmap originalAttackSprite;

    originalIdleSprite.load(":/rsc/sprites/player_idle.jpg");
    originalAttackSprite.load(":/rsc/sprites/player_attack.jpg");
    m_idleWhiteSprite.load(":/rsc/sprites/player_idle_white.png");
    m_attackWhiteSprite.load(":/rsc/sprites/player_attack_white.png");
    m_idleSprite = originalIdleSprite.scaled(m_spriteSize.width(), m_spriteSize.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_attackSprite = originalAttackSprite.scaled(m_spriteSize.width(), m_spriteSize.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_idleWhiteSprite = m_idleWhiteSprite.scaled(m_spriteSize.width(), m_spriteSize.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_attackWhiteSprite = m_attackWhiteSprite.scaled(m_spriteSize.width(), m_spriteSize.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QRectF Player::boundingRect() const
{
    return m_bodyRect;
}

void Player::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(Qt::NoPen);

    // //攻击闪烁(身体下方)
    // if (age() - m_lastDamageTime < 0.1) {
    //     painter->setBrush(Qt::white);
    //     painter->drawRect(m_bodyRect);
    // } else if (m_attackTimer > 0.0) {
    //     // painter->setBrush(QColor(255, 255, 255, 120));
    //     // painter->drawRect(m_bodyRect);
    //     painter->drawPixmap(-m_spriteSize.width()/2, -m_spriteSize.height()/2, m_attackSprite);

    // } else {
    //     //主体
    //     // painter->setBrush(QColor(253, 184, 39));
    //     // painter->drawRect(m_bodyRect);
    //     painter->drawPixmap(-m_spriteSize.width()/2, -m_spriteSize.height()/2, m_idleSprite);

    // }
    if (m_attackTimer > 0.0)
    {
        if (age() - m_lastDamageTime < 0.1)
        {
            painter->drawPixmap(-m_spriteSize.width() / 2, -m_spriteSize.height() / 2, m_attackWhiteSprite);
        }
        else
        {
            painter->drawPixmap(-m_spriteSize.width() / 2, -m_spriteSize.height() / 2, m_attackSprite);
        }
    }
    else
    {
        if (age() - m_lastDamageTime < 0.1)
        {
            painter->drawPixmap(-m_spriteSize.width() / 2, -m_spriteSize.height() / 2, m_idleWhiteSprite);
        }
        else
        {
            painter->drawPixmap(-m_spriteSize.width() / 2, -m_spriteSize.height() / 2, m_idleSprite);
        }
    }

    // //朝向指示
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
    TileMap &tileMap = ctx.world->tileMap();
    const qreal dt = ctx.dt;
    const InputState input = ctx.input ? *ctx.input : InputState{};

    //边缘检测
    const bool jumpPressed = input.jump && !m_prevJump;
    const bool attackPressed = input.attack && !m_prevAttack;
    m_prevJump = input.jump;
    m_prevAttack = input.attack;

    //计时器
    m_attackCooldown = qMax(0.0, m_attackCooldown - dt);
    m_attackTimer = qMax(0.0, m_attackTimer - dt);
    m_wallJumpLockTimer = qMax(0.0, m_wallJumpLockTimer - dt);

    //哈基米增强计时
    if (m_hachimiRemaining > 0.0) {
        m_hachimiRemaining -= dt;
        if (m_hachimiRemaining <= 0.0) {
            m_hachimiRemaining = 0.0;
            m_hachimiMultiplier = 1.0;
        }
    }

    //柯基计时
    if (onGround())
    {
        m_coyoteTimer = 0.0;
    }
    else
    {
        m_coyoteTimer += dt;
    }

    //朝向
    if (input.moveLeft && !input.moveRight)
    {
        m_facing = -1;
    }
    else if (input.moveRight && !input.moveLeft)
    {
        m_facing = 1;
    }

    //墙壁检测
    m_wallSide = detectWallSide(tileMap);

    //状态机
    updateState(ctx, jumpPressed);

    //物理
    moveHorizontally(dt, tileMap);
    moveVertically(dt, tileMap);

    //哈基米拖尾
    if (m_hachimiRemaining > 0.0 && (!qFuzzyIsNull(velocityX()) || !qFuzzyIsNull(velocityY())) && ctx.world) {
        m_hachimiTrailTimer += dt;
        constexpr qreal kTrailInterval = 0.04;
        if (m_hachimiTrailTimer >= kTrailInterval) {
            m_hachimiTrailTimer -= kTrailInterval;
            const QPixmap &sprite = (m_attackTimer > 0.0) ? m_attackSprite : m_idleSprite;
            ctx.world->createEntity<TrailGhost>(sceneBoundingRect().center(), sprite);
        }
    } else {
        m_hachimiTrailTimer = 0.0;
    }

    //碰撞后修正
    if (onGround())
    {
        m_jumpsUsed = 0;
        m_coyoteTimer = 0.0;
        if (m_state != PlayerState::Grounded)
            m_state = PlayerState::Grounded;
    }
    else if (m_state == PlayerState::Grounded)
    {
        m_state = PlayerState::Airborne;
    }

    //攻击(并行于状态机)
    processAttack(attackPressed, ctx);
}

void Player::updateState(const TickContext &ctx, bool jumpPressed)
{
    switch (m_state)
    {
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

    //地面加速/减速
    if (movingLeft)
        setVelocityX(velocityX() - kGroundAccel * m_hachimiMultiplier * dt);
    else if (movingRight)
        setVelocityX(velocityX() + kGroundAccel * m_hachimiMultiplier * dt);

    if (velocityX() > 0.0)
        setVelocityX(qMax(0.0, velocityX() - kGroundDrag * dt));
    else if (velocityX() < 0.0)
        setVelocityX(qMin(0.0, velocityX() + kGroundDrag * dt));

    setVelocityX(qBound(-kRunSpeed * m_hachimiMultiplier, velocityX(), kRunSpeed * m_hachimiMultiplier));

    //柯基跳：离开地面后仍可跳跃
    if (jumpPressed && (onGround() || m_coyoteTimer < kCoyoteTime))
    {
        setVelocityY(-kJumpImpulse * m_hachimiMultiplier);
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

    //空中加速/减速
    if (movingLeft)
        setVelocityX(velocityX() - kAirAccel * m_hachimiMultiplier * dt);
    else if (movingRight)
        setVelocityX(velocityX() + kAirAccel * m_hachimiMultiplier * dt);

    if (velocityX() > 0.0)
        setVelocityX(qMax(0.0, velocityX() - kAirDrag * dt));
    else if (velocityX() < 0.0)
        setVelocityX(qMin(0.0, velocityX() + kAirDrag * dt));

    setVelocityX(qBound(-kRunSpeed * m_hachimiMultiplier, velocityX(), kRunSpeed * m_hachimiMultiplier));

    //施加重力
    setVelocityY(velocityY() + gravity * dt);

    //二段跳
    if (jumpPressed && m_jumpsUsed < kMaxJumps)
    {
        setVelocityY(-kDoubleJumpImpulse * m_hachimiMultiplier);
        m_jumpsUsed++;
    }

    //滑墙过渡：必须下落、靠墙、向墙移动、且不在墙跳锁定中
    if (m_wallSide != 0 && velocityY() >= 0.0 && m_wallJumpLockTimer <= 0.0)
    {
        const bool pressingTowardWall = (m_wallSide == -1 && movingLeft) || (m_wallSide == 1 && movingRight);
        if (pressingTowardWall)
        {
            m_state = PlayerState::WallSliding;
        }
    }
}

void Player::behaveWallSliding(const TickContext &ctx, bool jumpPressed)
{
    const InputState input = ctx.input ? *ctx.input : InputState{};
    const qreal dt = ctx.dt;
    const qreal gravity = ctx.gravity;

    //无墙或落地时退出滑墙
    if (m_wallSide == 0 || onGround())
    {
        m_state = onGround() ? PlayerState::Grounded : PlayerState::Airborne;
        return;
    }

    //面朝墙外
    m_facing = -m_wallSide;

    //施加重力并限制滑速
    setVelocityY(velocityY() + gravity * dt);
    if (velocityY() > kWallSlideSpeed)
        setVelocityY(kWallSlideSpeed);

    //使用空中加速离开墙壁
    const bool pressingAway = (m_wallSide == -1 && input.moveRight) || (m_wallSide == 1 && input.moveLeft);
    const bool pressingToward = (m_wallSide == -1 && input.moveLeft) || (m_wallSide == 1 && input.moveRight);

    if (pressingAway)
    {
        const qreal dir = (m_wallSide == -1) ? 1.0 : -1.0;
        setVelocityX(velocityX() + dir * kAirAccel * dt);
        //空中减速
        if (velocityX() * dir > 0.0)
            setVelocityX(qMax(0.0, qAbs(velocityX()) - kAirDrag * dt) * dir);
    }
    else if (!pressingToward)
    {
        //无水平输入：减速至零
        if (velocityX() > 0.0)
            setVelocityX(qMax(0.0, velocityX() - kAirDrag * dt));
        else if (velocityX() < 0.0)
            setVelocityX(qMin(0.0, velocityX() + kAirDrag * dt));
    }
    else
    {
        //向墙按压：吸附，水平速度归零
        setVelocityX(0.0);
    }

    setVelocityX(qBound(-kRunSpeed * m_hachimiMultiplier, velocityX(), kRunSpeed * m_hachimiMultiplier));

    //墙跳
    if (jumpPressed)
    {
        setVelocityY(-kWallJumpVertical * m_hachimiMultiplier);
        setVelocityX(-m_wallSide * kWallJumpHorizontal * m_hachimiMultiplier);
        m_jumpsUsed = 1;
        m_wallJumpLockTimer = kWallJumpLockTime;
        m_state = PlayerState::Airborne;
    }
}

void Player::moveHorizontally(qreal dt, TileMap &tileMap)
{
    if (qFuzzyIsNull(velocityX()))
        return;

    moveBy(velocityX() * dt, 0.0);
    resolveTileCollisionsX(tileMap);
}

void Player::moveVertically(qreal dt, TileMap &tileMap)
{
    moveBy(0.0, velocityY() * dt);

    setOnGround(false);
    resolveTileCollisionsY(tileMap);
}

void Player::resolveTileCollisionsX(TileMap &tileMap)
{
    QRectF playerRect = sceneBoundingRect();
    const auto tiles = tileMap.solidTilesOverlapping(playerRect);
    for (const QPoint &tile : tiles)
    {
        const QRectF tileRect(tileMap.tileToScene(tile.y(), tile.x()), tileMap.tileSize().toSizeF());
        if (!playerRect.intersects(tileRect))
            continue;

        if (tileMap.tryPassOneWayWall(tile.y(), tile.x(), playerRect, velocityX(), velocityY()))
            continue;

        const QRectF overlap = playerRect.intersected(tileRect);
        if (velocityX() > 0.0)
        {
            setX(x() - overlap.width());
            setVelocityX(0.0);
        }
        else if (velocityX() < 0.0)
        {
            setX(x() + overlap.width());
            setVelocityX(0.0);
        }
        playerRect = sceneBoundingRect();
    }
}

void Player::resolveTileCollisionsY(TileMap &tileMap)
{
    QRectF playerRect = sceneBoundingRect();
    const auto tiles = tileMap.solidTilesOverlapping(playerRect);
    for (const QPoint &tile : tiles)
    {
        const QRectF tileRect(tileMap.tileToScene(tile.y(), tile.x()), tileMap.tileSize().toSizeF());
        if (!playerRect.intersects(tileRect))
            continue;

        if (tileMap.tryPassOneWayWall(tile.y(), tile.x(), playerRect, velocityX(), velocityY()))
            continue;

        const QRectF overlap = playerRect.intersected(tileRect);
        if (velocityY() > 0.0)
        {
            setY(y() - overlap.height());
            setVelocityY(0.0);
            setOnGround(true);
        }
        else if (velocityY() < 0.0)
        {
            setY(y() + overlap.height());
            setVelocityY(0.0);
        }
        playerRect = sceneBoundingRect();
    }
}

int Player::detectWallSide(TileMap &tileMap) const
{
    const QRectF r = sceneBoundingRect();
    const int tileW = tileMap.tileWidth();
    const int tileH = tileMap.tileHeight();
    const int topRow = static_cast<int>(r.top() / tileH);
    const int botRow = static_cast<int>((r.bottom() - 1.0) / tileH);

    //检测左侧
    const int leftCol = static_cast<int>((r.left() - 1.0) / tileW);
    for (int row = topRow; row <= botRow; ++row)
    {
        if (tileMap.isSolidTile(row, leftCol) && !TileMap::isOneWayWallType(tileMap.tileAt(row, leftCol)))
            return -1;
    }

    //检测右侧
    const int rightCol = static_cast<int>((r.right() + 1.0) / tileW);
    for (int row = topRow; row <= botRow; ++row)
    {
        if (tileMap.isSolidTile(row, rightCol) && !TileMap::isOneWayWallType(tileMap.tileAt(row, rightCol)))
            return 1;
    }

    return 0;
}

void Player::processAttack(bool attackPressed, const TickContext &ctx)
{
    if (!attackPressed || m_attackCooldown > 0.0)
        return;

    //攻击时镜头震动
    if (ctx.events)
    {
        ctx.events->cameraShakes.append(CameraShakeEvent{20.0, 0.15, 28.0});
    }
    playQrcSound(":/rsc/sounds/ha.wav");

    const QRectF playerRect = sceneBoundingRect();
    QRectF hitbox;
    if (m_facing == 1)
    {
        hitbox = QRectF(playerRect.right(),
                        playerRect.center().y() - kAttackHeight / 2.0,
                        kAttackRange, kAttackHeight);
    }
    else
    {
        hitbox = QRectF(playerRect.left() - kAttackRange,
                        playerRect.center().y() - kAttackHeight / 2.0,
                        kAttackRange, kAttackHeight);
    }

    for (auto *entity : ctx.world->entitiesOfKind(EntityKind::Enemy))
    {
        if (!entity->sceneBoundingRect().intersects(hitbox))
            continue;

        if (auto *enemy = qobject_cast<Enemy *>(entity))
            enemy->takeDamage(ctx);
    }

    //在攻击位置生成爪痕效果
    if (ctx.world)
    {
        auto *rng = QRandomGenerator::global();
        QPointF attackPos = sceneBoundingRect().center();
        attackPos.setX(attackPos.x() + m_facing * 60.0);
        attackPos.setX(attackPos.x() + rng->bounded(30.0) - 15.0);
        attackPos.setY(attackPos.y() + rng->bounded(50.0) - 25.0);

        ctx.world->createEntity<ClawEffect>(attackPos);
    }

    m_attackCooldown = kAttackCooldown;
    m_attackTimer = kAttackVisualDuration;
}

void Player::applyHachimiBoost()
{
    m_hachimiMultiplier *= 1.2;
    m_hachimiRemaining += 6.0;
}

void Player::takeDamage(const TickContext &ctx)
{
    m_lastDamageTime = age();

    //受伤时镜头震动
    if (ctx.events)
    {
        ctx.events->cameraShakes.append(CameraShakeEvent{20.0, 0.15, 28.0});
    }

    //受伤粒子
    if (ctx.world)
    {
        Particle::fireworks(ctx.world, sceneBoundingRect().center(), 10,
                            m_bodyRect.width() / 2.0,
                            m_bodyRect.height() / 2.0);
    }

    if (--m_health <= 0)
    {
        //死亡粒子
        if (ctx.world)
        {
            Particle::fireworks(ctx.world, sceneBoundingRect().center(), 50,
                                m_bodyRect.width() / 2.0,
                                m_bodyRect.height() / 2.0);
        }
        if (ctx.world)
        {
            ctx.world->destroyLater(this);
        }
    }
}
