#include "graphics/parallaxbackground.h"

#include <QPainter>
#include <QtMath>

static const QString kBackgroundDir = QStringLiteral(":/rsc/background/");

ParallaxBackground::ParallaxBackground()
{
    loadImages();
}

void ParallaxBackground::loadImages()
{
    auto loadPix = [](const QString &path) {
        QPixmap p;
        p.load(path);
        return p;
    };

    //从后到前：天空→沙滩→海→红树林→椰树
    m_layers = {
        { loadPix(kBackgroundDir + "sky.png"), {}, {}, 0.0 },
        { loadPix(kBackgroundDir + "beach.png"), {}, {}, 0.50 },
        { loadPix(kBackgroundDir + "sea.png"), {}, {}, 0.28 },
        { loadPix(kBackgroundDir + "mangrove.png"), {}, {}, 0.12 },
        { loadPix(kBackgroundDir + "coco.png"), {}, {}, 0.50 },
    };
}

void ParallaxBackground::paint(QPainter *painter,
                                const QSizeF &viewportSize,
                                const QPointF &cameraCenter,
                                qreal zoom)
{
    if (viewportSize.isEmpty())
        return;

    if (!qIsFinite(cameraCenter.x()) || !qIsFinite(cameraCenter.y())
        || !qIsFinite(zoom))
        return;

    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    const QSize vs = viewportSize.toSize();

    for (auto &layer : m_layers) {
        if (layer.original.isNull())
            continue;

        //有效缩放：k=0恒为1倍；k=1为完整缩放
        const qreal zk = 1.0 + layer.parallaxFactor * (zoom - 1.0);
        const QSize scaledSize = vs * zk;

        if (layer.scaled.isNull() || layer.lastViewport != scaledSize) {
            layer.scaled = layer.original.scaled(scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            layer.lastViewport = scaledSize;
        }

        const int tw = layer.scaled.width();
        const int th = layer.scaled.height();
        if (tw <= 0)
            continue;

        qreal offsetX = std::fmod(cameraCenter.x() * layer.parallaxFactor * zoom,
                                   static_cast<qreal>(tw));
        if (offsetX < 0.0)
            offsetX += tw;

        const qreal vertPos = -(th - viewportSize.height()) * 0.5;

        for (qreal x = -offsetX; x < viewportSize.width(); x += tw) {
            painter->drawPixmap(QPointF(x, vertPos), layer.scaled);
        }
    }
}
