#ifndef PLAYER_H
#define PLAYER_H

#include "entities/actoritem.h"
#include <QPixmap>
#include <QRectF>

class TileMap;
class Enemy;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

enum class PlayerState
{
    Grounded,
    Airborne,
    WallSliding
};

class Player : public ActorItem
{
    Q_OBJECT
public:
    Player();

    void tick(const TickContext &ctx) override;
    EntityKind kind() const override { return EntityKind::Player; }
    Faction faction() const override { return Faction::Player; }
    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

    void takeDamage(const TickContext &ctx);
    void addHealth(int amount) { m_health += amount; }
    int health() const { return m_health; }

    void applyHachimiBoost();
    qreal hachimiRemaining() const { return m_hachimiRemaining; }

private:
    void moveHorizontally(qreal dt, TileMap &tileMap);
    void moveVertically(qreal dt, TileMap &tileMap);
    void resolveTileCollisionsX(TileMap &tileMap);
    void resolveTileCollisionsY(TileMap &tileMap);

    //状态机
    void updateState(const TickContext &ctx, bool jumpPressed);
    void behaveGrounded(const TickContext &ctx, bool jumpPressed);
    void behaveAirborne(const TickContext &ctx, bool jumpPressed);
    void behaveWallSliding(const TickContext &ctx, bool jumpPressed);

    //墙壁检测
    int detectWallSide(TileMap &tileMap) const;

    //攻击
    void processAttack(bool attackPressed, const TickContext &ctx);
    QSizeF m_spriteSize{50.0,50.0};
    QRectF m_bodyRect{-m_spriteSize.width()/2,-m_spriteSize.height()/2,m_spriteSize.width(),m_spriteSize.height()};//原固定值
    QPixmap m_idleSprite;
    QPixmap m_attackSprite;
    QPixmap m_idleWhiteSprite;
    QPixmap m_attackWhiteSprite;
    int m_health{3};

    //状态
    PlayerState m_state{PlayerState::Grounded};

    //朝向
    int m_facing{1};

    //柯基时间
    qreal m_coyoteTimer{0.0};

    //跳跃追踪
    int m_jumpsUsed{0};

    //墙壁交互
    int m_wallSide{0};
    qreal m_wallJumpLockTimer{0.0};

    //攻击(并行，非状态)
    qreal m_attackTimer{0.0};
    qreal m_attackCooldown{0.0};

    //边缘检测
    bool m_prevJump{false};
    bool m_prevAttack{false};

    //受伤闪烁
    qreal m_lastDamageTime{-9.0};

    //哈基米增强
    qreal m_hachimiRemaining{0.0};
    qreal m_hachimiMultiplier{1.0};
    qreal m_hachimiTrailTimer{0.0};
};

#endif // PLAYER_H
