#include "graphics/tilelayeritem.h"

#include "world/tilemap.h"

#include <QPainter>

TileLayerItem::TileLayerItem(const TileMap &tileMap, QGraphicsItem *parent)
    : QGraphicsItem(parent), m_tileMap(tileMap)
{
}

QRectF TileLayerItem::boundingRect() const
{
    return m_tileMap.sceneBounds();
}

void TileLayerItem::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *,
                          QWidget *)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(90, 204, 235));

    for (int row = 0; row < m_tileMap.mapHeight(); ++row) {
        for (int col = 0; col < m_tileMap.mapWidth(); ++col) {
            if (m_tileMap.tileAt(row, col) != TileMap::TileType::Platform)
                continue;
            painter->drawRect(QRectF(m_tileMap.tileToScene(row, col), m_tileMap.tileSize().toSizeF()));
        }
    }
}
