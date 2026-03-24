#ifndef GAMESCENE_H
#define GAMESCENE_H
#include<QElapsedTimer>
#include<QGraphicsScene>
#include<QTimer>
#include"player.h"
class GameScene:public QGraphicsScene{
    Q_OBJECT
public:
    explicit GameScene(QObject *parent=nullptr);
    void setInput(const InputState &input);
    Player *player()const{return m_player;};
signals:
    void playerMoved();
private slots:
    void tick();

private:
    void initWorld();
    Player *m_player{nullptr};
    void createPlatform(const QPointF &pos, const QSizeF &size);
    QList<QGraphicsItem *> m_platforms;
    InputState m_input;
    QTimer m_timer;
    QElapsedTimer m_frameTimer;
    
};

#endif // GAMESCENE_H
