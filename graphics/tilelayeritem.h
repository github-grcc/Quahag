#ifndef TILELAYERITEM_H
#define TILELAYERITEM_H

#include <QGraphicsItem>
#include <QPixmap>
#include <QVector>
class TileMap;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

class TileLayerItem : public QGraphicsItem
{
public:
    explicit TileLayerItem(const TileMap *tileMap, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

private:
    QVector<QPixmap> m_woodTextures;   // 预生成的不同木纹
    int m_textureCount = 10; // 缓存 10 种随机纹理
    void initWoodTextures();
    QImage generateWoodTile(int width = 30, int height = 30, int seed = 0);
    const TileMap *m_tileMap;
};

#endif // TILELAYERITEM_H
