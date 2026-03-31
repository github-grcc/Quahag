#ifndef TILELAYERITEM_H
#define TILELAYERITEM_H

#include <QGraphicsItem>

class TileMap;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

class TileLayerItem : public QGraphicsItem
{
public:
    explicit TileLayerItem(const TileMap &tileMap, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

private:
    const TileMap &m_tileMap;
};

#endif // TILELAYERITEM_H
