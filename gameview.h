#ifndef GAMEVIEW_H
#define GAMEVIEW_H
#include "camera2d.h"
#include "gamescene.h"
#include<QGraphicsView>
#include<QPointer>

class GameView : public QGraphicsView{
    Q_OBJECT

public:
    explicit GameView(QWidget *parent = nullptr);
    ~GameView() override;
    void setCameraZoom(qreal zoom);
    void startCameraZoomPulse(qreal amplitude, qreal duration, qreal cycles = 0.5);
    void addCameraShake(qreal amplitude, qreal duration, qreal frequency = 28.0);
protected:
    void keyPressEvent(QKeyEvent* event)override;
    void keyReleaseEvent(QKeyEvent* event)override;
    void resizeEvent(QResizeEvent *event)override;
    void wheelEvent(QWheelEvent *event) override;
private:
    void applyCameraTransform();
    void updateCamera(qreal dt = 0.0);
    void updateSceneInput();
    QPointer<GameScene> m_scene;
    bool m_moveLeft{false};
    bool m_moveRight{false};
    bool m_jumpRequested{false};
    Camera2D m_camera;

};
#endif // GAMEVIEW_H
