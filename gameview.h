#ifndef GAMEVIEW_H
#define GAMEVIEW_H
#include "gamescene.h"
#include<QGraphicsView>
#include<QPointer>

class GameView : public QGraphicsView{
    Q_OBJECT

public:
    explicit GameView(QWidget *parent = nullptr);
protected:
    void keyPressEvent(QKeyEvent* event)override;
    void keyReleaseEvent(QKeyEvent* event)override;
    void resizeEvent(QResizeEvent *event)override;
private:
    void updateCamera();
    void updateSceneInput();
    QPointer<GameScene> m_scene;
    bool m_moveLeft{false};
    bool m_moveRight{false};
    bool m_jumpRequested{false};

};
#endif // GAMEVIEW_H
