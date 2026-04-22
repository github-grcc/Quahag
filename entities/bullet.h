#pragma once
#include "entities/actoritem.h"
#include"core/tickcontext.h"
#include <QRectF>
class TileMap;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class Bullet : public ActorItem
{
    Q_OBJECT
public:
    Bullet(ActorItem *owner, QPointF spawnPosition, qreal shootAngle, qreal speed);
    void tick(const TickContext &ctx) override;
    EntityKind kind() const override { return EntityKind::Bullet; }
    Faction faction() const override { return Faction::Enemy; }
    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;
private:
    bool checkCollision(const TickContext &ctx);
    ActorItem *m_owner{nullptr};
    qreal m_shootAngle{0.0};
    qreal m_speed{100.0};
    QPointF m_spawnPosition{0.0, 0.0};
    QRectF m_bodyRect{-5.0,-5.0,10.0,10.0};
};