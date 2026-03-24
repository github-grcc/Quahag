#include "gamescene.h"
#include <QGraphicsRectItem>
namespace {
constexpr qreal kGravity = 900.0;
constexpr qreal kTickIntervalMs = 16.0;
constexpr qreal kSceneWidth = 2000.0;
constexpr qreal kSceneHeight = 900.0;
}
GameScene::GameScene(QObject *parent) : QGraphicsScene(parent) {
    setSceneRect(0.0,0.0,kSceneWidth,kSceneHeight);
    initWorld();
    connect(&m_timer,&QTimer::timeout,this,&GameScene::tick);
    m_timer.start(static_cast<int>(kTickIntervalMs));
}
void GameScene::setInput(const InputState &input){
    m_input=input;
}
void GameScene::tick(){
    if(!m_player)
        return;
    const qreal dt=kTickIntervalMs/1000.0;
    m_player->simulate(dt,m_input,m_platforms,kGravity);
    emit playerMoved();
}
void GameScene::initWorld(){
    m_player=new Player();
    addItem(m_player);
    m_player->setPos(120.0, kSceneHeight - 150.0);
    m_player->setZValue(2.0);

    createPlatform(QPointF(0.0, kSceneHeight - 40.0),
                   QSizeF(kSceneWidth, 40.0));
    createPlatform(QPointF(220.0, kSceneHeight - 220.0), QSizeF(160.0, 24.0));
    createPlatform(QPointF(560.0, kSceneHeight - 320.0), QSizeF(220.0, 24.0));
    createPlatform(QPointF(960.0, kSceneHeight - 160.0), QSizeF(260.0, 24.0));
    createPlatform(QPointF(1380.0, kSceneHeight - 280.0), QSizeF(180.0, 24.0));
    createPlatform(QPointF(1660.0, kSceneHeight - 460.0), QSizeF(240.0, 24.0));

}
void GameScene::createPlatform(const QPointF &pos, const QSizeF &size) {
    auto *platform =addRect(QRectF(pos, size), Qt::NoPen, QBrush(QColor(90, 204, 235)));
    platform->setZValue(1.0);
    m_platforms.append(platform);
}
