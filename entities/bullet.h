// #pragma once
// #include "entities/actoritem.h"
// #include <QRectF>
// class TileMap;
// class QPainter;
// class QStyleOptionGraphicsItem;
// class QWidget;
// class Bullet : public ActorItem
// {
//     Q_OBJECT
// public:
//     Bullet();

//     void tick(const TickContext &ctx) override;
//     EntityKind kind() const override { return EntityKind::Player; }
//     Faction faction() const override { return Faction::Player; }
//     QRectF boundingRect() const override;
//     void paint(QPainter *painter,
//                const QStyleOptionGraphicsItem *option,
//                QWidget *widget = nullptr) override;

// private:
//     QRectF m_bodyRect{-12.0, -24.0, 24.0, 48.0};
// };