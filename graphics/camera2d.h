#ifndef CAMERA2D_H
#define CAMERA2D_H

#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QTransform>

class Camera2D
{
public:
    void setSceneBounds(const QRectF &bounds);
    void setViewportSize(const QSizeF &size);
    void setTargetCenter(const QPointF &center);
    void setTargetZoom(qreal zoom);
    void setFollowResponsiveness(qreal responsiveness);
    void setFollowDamping(qreal damping);
    void setZoomResponsiveness(qreal responsiveness);
    void snapToTarget();
    void update(qreal dt);

    void startZoomPulse(qreal amplitude,
                        qreal duration,
                        qreal cycles = 0.5,
                        qreal center = 1.0,
                        qreal initialPhase = 0.0);
    void addShake(qreal amplitude, qreal duration, qreal frequency = 28.0);

    QTransform transform() const;
    QPointF currentCenter() const { return m_currentCenter; }
    qreal currentZoom() const { return m_currentZoom; }

private:
    QPointF clampedBaseCenter() const;
    QPointF effectiveCenter() const;
    qreal effectiveZoom() const;
    static qreal expSmoothingAlpha(qreal responsiveness, qreal dt);

    QRectF m_sceneBounds;
    QSizeF m_viewportSize;

    QPointF m_targetCenter;
    QPointF m_currentCenter;
    QPointF m_cameraVelocity;
    qreal m_targetZoom{1.0};
    qreal m_currentZoom{1.0};
    qreal m_followResponsiveness{8.0};
    qreal m_followDamping{0.1};
    qreal m_zoomResponsiveness{10.0};
    bool m_initialized{false};

    qreal m_zoomPulseAmplitude{0.0};
    qreal m_zoomPulseDuration{0.0};
    qreal m_zoomPulseElapsed{0.0};
    qreal m_zoomPulseCycles{0.5};
    qreal m_zoomPulseCenter{1.0};
    qreal m_zoomPulseInitialPhase{0.0};

    qreal m_shakeAmplitude{0.0};
    qreal m_shakeDuration{0.0};
    qreal m_shakeElapsed{0.0};
    qreal m_shakeFrequency{28.0};
};

#endif // CAMERA2D_H
