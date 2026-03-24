#ifndef PLAYER_H
#define PLAYER_H
#include<QGraphicsItem>
#include<QPointF>
struct InputState {
    bool moveLeft{false};
    bool moveRight{false};
    bool jump{false};
};

class Player:public QGraphicsRectItem
{
public:
    Player();
    void simulate(qreal dt,const InputState &input,const QList<QGraphicsItem*> &platforms,qreal gravity);
    QPointF velocity()const{return m_velocity;}
    bool onGround()const{return m_onGround;}
private:
    void resolveCollisions(const QList<QGraphicsItem*> &platforms);
    QPointF m_velocity{0.0,0.0};
    bool m_onGround{false};

};

#endif // PLAYER_H
