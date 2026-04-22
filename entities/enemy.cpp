#include "entities/enemy.h"
#include "world/gameworld.h"
#include "world/tilemap.h"

#include <QPainter>
#include<QtMath>
#include<QtNumeric>
#include<cmath>
#include <QtGlobal>

namespace {
constexpr qreal kRunSpeed = 420.0;
constexpr qreal kJumpImpulse = 820.0;
constexpr qreal kAirDrag = 10.0;
constexpr qreal kGroundDrag = 400.0;
constexpr qreal kAirAccel = 600.0;
constexpr qreal kGroundAccel = 1200.0;

constexpr qreal kPatrolSpeed=100.0;
constexpr qreal kChaseSpeed=150.0;
constexpr qreal kPatrolCycleDuration=8.0;
constexpr qreal kPatrolWalkDuration=5.0;
constexpr qreal kAlertDuration=0.3;
constexpr qreal kConfusedDuration=2.0;
constexpr qreal kStunnedDuration=0.5;
constexpr qreal kMaxVisionDistance=300.0;
constexpr qreal kVisionCheckInterval=0.2;
constexpr qreal kVisionTop=M_PI/4.0;
constexpr qreal kVisionBottom=M_PI/3.0;
constexpr qreal kShotInterval=0.2;
constexpr qreal kDamageShotInterval=0.5;
constexpr qreal kVisionGrowRate=1000;
} // namespace
Enemy::Enemy()
{
}
QRectF Enemy::boundingRect() const
{
    return m_bodyRect;
}
void Enemy::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(75, 207, 89));
    painter->drawRect(m_bodyRect);
}
void Enemy::tick(const TickContext &ctx)
{   
    if (!ctx.world)
        return;
    advanceAge(ctx.dt);
    m_stateTimer += ctx.dt;
    updateVision(ctx);
    updateState(ctx);
    applyGravity(ctx.dt, ctx.world->tileMap(), ctx.gravity);
    applyMovement(ctx.dt, ctx.world->tileMap());
    if(m_shotCooldown>0.0){
        m_shotCooldown-=ctx.dt;
    }
}
void Enemy::updateState(const TickContext &ctx)
{
    switch(m_state)
    {
    case EnemyState::Patrol:
        behavePatrol(ctx);
        if(checkTransitionToAlert(ctx)){
            transitionTo(EnemyState::Alert, ctx);
        }
        break;
    case EnemyState::Alert:
        behaveAlert(ctx);
        if(m_stateTimer>=kAlertDuration&&m_seenPlayer){
            transitionTo(EnemyState::Chase, ctx);
        }else if(!m_seenPlayer){
            transitionTo(EnemyState::Confused, ctx);
        }
        break;
    case EnemyState::Chase:
        behaveChase(ctx);
        if(!m_seenPlayer){
            transitionTo(EnemyState::Confused, ctx);
        }
        break;
    case EnemyState::Confused:
        behaveConfused(ctx);
        if(checkTransitionToAlert(ctx)){
            transitionTo(EnemyState::Alert, ctx);
        }else if(m_stateTimer>=kConfusedDuration){
            transitionTo(EnemyState::Patrol, ctx);
        }
        break;
    case EnemyState::Stunned:
        behaveStunned(ctx);
        if(m_stateTimer>=kStunnedDuration){
            transitionTo(m_seenPlayer?EnemyState::Chase:EnemyState::Patrol, ctx);
        }
        break;
    }
}
void Enemy::transitionTo(EnemyState newState, const TickContext &ctx)
{
    const EnemyState oldState = m_state;
    switch(m_state){
    case EnemyState::Patrol:
        m_patrolTimer=0.0;
        break;
    default:
        break;
    }
    m_state=newState;
    m_stateTimer=0.0;
    switch(m_state){
    case EnemyState::Alert:
        m_shotCooldown=qMax(m_shotCooldown,kAlertDuration);
        if(oldState==EnemyState::Patrol && ctx.events && !ctx.events->zoomPulseActive){
            ctx.events->cameraZoomPulses.append(CameraZoomPulseEvent{});
        }
        break;
    case EnemyState::Confused:
        setVelocityX(0.0);
        break;
    case EnemyState::Stunned:
        setVelocityX(0.0);
        break;
    default:
        break;
    }
}
void Enemy::behavePatrol(const TickContext &ctx)
{
    const TileMap &tileMap=ctx.world->tileMap();
    m_patrolTimer+=ctx.dt;
    const qreal cycleTime=fmod(age()+m_seed*kPatrolCycleDuration,kPatrolCycleDuration);
    const bool shouldWalk=cycleTime<kPatrolWalkDuration&&onGround()&&age()-m_lastDamageTime>0.5;
    if(shouldWalk){
        if(!hasGroundAhead(tileMap)||hasWallAhead(tileMap)){
            m_walkDirection=-m_walkDirection;
        }
        setVelocityX(m_walkDirection*kPatrolSpeed);
    }else{
        setVelocityX(0.0);
    }
    if(velocityX()!=0){
        m_facing=velocityX()>0?1:-1;
    }
    m_aimAngle=qAtan2(1.0,m_facing*0.5);
}
void Enemy::behaveAlert(const TickContext &ctx){
    setVelocityX(0.0);
    if(m_seenPlayer){
        
        


        const QPointF eye=eyePosition();
        m_aimAngle=qAtan2(m_seenPlayer->y()-eye.y(),m_seenPlayer->x()-eye.x());
    }
}
void Enemy::behaveChase(const TickContext &ctx){
    if(!m_seenPlayer)return;
    m_facing=m_seenPlayer->x()>x()?1:-1;
    const QPointF eye=eyePosition();
    m_aimAngle=qAtan2(m_seenPlayer->y()-eye.y(),m_seenPlayer->x()-eye.x());
    tryShoot(ctx);
}
void Enemy::behaveConfused(const TickContext &ctx){
    setVelocityX(0.0);
    const qreal lookAngle=qSin(age()*8.0)*M_PI/4;
    m_aimAngle=lookAngle+(m_facing>0?0:M_PI);
}
void Enemy::behaveStunned(const TickContext &ctx){
    setVelocityX(0.0);
}
bool Enemy::checkTransitionToAlert(const TickContext &ctx){
    return m_seenPlayer&&m_state!=EnemyState::Alert&&m_state!=EnemyState::Chase;
}
bool Enemy::checkTransitionToChase(const TickContext &ctx){
    return m_seenPlayer&&m_state==EnemyState::Alert;
}
void Enemy::applyGravity(qreal dt,const TileMap &tileMap,qreal gravity){
    setVelocityY(velocityY()+gravity*dt);
    moveBy(0.0,velocityY()*dt);
    setOnGround(false);
    resolveTileCollisionsY(tileMap);
}
void Enemy::applyMovement(qreal dt,const TileMap &tileMap){
    if(qFuzzyIsNull(velocityX()))return;
    moveBy(velocityX()*dt,0.0);
    resolveTileCollisionsX(tileMap);
}
void Enemy::resolveTileCollisionsX(const TileMap &tileMap){
    QRectF enemyRect=sceneBoundingRect();
    const auto tiles=tileMap.solidTilesOverlapping(enemyRect);
    for(const QPoint &tile:tiles){
        const QRectF tileRect(tileMap.tileToScene(tile.y(),tile.x()),tileMap.tileSize().toSizeF());
        if(!enemyRect.intersects(tileRect))continue;
        const QRectF overlap=enemyRect.intersected(tileRect);
        if(velocityX()>0.0){
            setX(x()-overlap.width());
            setVelocityX(0.0);
            m_walkDirection=-1;
        }else if(velocityX()<0.0){
            setX(x()+overlap.width());
            setVelocityX(0.0);
            m_walkDirection=1;
        }
        enemyRect=sceneBoundingRect();
    }
}
void Enemy::resolveTileCollisionsY(const TileMap &tileMap){
    QRectF enemyRect=sceneBoundingRect();
    const auto tiles=tileMap.solidTilesOverlapping(enemyRect);
    for(const QPoint &tile:tiles){
        const QRectF tileRect(tileMap.tileToScene(tile.y(),tile.x()),tileMap.tileSize().toSizeF());
        if (!enemyRect.intersects(tileRect)) continue;
        const QRectF overlap=enemyRect.intersected(tileRect);
        if (velocityY()>0.0) {
            setY(y()-overlap.height());
            setVelocityY(0.0);
            setOnGround(true);
        } else if (velocityY()<0.0) {
            setY(y()+overlap.height());
            setVelocityY(0.0);
        }
        enemyRect = sceneBoundingRect();
    }
}
void Enemy::updateVision(const TickContext &ctx){
    m_visionDistance=qMin(kMaxVisionDistance,m_visionDistance+ctx.dt*kVisionGrowRate);
    if(m_facing!=m_presiousFacing){
        m_visionDistance=0.0;
        m_presiousFacing=m_facing;
    }
    if(age()-m_lastVisonCheckTime<kVisionCheckInterval)return;
    m_lastVisonCheckTime=age();
    m_seenPlayer=nullptr;
    Player *player=ctx.world->player();
    if(!player)return;
    if(!isInVisionCone(player))return;
    const QPointF eye=eyePosition();
    const qreal dist=QLineF(eye,player->pos()).length();
    if(dist>m_visionDistance)return;
    if(!hasLineOfSight(eye,player->pos(),ctx.world->tileMap()))return;
    m_seenPlayer=player;
}
bool Enemy::isInVisionCone(const Player *player)const{
    QPointF eye=eyePosition();
    const qreal angleToPlayer=qAtan2(player->y()-eye.y(),player->x()-eye.x());
    const qreal baseAngle=m_facing>0?0.0:M_PI;
    qreal angleDiff=angleToPlayer-baseAngle;
    while(angleDiff>M_PI)angleDiff-=2*M_PI;
    while(angleDiff<-M_PI)angleDiff+=2*M_PI;
    const qreal halfCone=player->y()>y()?kVisionBottom:kVisionTop;
    return qAbs(angleDiff)<=halfCone;
}
bool Enemy::hasLineOfSight(const QPointF &from,const QPointF &to,const TileMap &tileMap)const{
    const qreal dy=to.y()-from.y();
    const qreal dx=to.x()-from.x();
    const qreal dist= qSqrt(dx*dx+dy*dy);
    const int steps=qCeil(dist/tileMap.tileWidth());
    for(int i=0;i<=steps;++i){
        const qreal t=i/static_cast<qreal>(steps);
        const qreal checkX=from.x()+dx*t;
        const qreal checkY=from.y()+dy*t;
        if(tileMap.isSolidTile(static_cast<int>(checkY/tileMap.tileHeight()),static_cast<int>(checkX/tileMap.tileWidth())))
            return false;
    }
    return true;
}
QPointF Enemy::eyePosition()const{
    return QPointF(x(),y());
}
bool Enemy::hasGroundAhead(const TileMap &tileMap)const{
    const qreal checkX=x()+m_facing*(m_bodyRect.width()/2+5);
    const qreal checkY=y()+m_bodyRect.height()/2+5;
    return tileMap.isSolidTile(static_cast<int>(checkY/tileMap.tileHeight()),static_cast<int>(checkX/tileMap.tileWidth()));
}
bool Enemy::hasWallAhead(const TileMap &tileMap)const{
    const qreal checkX=x()+m_facing*(m_bodyRect.width()/2+2);
    const qreal checkY=y();
    return tileMap.isSolidTile(static_cast<int>(checkY/tileMap.tileHeight()),static_cast<int>(checkX/tileMap.tileWidth()));
}
void Enemy::tryShoot(const TickContext &ctx){
    if(m_shotCooldown>0||!m_seenPlayer)return;
    // auto *bullet = new Bullet(this);
    // bullet->setPos(x() + m_facing * 10 + qCos(m_aimAngle) * 35,
    //                y() - 20 + qSin(m_aimAngle) * 35);
    // ctx.world->spawn(bullet);
    m_shotCooldown = kShotInterval;
}
void Enemy::takeDamage(const TickContext &ctx){
    m_lastDamageTime=age();
    m_shotCooldown=qMax(m_shotCooldown,kDamageShotInterval);
    if(--m_health<=0){
        if(ctx.events && ctx.events->zoomPulseActive){
            ctx.events->stopZoomPulseRequested=true;
        }
        ctx.world->destroyLater(this);
    }else{
        transitionTo(EnemyState::Stunned, ctx);
    }
}