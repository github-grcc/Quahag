#ifndef GAMEVIEW_H
#define GAMEVIEW_H
#include "core/gameloop.h"
#include "graphics/camera2d.h"
#include "world/gamescene.h"
#include"core/tickcontext.h"
#include <QGraphicsView>
#include <QPointer>

class QPainter;

class GameView : public QGraphicsView{
    Q_OBJECT

public:
    explicit GameView(QWidget *parent = nullptr);
    ~GameView() override;
    void setCameraZoom(qreal zoom);
    void startCameraZoomPulse(CameraZoomPulseEvent zoomPulseEvent);
    void addCameraShake(CameraShakeEvent shakeEvent);
private slots:
    void addShake(CameraShakeEvent shakeEvent);
    void startZoomPulse(CameraZoomPulseEvent zoomPulseEvent);
    void stopZoomPulse();
protected:
    void keyPressEvent(QKeyEvent* event)override;
    void keyReleaseEvent(QKeyEvent* event)override;
    void resizeEvent(QResizeEvent *event)override;
    void wheelEvent(QWheelEvent *event) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;
private:
    void applyCameraTransform();
    void updateCamera(qreal dt = 0.0);
    void resetGame();
    QPointer<GameScene> m_scene;
    InputState m_input;
    bool m_zoomPulseRequested{false};
    bool m_shakeRequested{false};
    CameraShakeEvent m_shakeEvent;
    CameraZoomPulseEvent m_zoomPulseEvent;

    GameLoop m_loop;
    Camera2D m_camera;

    enum class GameState { Title, Playing, GameOver };
    GameState m_gameState{GameState::Title};
};
#endif // GAMEVIEW_H
