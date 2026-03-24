#include "player.h"
#include<QGraphicsItem>
#include<QBrush>
#include<QPen>
#include<QtGlobal>
namespace {
// 玩家运动常量：奔跑速度、跳跃初速度和空气阻力系数。
constexpr qreal kRunSpeed = 220.0;
constexpr qreal kJumpImpulse = 520.0;
constexpr qreal kAirDrag = 10.0;
constexpr qreal kGroundDrag = 100.0;

} // namespace

Player::Player() {
    setRect(-12.0,-24.0,24.0,48.0);
    setBrush(QBrush(QColor(253,184,39)));

}
void Player::simulate(qreal dt,const InputState &input,const QList<QGraphicsItem*> &platforms,qreal gravity){
    // 读取输入：同时按左右键会相互抵消。
    const bool movingLeft = input.moveLeft && !input.moveRight;
    const bool movingRight = input.moveRight && !input.moveLeft;

    // 水平移动：直接设定目标速度，若无输入则进行简易阻尼减速。
    if (movingLeft) {
        m_velocity.setX(-kRunSpeed);
    } else if (movingRight) {
        m_velocity.setX(kRunSpeed);
    } else {
        // Basic drag to slow down when idle.
        qreal decel=0.0;
        if(!m_onGround){
            decel = kAirDrag * dt;
        }else{
            decel = kGroundDrag * dt;
        }
        if (qAbs(m_velocity.x()) <= decel) {
            m_velocity.setX(0.0);
        } else if (m_velocity.x() > 0) {
            m_velocity.setX(m_velocity.x() - decel);
        } else {
            m_velocity.setX(m_velocity.x() + decel);
        }
    }

    // 跳跃：只有在接触地面时才允许产生向上的初速度。
    if (input.jump && m_onGround) {
        m_velocity.setY(-kJumpImpulse);
        m_onGround = false;
    }

    // 重力：每帧累加到垂直速度。
    m_velocity.setY(m_velocity.y() + gravity * dt);
    // 根据速度积分位置。
    moveBy(m_velocity.x() * dt, m_velocity.y() * dt);

    // 先假设离开地面，碰撞处理中再恢复。
    m_onGround = false;
    resolveCollisions(platforms);
};
void Player::resolveCollisions(const QList<QGraphicsItem*>&platforms){
    // 简单的 AABB（轴对齐包围盒）碰撞：检测重叠并沿最小轴分离。
    for (QGraphicsItem *platform : platforms) {
        const QRectF playerRect = sceneBoundingRect();
        const QRectF platformRect = platform->sceneBoundingRect();

        if (!playerRect.intersects(platformRect))
            continue;

        const QRectF overlap = playerRect.intersected(platformRect);
        if (overlap.isEmpty())
            continue;

        // 决定从垂直还是水平方向分离：重叠更小的方向优先。
        if (overlap.height() < overlap.width()) {
            if (playerRect.center().y() < platformRect.center().y()) {
                // 顶部碰撞：把玩家推出平台上方并清空下落速度。
                setY(y() - overlap.height());
                m_velocity.setY(0.0);
                m_onGround = true;
            } else {
                // 底部碰撞：阻止继续向下穿透。
                setY(y() + overlap.height());
                m_velocity.setY(qMax(0.0, m_velocity.y()));
            }
        } else {
            if (playerRect.center().x() < platformRect.center().x()) {
                // 左侧碰撞。
                setX(x() - overlap.width());
                m_velocity.setX(qMin(0.0, m_velocity.x()));
            } else {
                // 右侧碰撞。
                setX(x() + overlap.width());
                m_velocity.setX(qMax(0.0, m_velocity.x()));
            }
        }
    }
}
