#pragma once
#include <QHash>
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
        EnemySpawn=3,
        OneWayUp=4,
        OneWayDown=5,
        OneWayRight=6,
        OneWayLeft=7,
        ThroneSpawn=8,
        HachimiSpawn=9
    };

    static bool isOneWayWallType(TileType type);

    TileMap();
    int mapWidth() const;
    int mapHeight() const;
    QSize tileSize() const;
    int tileWidth() const;
    int tileHeight() const;
    TileType tileAt(int row,int col) const;
    bool isSolidTile(int row, int col) const;
    QPointF tileToScene(int row,int col) const;
    QPoint sceneToTile(QPointF scenePos) const;
    QPointF tileCenterToScene(int row,int col) const;
    QPointF tileBottomCenterToScene(int row,int col) const;
    QRectF sceneBounds() const;
    QPoint playerSpawnTile() const;
    QPointF playerSpawnScenePosition() const;
    QVector<QPoint> solidTilesOverlapping(const QRectF &sceneRect) const;

    bool tryPassOneWayWall(int row, int col, const QRectF &entityRect, qreal velX, qreal velY);
    bool isWallOpen(int row, int col) const;
    void updateOneWayWalls(qreal dt, const QVector<QRectF> &entityRects);

private:
    void openWall(int row, int col);
    QVector<QVector<int>> m_tiles;
    QSize m_tileSize;
    QPoint m_playerSpawnTile;
    mutable QHash<QPoint, qreal> m_openWalls;
    void initTiles();
    void setTileSize(QSize size);

};
