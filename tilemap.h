#pragma once
#include<QVector>
#include<QPoint>
#include<QPointF>
#include<QSize>
class TileMap
{
public:
    TileMap();
    int mapWidth() const;
    int mapHeight() const;
    enum class TileType{
        Empty = 0,
        Platform=1,
        PlayerSpawn = 2
    };
    QSize tileSize() const;
    int tileWidth() const;
    int tileHeight() const;
    TileType tileAt(int row,int col) const;
    QPointF tileToScene(int row,int col) const;
    QPointF tileCenterToScene(int row,int col) const;
    QPointF tileBottomCenterToScene(int row,int col) const;
private:
    QVector<QVector<int>> m_tiles;
    QSize m_tileSize;
    QPoint m_playerSpawnTile;
    void initTiles();
    void setTileSize(QSize size);

};
