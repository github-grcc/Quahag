#include "graphics/tilelayeritem.h"

#include "world/tilemap.h"
#include<QStyleOptionGraphicsItem>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QtMath>
void TileLayerItem::initWoodTextures()                                                                                                                                                                                                                                                                                                                                                                   
{
    m_woodTextures.clear();
    for (int i = 0; i < m_textureCount; ++i) {
        QImage img = generateWoodTile(m_tileMap->tileWidth(), m_tileMap->tileHeight(), i + 1);
        m_woodTextures.append(QPixmap::fromImage(img));
    }
}
QImage TileLayerItem::generateWoodTile(int width, int height, int seed)
{
    QImage img(width, height, QImage::Format_ARGB32);
    QRandomGenerator rng(seed ? seed : QRandomGenerator::global()->generate());
    //底色
    QColor base(130, 90, 50);
    img.fill(base);

    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing, false);

    int stripeCount = 3 + rng.bounded(4);
    for (int i = 0; i < stripeCount; ++i) {
        //纹路
        QColor stripe(100, 65, 30);
        stripe.setRed(stripe.red()     + rng.bounded(10));
        stripe.setGreen(stripe.green() + rng.bounded(10));
        painter.setPen(QPen(stripe, 1 + rng.bounded(3)));

        QPainterPath path;
        int y = rng.bounded(height);
        path.moveTo(0, y);
        for (int x = 1; x < width; ++x) {
            int step = rng.bounded(3) - 1;
            y = qBound(0, y + step, height - 1);
            path.lineTo(x, y);
        }
        painter.drawPath(path);
    }

    // 木节
    int knotCount = rng.bounded(3);
    painter.setBrush(Qt::NoBrush);
    for (int i = 0; i < knotCount; ++i) {
        QColor knotColor(70, 40, 20);
        painter.setPen(QPen(knotColor, 1));
        int kx = rng.bounded(4, width - 4);
        int ky = rng.bounded(4, height - 4);
        QPoint points[5] = {
            {kx, ky - 1}, {kx - 1, ky}, {kx, ky}, {kx + 1, ky}, {kx, ky + 1}
        };
        painter.drawPoints(points, 5);
    }

    //噪点
    int noiseCount = width * height * 0.1;
    for (int i = 0; i < noiseCount; ++i) {
        int nx = rng.bounded(width);
        int ny = rng.bounded(height);
        QColor orig = img.pixelColor(nx, ny);
        QColor noise(orig.red()   - rng.bounded(25),
                     orig.green() - rng.bounded(20),
                     orig.blue()  - rng.bounded(20));
        img.setPixelColor(nx, ny, noise);
    }

    painter.end();
    return img;
}
TileLayerItem::TileLayerItem(const TileMap *tileMap, QGraphicsItem *parent)
    : QGraphicsItem(parent), m_tileMap(tileMap)
{
    initWoodTextures();
}

QRectF TileLayerItem::boundingRect() const
{
    if (!m_tileMap)
        return {};
    return m_tileMap->sceneBounds();
}

void TileLayerItem::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *option,
                          QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing, false); // 保持像素锐利
    painter->setPen(Qt::NoPen);

    if (!m_tileMap)
        return;

    QRandomGenerator rng(42); // 固定种子让每次刷新图案稳定
    QRectF exposed = option->exposedRect;
    int startRow = qMax(0, static_cast<int>(exposed.top() / m_tileMap->tileHeight()));
    int endRow = qMin(m_tileMap->mapHeight() - 1, static_cast<int>(exposed.bottom() / m_tileMap->tileHeight()));
    int startCol = qMax(0, static_cast<int>(exposed.left() / m_tileMap->tileWidth()));
    int endCol = qMin(m_tileMap->mapWidth() - 1, static_cast<int>(exposed.right() / m_tileMap->tileWidth()));
    for (int row = startRow; row <= endRow; ++row) {
        for (int col = startCol; col <= endCol; ++col) {
            if (m_tileMap->tileAt(row, col) != TileMap::TileType::Platform)
                continue;

            QRectF targetRect(m_tileMap->tileToScene(row, col),
                              m_tileMap->tileSize().toSizeF());

            // 随机选取一种预生成的木纹
            int texIdx = rng.bounded(m_woodTextures.size());
            painter->drawPixmap(targetRect.toRect(), m_woodTextures.at(texIdx));
        }
    }
}