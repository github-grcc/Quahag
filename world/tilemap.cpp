#include "world/tilemap.h"

#include <QFile>
#include <QTextStream>
#include <QtGlobal>
#include "tilemap.h"
namespace
{
    qreal kWallOpenDuration = 1.0;
}
int TileMap::mapWidth() const
{
    return m_tiles.isEmpty() ? 0 : m_tiles[0].size();
}
int TileMap::mapHeight() const
{
    return m_tiles.size();
}
QSize TileMap::tileSize() const
{
    return m_tileSize;
}
void TileMap::setTileSize(QSize size)
{
    m_tileSize = size;
}
int TileMap::tileWidth() const
{
    return m_tileSize.width();
}
int TileMap::tileHeight() const
{
    return m_tileSize.height();
}
TileMap::TileType TileMap::tileAt(int row, int col) const
{
    if (row < 0 || row >= m_tiles.size())
        return TileType::Empty;
    if (col < 0 || col >= m_tiles[row].size())
        return TileType::Empty;
    return static_cast<TileType>(m_tiles[row][col]);
}
bool TileMap::isSolidTile(int row, int col) const
{
    TileType type = tileAt(row, col);
    if (type == TileType::Platform)
        return true;
    if (isOneWayWallType(type))
        return !isWallOpen(row, col);
    return false;
}
QPointF TileMap::tileToScene(int row, int col) const
{
    return QPointF(col * tileWidth(), row * tileHeight());
}
QPointF TileMap::tileCenterToScene(int row, int col) const
{
    return QPointF((col + 0.5) * tileWidth(), (row + 0.5) * tileHeight());
}
QPointF TileMap::tileBottomCenterToScene(int row, int col) const
{
    return QPointF((col + 0.5) * tileWidth(), (row + 1.0) * tileHeight());
}
QRectF TileMap::sceneBounds() const
{
    return QRectF(0.0, 0.0, mapWidth() * tileWidth(), mapHeight() * tileHeight());
}
QPoint TileMap::playerSpawnTile() const
{
    return m_playerSpawnTile;
}
QPointF TileMap::playerSpawnScenePosition() const
{
    return tileCenterToScene(m_playerSpawnTile.y(), m_playerSpawnTile.x());
}
QVector<QPoint> TileMap::solidTilesOverlapping(const QRectF &sceneRect) const
{
    QVector<QPoint> result;
    if (sceneRect.isEmpty())
        return result;

    const int leftCol = qMax(0, static_cast<int>(sceneRect.left() / tileWidth()));
    const int rightCol = qMin(mapWidth() - 1, static_cast<int>((sceneRect.right() - 0.001) / tileWidth()));
    const int topRow = qMax(0, static_cast<int>(sceneRect.top() / tileHeight()));
    const int bottomRow = qMin(mapHeight() - 1, static_cast<int>((sceneRect.bottom() - 0.001) / tileHeight()));

    for (int row = topRow; row <= bottomRow; ++row)
    {
        for (int col = leftCol; col <= rightCol; ++col)
        {
            if (isSolidTile(row, col))
                result.append(QPoint(col, row));
        }
    }
    return result;
}
TileMap::TileMap()
{
    initTiles();
    setTileSize(QSize(25, 25));
}
void TileMap::initTiles()
{
    QFile file(":/rsc/map3.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning("TileMap: Failed to open rsc/map3.txt");
        return;
    }

    QTextStream in(&file);

    // 读取第一行：宽度和高度
    int width, height;
    in >> width >> height;
    in.readLine(); // 消耗第一行剩余内容

    m_tiles.reserve(height);
    for (int row = 0; row < height; ++row)
    {
        const QString line = in.readLine();
        QVector<int> tileRow;
        tileRow.reserve(width);
        for (int col = 0; col < width && col < line.length(); ++col)
        {
            const QChar ch = line[col];
            int tileValue;
            if (ch == 'u')
                tileValue = 4;
            else if (ch == 'd')
                tileValue = 5;
            else if (ch == 'r')
                tileValue = 6;
            else if (ch == 'l')
                tileValue = 7;
            else if (ch == 'w') // 王座
                tileValue = 8;
            else if (ch == 's') // 哈基米
                tileValue = 9;
            else if (ch == 't') // 口香糖
                tileValue = 10;
            else
                tileValue = ch.digitValue();
            tileRow.append(tileValue);
        }
        // 如果行短于预期宽度，用空填充
        while (tileRow.size() < width)
            tileRow.append(0);
        m_tiles.append(tileRow);
    }

    file.close();

    // 查找玩家生成瓦片
    for (int row = 0; row < m_tiles.size(); ++row)
    {
        for (int col = 0; col < m_tiles[row].size(); ++col)
        {
            if (m_tiles[row][col] == static_cast<int>(TileType::PlayerSpawn))
            {
                m_playerSpawnTile = QPoint(col, row);
                return;
            }
        }
    }

    m_playerSpawnTile = QPoint();
}

bool TileMap::isOneWayWallType(TileType type)
{
    return type == TileType::OneWayUp || type == TileType::OneWayDown || type == TileType::OneWayRight || type == TileType::OneWayLeft;
}

bool TileMap::tryPassOneWayWall(int row, int col, const QRectF &entityRect,
                                qreal velX, qreal velY)
{
    TileType type = tileAt(row, col);
    if (!isOneWayWallType(type))
        return false;

    QPointF tileCenter = QRectF(tileToScene(row, col), m_tileSize.toSizeF()).center();
    QPointF entityCenter = entityRect.center();

    bool canPass = false;
    switch (type)
    {
    case TileType::OneWayRight:
        canPass = entityCenter.x() < tileCenter.x() && velX > 0.0;
        break;
    case TileType::OneWayLeft:
        canPass = entityCenter.x() > tileCenter.x() && velX < 0.0;
        break;
    case TileType::OneWayUp:
        canPass = entityCenter.y() > tileCenter.y() && velY < 0.0;
        break;
    case TileType::OneWayDown:
        canPass = entityCenter.y() < tileCenter.y() && velY > 0.0;
        break;
    default:
        break;
    }
    if (!canPass)
        return false;

    openWall(row, col);
    return true;
}

bool TileMap::isWallOpen(int row, int col) const
{
    return m_openWalls.contains(QPoint(col, row));
}

void TileMap::updateOneWayWalls(qreal dt, const QVector<QRectF> &entityRects)
{
    QList<QPoint> toRemove;
    for (auto it = m_openWalls.begin(); it != m_openWalls.end(); ++it)
    {
        const QPoint &tile = it.key();
        qreal &remaining = it.value();

        QRectF tileRect(tileToScene(tile.y(), tile.x()), m_tileSize.toSizeF());

        bool anyOverlap = false;
        for (const QRectF &rect : entityRects)
        {
            if (rect.intersects(tileRect))
            {
                anyOverlap = true;
                break;
            }
        }

        if (anyOverlap)
        {
            remaining = kWallOpenDuration;
        }
        else
        {
            remaining -= dt;
            if (remaining <= 0.0)
                toRemove.append(tile);
        }
    }

    for (const QPoint &tile : toRemove)
        m_openWalls.remove(tile);
}

void TileMap::openWall(int row, int col)
{
    m_openWalls[QPoint(col, row)] = 0.5;
}
