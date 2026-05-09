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
    int health() const { return m_health; }

private:
    void moveHorizontally(qreal dt, TileMap &tileMap);
    void moveVertically(qreal dt, TileMap &tileMap);
    void resolveTileCollisionsX(TileMap &tileMap);
    void resolveTileCollisionsY(TileMap &tileMap);

    // State machine
    void updateState(const TickContext &ctx, bool jumpPressed);
    void behaveGrounded(const TickContext &ctx, bool jumpPressed);
    void behaveAirborne(const TickContext &ctx, bool jumpPressed);
    void behaveWallSliding(const TickContext &ctx, bool jumpPressed);

    // Wall detection
    int detectWallSide(TileMap &tileMap) const;

    // Attack
    void processAttack(bool attackPressed, const TickContext &ctx);
    QSizeF m_spriteSize{50.0,50.0};
    QRectF m_bodyRect{-m_spriteSize.width()/2,-m_spriteSize.height()/2,m_spriteSize.width(),m_spriteSize.height()};//{-12.0, -24.0, 24.0, 48.0};
    QPixmap m_idleSprite;
    QPixmap m_attackSprite;
    QPixmap m_idleWhiteSprite;
    QPixmap m_attackWhiteSprite;
    int m_health{3};

    // State
    PlayerState m_state{PlayerState::Grounded};

    // Facing
    int m_facing{1};

    // Coyote time
    qreal m_coyoteTimer{0.0};

    // Jump tracking
    int m_jumpsUsed{0};

    // Wall interaction
    int m_wallSide{0};
    qreal m_wallJumpLockTimer{0.0};

    // Attack (parallel, not a state)
    qreal m_attackTimer{0.0};
    qreal m_attackCooldown{0.0};

    // Edge detection
    bool m_prevJump{false};
    bool m_prevAttack{false};

    // Damage flash
    qreal m_lastDamageTime{-9.0};
};

#endif // PLAYER_H
