#pragma once
#include <QRectF>
#include <QVector>
#include <QPoint>
#include <QPointF>
#include <QSize>

class TileMap
{
public:
    enum class TileType{
        Empty = 0,
        Platform=1,
        PlayerSpawn = 2,
        EnemySpawn=3
    };

    TileMap();
    int mapWidth() const;
    int mapHeight() const;
    QSize tileSize() const;
    int tileWidth() const;
    int tileHeight() const;
    TileType tileAt(int row,int col) const;
    bool isSolidTile(int row, int col) const;
    QPointF tileToScene(int row,int col) const;
    QPointF tileCenterToScene(int row,int col) const;
    QPointF tileBottomCenterToScene(int row,int col) const;
    QRectF sceneBounds() const;
    QPoint playerSpawnTile() const;
    QPointF playerSpawnScenePosition() const;
    QVector<QPoint> solidTilesOverlapping(const QRectF &sceneRect) const;
private:
    QVector<QVector<int>> m_tiles;
    QSize m_tileSize;
    QPoint m_playerSpawnTile;
    void initTiles();
    void setTileSize(QSize size);

};
