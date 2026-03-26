#include "gameview.h"
#include"player.h"
#include <QKeyEvent>
#include <QResizeEvent>
#include <QTimer>

GameView::GameView(QWidget *parent)
    : QGraphicsView(parent),m_scene(new GameScene())
{
    setScene(m_scene);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);//场景的坐标原点固定在视图的左上角
    setTransformationAnchor(QGraphicsView::NoAnchor);
    setResizeAnchor(QGraphicsView::NoAnchor);
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);

    setFrameShape(QFrame::NoFrame);
    setCacheMode(QGraphicsView::CacheNone);
    setBackgroundBrush(QColor(25,28,35));
    setMinimumSize(800,480);

    connect(m_scene,&GameScene::playerMoved,this,&GameView::updateCamera);
    m_camera.setSceneBounds(sceneRect());
    m_camera.setViewportSize(viewport()->size());
    m_camera.setFollowResponsiveness(15.0);
    m_camera.setFollowDamping(0.1);
    m_camera.setZoomResponsiveness(10.0);
    updateSceneInput();
    QTimer::singleShot(0, this, [this]() { updateCamera(0.0); });
}

GameView::~GameView()
{
    delete m_scene;
}

void GameView::setCameraZoom(qreal zoom)
{
    m_camera.setTargetZoom(zoom);
    applyCameraTransform();
}

void GameView::startCameraZoomPulse(qreal amplitude,
                                    qreal duration,
                                    qreal cycles,
                                    qreal center,
                                    qreal initialPhase)
{
    m_camera.startZoomPulse(amplitude, duration, cycles, center, initialPhase);
}

void GameView::addCameraShake(qreal amplitude, qreal duration, qreal frequency)
{
    m_camera.addShake(amplitude, duration, frequency);
}

void GameView::keyPressEvent(QKeyEvent *event){
    if (event->isAutoRepeat()) {
        switch (event->key()) {
        case Qt::Key_Left:
        case Qt::Key_A:
        case Qt::Key_Right:
        case Qt::Key_D:
        case Qt::Key_Up:
        case Qt::Key_W:
        case Qt::Key_Space:
        case Qt::Key_Down:
        case Qt::Key_S:
        
        case Qt::Key_Q:
        case Qt::Key_E:

            event->accept();
            return;
        default:
            QGraphicsView::keyPressEvent(event);
            return;
        }
    }

    switch (event->key()) {
    case Qt::Key_Down:
    case Qt::Key_S:
        event->accept();
        return;
    case Qt::Key_Left:
    case Qt::Key_A:
        m_moveLeft = true;
        break;
    case Qt::Key_Right:
    case Qt::Key_D:
        m_moveRight = true;
        break;
    case Qt::Key_Space:
    case Qt::Key_Up:
    case Qt::Key_W:
        m_jumpRequested = true;
        break;

    case Qt::Key_Q:
        m_zoomPulseRequested = true;
        break;
    case Qt::Key_E:
        m_shakeRequested = true;
        break;
    default:
        QGraphicsView::keyPressEvent(event);
        return;
    }

    updateSceneInput();
}
void GameView::keyReleaseEvent(QKeyEvent *event){
    if (event->isAutoRepeat()) {
        switch (event->key()) {
        case Qt::Key_Left:
        case Qt::Key_A:
        case Qt::Key_Right:
        case Qt::Key_D:
        case Qt::Key_Up:
        case Qt::Key_W:
        case Qt::Key_Space:
        case Qt::Key_Down:
        case Qt::Key_S:

        case Qt::Key_Q:
        case Qt::Key_E:
            event->accept();
            return;
        default:
            QGraphicsView::keyReleaseEvent(event);
            return;
        }
    }

    switch (event->key()) {
    case Qt::Key_Down:
    case Qt::Key_S:
        event->accept();
        return;
    case Qt::Key_Left:
    case Qt::Key_A:
        m_moveLeft = false;
        break;
    case Qt::Key_Right:
    case Qt::Key_D:
        m_moveRight = false;
        break;
    case Qt::Key_Space:
    case Qt::Key_Up:
    case Qt::Key_W:
        m_jumpRequested = false;
        break;

    case Qt::Key_Q:
        m_zoomPulseRequested = false;
        break;
    case Qt::Key_E:
        m_shakeRequested = false;
        break;
    default:
        QGraphicsView::keyReleaseEvent(event);
        return;
    }

    updateSceneInput();
}
void GameView::resizeEvent(QResizeEvent *event){
    QGraphicsView::resizeEvent(event);
    m_camera.setViewportSize(viewport()->size());
    applyCameraTransform();
}
void GameView::updateSceneInput(){
    if (!m_scene)
        return;

    InputState input;
    input.moveLeft = m_moveLeft;
    input.moveRight = m_moveRight;
    input.jump = m_jumpRequested;
    m_scene->setInput(input);
}
void GameView::wheelEvent(QWheelEvent *event)
{
    event->ignore();     
}
void GameView::applyCameraTransform()
{
    setTransform(m_camera.transform(), false);
}

void GameView::updateCamera(qreal dt)
{
    if (!m_scene || !m_scene->player())
        return;

    m_camera.setSceneBounds(sceneRect());
    m_camera.setViewportSize(viewport()->size());
    m_camera.setTargetCenter(m_scene->player()->sceneBoundingRect().center());
    if(m_zoomPulseRequested){
        startCameraZoomPulse(0.5,2.4,2.0,1.5,1.5*M_PI);
        m_zoomPulseRequested = false;
    }
    if(m_shakeRequested){
        addCameraShake(20.0,0.5,20.0);
        m_shakeRequested = false;
    }
    m_camera.update(dt);
    applyCameraTransform();
}
