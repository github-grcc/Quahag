#pragma once
#include "entities/actoritem.h"
#include"entities/player.h"
#include <QRectF>

class TileMap;
class QPainter;
// class Player;
class QStyleOptionGraphicsItem;
class QWidget;
enum class EnemyState{
    Patrol,
    Alert,
    Chase,
    Confused,
    Stunned
};
class Enemy : public ActorItem
{
    Q_OBJECT
public:
    Enemy();

    void tick(const TickContext &ctx) override;
    EntityKind kind() const override { return EntityKind::Enemy; }
    Faction faction() const override { return Faction::Enemy; }
    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;
    EnemyState state()const{return m_state;}
private:
    void updateState(const TickContext &ctx);
    void transitionTo(EnemyState newState, const TickContext &ctx);

    void behavePatrol(const TickContext &ctx);
    void behaveAlert(const TickContext &ctx);
    void behaveChase(const TickContext &ctx);
    void behaveConfused(const TickContext &ctx);
    void behaveStunned(const TickContext &ctx);
    
    bool checkTransitionToAlert(const TickContext &ctx);
    bool checkTransitionToChase(const TickContext &ctx);


    void applyMovement(qreal dt,const TileMap &tileMap);
    void applyGravity(qreal dt,const TileMap &tileMap, qreal gravity);
    void resolveTileCollisionsX(const TileMap &tileMap);
    void resolveTileCollisionsY(const TileMap &tileMap);

    void updateVision(const TickContext &ctx);
    bool isInVisionCone(const Player *player) const;
    bool hasLineOfSight(const QPointF &from, const QPointF &to,const TileMap &tileMap) const;
    QPointF eyePosition() const;

    bool hasGroundAhead(const TileMap &tileMap) const; 
    bool hasWallAhead(const TileMap &tileMap) const;
    
    void tryShoot(const TickContext &ctx);
public:
    void takeDamage(const TickContext &ctx);

    EnemyState m_state{EnemyState::Patrol};
    qreal m_stateTimer{0.0};

    int m_facing{1};
    int m_walkDirection{1};
    qreal m_patrolTimer{0.0};

    Player *m_seenPlayer{nullptr};
    qreal m_lastSeenPlayerTime{-9.0};
    qreal m_lastVisonCheckTime{0.0};
    qreal m_visionDistance{0.0};
    int m_presiousFacing{1};

    qreal m_shotCooldown{0.0};
    qreal m_aimAngle{0.0};

    int m_health{3};
    qreal m_lastDamageTime{-9.0};
    qreal m_seed{0.6};
    QRectF m_bodyRect{-12.0, -24.0, 24.0, 48.0};
};
