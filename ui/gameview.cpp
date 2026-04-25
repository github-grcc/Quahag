#include "ui/gameview.h"

#include "core/tickcontext.h"
#include "entities/player.h"
#include "world/gameworld.h"
#include "world/levelbuilder.h"

#include <QKeyEvent>
#include <QResizeEvent>
#include <QTimer>

GameView::GameView(QWidget *parent)
    : QGraphicsView(parent)
    , m_scene(new GameScene(this))
    , m_loop(this)
{
    auto *world = new GameWorld(this);
    const LevelBuilder builder;
    builder.build(*world);

    m_scene->attachWorld(world);

    TickContext initCtx;
    initCtx.world = world;
    world->step(initCtx);

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

    connect(&m_loop, &GameLoop::stepped, this, &GameView::updateCamera);
    connect(&m_loop,&GameLoop::cameraShakeRequested,this,&GameView::addShake);
    connect(&m_loop,&GameLoop::cameraZoomPulseRequested,this,&GameView::startZoomPulse);
    connect(&m_loop,&GameLoop::cameraZoomPulseStopRequested,this,&GameView::stopZoomPulse);

    m_camera.setSceneBounds(m_scene->sceneRect());
    m_camera.setViewportSize(viewport()->size());
    m_camera.setFollowResponsiveness(15.0);
    m_camera.setFollowDamping(0.1);
    m_camera.setZoomResponsiveness(10.0);
    m_loop.setWorld(world);
    m_loop.setInputState(&m_input);
    m_loop.start();
    QTimer::singleShot(0, this, [this]() { updateCamera(0.0); });
}

GameView::~GameView()
{
}

void GameView::setCameraZoom(qreal zoom)
{
    m_camera.setTargetZoom(zoom);
    applyCameraTransform();
}

void GameView::startCameraZoomPulse(CameraZoomPulseEvent zoomPulseEvent)
{
    m_camera.startZoomPulse(zoomPulseEvent.amplitude, zoomPulseEvent.duration, zoomPulseEvent.cycles, zoomPulseEvent.center, zoomPulseEvent.initialPhase);
}

void GameView::addCameraShake(CameraShakeEvent shakeEvent)
{
    m_camera.addShake(shakeEvent.amplitude, shakeEvent.duration, shakeEvent.frequency);
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
        m_input.moveLeft = true;
        break;
    case Qt::Key_Right:
    case Qt::Key_D:
        m_input.moveRight = true;
        break;
    case Qt::Key_Space:
    case Qt::Key_Up:
    case Qt::Key_W:
        m_input.jump = true;
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
        m_input.moveLeft = false;
        break;
    case Qt::Key_Right:
    case Qt::Key_D:
        m_input.moveRight = false;
        break;
    case Qt::Key_Space:
    case Qt::Key_Up:
    case Qt::Key_W:
        m_input.jump = false;
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
}
void GameView::resizeEvent(QResizeEvent *event){
    QGraphicsView::resizeEvent(event);
    m_camera.setViewportSize(viewport()->size());
    applyCameraTransform();
}
void GameView::wheelEvent(QWheelEvent *event)
{
    event->ignore();     
}
void GameView::applyCameraTransform()
{
    setTransform(m_camera.transform(), false);
}
void GameView::addShake(CameraShakeEvent shakeEvent){
    m_shakeRequested=true;
    m_shakeEvent=shakeEvent;
}
void GameView::startZoomPulse(CameraZoomPulseEvent zoomPulseEvent){
    m_zoomPulseRequested=true;
    m_zoomPulseEvent=zoomPulseEvent;
}
void GameView::stopZoomPulse(){
    m_camera.stopZoomPulse();
    m_camera.setTargetZoom(1.0);
}
void GameView::updateCamera(qreal dt)
{
    if (!m_scene || !m_scene->player() || m_scene->player()->pendingDestroy())
        return;

    m_camera.setSceneBounds(m_scene->sceneRect());
    m_camera.setViewportSize(viewport()->size());
    m_camera.setTargetCenter(m_scene->player()->sceneBoundingRect().center());
    if(m_zoomPulseRequested){
        startCameraZoomPulse(m_zoomPulseEvent);
        m_zoomPulseRequested = false;
    }
    if(m_shakeRequested){
        addCameraShake(m_shakeEvent);
        m_shakeRequested = false;
    }
    m_camera.update(dt);
    applyCameraTransform();
}
