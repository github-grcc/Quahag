#ifndef PARALLAXBACKGROUND_H
#define PARALLAXBACKGROUND_H

#include <QPixmap>
#include <QPointF>
#include <QSizeF>
#include <QVector>

class QPainter;

class ParallaxBackground
{
public:
    ParallaxBackground();

    void paint(QPainter *painter,
               const QSizeF &viewportSize,
               const QPointF &cameraCenter,
               qreal zoom);

private:
    struct Layer {
        QPixmap original;
        QPixmap scaled;
        QSize lastViewport;
        qreal parallaxFactor;
    };

    QVector<Layer> m_layers;

    void loadImages();
};

#endif // PARALLAXBACKGROUND_H
