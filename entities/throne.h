#ifndef THRONE_H
#define THRONE_H

#include "entities/actoritem.h"
#include <QPixmap>

class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

class Throne : public ActorItem
{
    Q_OBJECT
public:
    Throne();

    void tick(const TickContext &ctx) override;
    EntityKind kind() const override { return EntityKind::Throne; }
    Faction faction() const override { return Faction::Neutral; }
    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

private:
    QSizeF m_spriteSize{35.0, 55.0};
    QRectF m_bodyRect{-m_spriteSize.width() / 2, -m_spriteSize.height() / 2,
                      m_spriteSize.width(), m_spriteSize.height()};
    QPixmap m_sprite;
};

#endif // THRONE_H
