#include "graphics/camera2d.h"

#include <QtGlobal>
#include <QtMath>

void Camera2D::setSceneBounds(const QRectF &bounds)
{
    m_sceneBounds = bounds;
}

void Camera2D::setViewportSize(const QSizeF &size)
{
    m_viewportSize = size;
}

void Camera2D::setTargetCenter(const QPointF &center)
{
    m_targetCenter = center;
    if (!m_initialized) {
        m_currentCenter = center;
        m_cameraVelocity = QPointF();
        m_initialized = true;
    }
}

void Camera2D::setTargetZoom(qreal zoom)
{
    m_targetZoom = qMax(0.1, zoom);
    if (!m_initialized) {
        m_currentZoom = m_targetZoom;
    }
}

void Camera2D::setFollowResponsiveness(qreal responsiveness)
{
    m_followResponsiveness = qMax(0.0, responsiveness);
}

void Camera2D::setFollowDamping(qreal damping)
{
    m_followDamping = qBound(0.0, damping, 1.0);
}

void Camera2D::setZoomResponsiveness(qreal responsiveness)
{
    m_zoomResponsiveness = qMax(0.0, responsiveness);
}

void Camera2D::snapToTarget()
{
    if (!m_initialized)
        return;

    m_currentCenter = m_targetCenter;
    m_cameraVelocity = QPointF();
    m_currentZoom = m_targetZoom;
}

void Camera2D::update(qreal dt)
{
    if (!m_initialized)
        return;

    const qreal safeDt = qMax(0.0, dt);
    const qreal zoomAlpha = expSmoothingAlpha(m_zoomResponsiveness, safeDt);
    m_currentZoom += (m_targetZoom - m_currentZoom) * zoomAlpha;

    if (safeDt > 0.0) {
        const QPointF displacement = m_targetCenter - m_currentCenter;
        const QPointF acceleration = displacement * m_followResponsiveness;
        const qreal dampingFactor = qPow(1.0 - m_followDamping, safeDt * 60.0);
        m_cameraVelocity = m_cameraVelocity * dampingFactor + acceleration * safeDt;
        m_currentCenter += m_cameraVelocity * safeDt;
    } else {
        m_currentCenter = m_targetCenter;
        m_cameraVelocity = QPointF();
    }

    if (m_zoomPulseDuration > 0.0) {
        m_zoomPulseElapsed = qMin(m_zoomPulseElapsed + safeDt, m_zoomPulseDuration);
    }

    if (m_shakeDuration > 0.0) {
        m_shakeElapsed = qMin(m_shakeElapsed + safeDt, m_shakeDuration);
    }
}

void Camera2D::startZoomPulse(qreal amplitude,
                              qreal duration,
                              qreal cycles,
                              qreal center,
                              qreal initialPhase)
{
    m_zoomPulseAmplitude = qMax(0.0, amplitude);
    m_zoomPulseDuration = qMax(0.0, duration);
    m_zoomPulseElapsed = 0.0;
    m_zoomPulseCycles = qMax(0.0, cycles);
    m_zoomPulseCenter = center >= 0.0 ? qMax(0.1, center) : qMax(0.1, m_targetZoom);
    m_zoomPulseInitialPhase = initialPhase;
}

void Camera2D::addShake(qreal amplitude, qreal duration, qreal frequency)
{
    m_shakeAmplitude = qMax(m_shakeAmplitude, qMax(0.0, amplitude));
    m_shakeDuration = qMax(m_shakeDuration, qMax(0.0, duration));
    m_shakeElapsed = 0.0;
    m_shakeFrequency = qMax(0.0, frequency);
}

QTransform Camera2D::transform() const
{
    if (!m_initialized || m_viewportSize.isEmpty())
        return QTransform();

    const qreal zoom = effectiveZoom();
    const QPointF center = effectiveCenter();
    const qreal halfViewWidth = m_viewportSize.width() * 0.5;
    const qreal halfViewHeight = m_viewportSize.height() * 0.5;

    const qreal tx = halfViewWidth - zoom * (center.x() - m_sceneBounds.left());
    const qreal ty = halfViewHeight - zoom * (center.y() - m_sceneBounds.top());
    return QTransform(zoom, 0.0, 0.0,
                      0.0, zoom, 0.0,
                      tx, ty, 1.0);
}

QPointF Camera2D::clampedBaseCenter() const
{
    if (m_viewportSize.isEmpty())
        return m_currentCenter;

    const qreal zoom = effectiveZoom();
    const qreal halfVisibleWidth = m_viewportSize.width() / (2.0 * zoom);
    const qreal halfVisibleHeight = m_viewportSize.height() / (2.0 * zoom);
    QPointF center = m_currentCenter;

    if (m_sceneBounds.width() > halfVisibleWidth * 2.0) {
        center.setX(qBound(m_sceneBounds.left() + halfVisibleWidth,
                           center.x(),
                           m_sceneBounds.right() - halfVisibleWidth));
    } else {
        center.setX(m_sceneBounds.center().x());
    }

    if (m_sceneBounds.height() > halfVisibleHeight * 2.0) {
        center.setY(qBound(m_sceneBounds.top() + halfVisibleHeight,
                           center.y(),
                           m_sceneBounds.bottom() - halfVisibleHeight));
    } else {
        center.setY(m_sceneBounds.center().y());
    }

    return center;
}

QPointF Camera2D::effectiveCenter() const
{
    QPointF center = clampedBaseCenter();
    if (m_shakeDuration <= 0.0 || m_shakeElapsed >= m_shakeDuration)
        return center;

    const qreal progress = m_shakeElapsed / m_shakeDuration;
    const qreal envelope = 1.0 - progress;
    const qreal angle = 2.0 * M_PI * m_shakeFrequency * m_shakeElapsed;
    center.rx() += qSin(angle) * m_shakeAmplitude * envelope;
    center.ry() += qSin(angle * 1.61803398875) * m_shakeAmplitude * 0.7 * envelope;
    return center;
}

qreal Camera2D::effectiveZoom() const
{
    qreal zoom = qMax(0.1, m_currentZoom);
    if (m_zoomPulseDuration <= 0.0 || m_zoomPulseElapsed >= m_zoomPulseDuration)
        return zoom;

    const qreal progress = m_zoomPulseElapsed / m_zoomPulseDuration;
    const qreal envelope = 1.0 - progress;
    const qreal angle = progress * m_zoomPulseCycles * 2.0 * M_PI + m_zoomPulseInitialPhase;
    zoom = m_zoomPulseCenter + qSin(angle) * m_zoomPulseAmplitude;// * envelope;
    return qMax(0.1, zoom);
}

qreal Camera2D::expSmoothingAlpha(qreal responsiveness, qreal dt)
{
    if (responsiveness <= 0.0 || dt <= 0.0)
        return 1.0;
    return 1.0 - qExp(-responsiveness * dt);
}
