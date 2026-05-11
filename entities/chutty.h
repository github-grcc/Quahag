#ifndef CHUTTY_H
#define CHUTTY_H

#include "entities/actoritem.h"
#include <QPixmap>

class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

class Chutty : public ActorItem
{
    Q_OBJECT
public:
    Chutty();

    void tick(const TickContext &ctx) override;
    EntityKind kind() const override { return EntityKind::Chutty; }
    Faction faction() const override { return Faction::Neutral; }
    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

private:
    QSizeF m_spriteSize{50.0, 40.0};
    QRectF m_bodyRect{-m_spriteSize.width() / 2, -m_spriteSize.height() / 2,
                      m_spriteSize.width(), m_spriteSize.height()};
    QPixmap m_sprite;
};

#endif // CHUTTY_H
