#include "gameview.h"
#include"player.h"
#include <QKeyEvent>
#include <QResizeEvent>

GameView::GameView(QWidget *parent)
    : QGraphicsView(parent),m_scene(new GameScene(this))
{
    setScene(m_scene);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setCacheMode(QGraphicsView::CacheBackground);
    setBackgroundBrush(QColor(25,28,35));
    setMinimumSize(800,480);

    connect(m_scene,&GameScene::playerMoved,this,&GameView::updateCamera);
    updateSceneInput();
    updateCamera();
}
void GameView::keyPressEvent(QKeyEvent *event){
    if (event->isAutoRepeat()) {
        QGraphicsView::keyPressEvent(event);
        return;
    }

    switch (event->key()) {
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
    default:
        QGraphicsView::keyPressEvent(event);
        return;
    }

    updateSceneInput();
}
void GameView::keyReleaseEvent(QKeyEvent *event){
    if (event->isAutoRepeat()) {
        QGraphicsView::keyReleaseEvent(event);
        return;
    }

    switch (event->key()) {
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
    default:
        QGraphicsView::keyReleaseEvent(event);
        return;
    }

    updateSceneInput();
}
void GameView::resizeEvent(QResizeEvent *event){
    QGraphicsView::resizeEvent(event);
    updateCamera();
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
void GameView::updateCamera(){
    if(!m_scene||!m_scene->player())
        return;
    centerOn(m_scene->player());
}
