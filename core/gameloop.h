#pragma once

#include "core/inputstate.h"

#include <QElapsedTimer>
#include <QObject>
#include <QPointer>
#include <QTimer>

class GameWorld;

class GameLoop : public QObject
{
    Q_OBJECT
public:
    explicit GameLoop(QObject *parent = nullptr);

    void setWorld(GameWorld *world);
    void setInputState(const InputState *input);
    void start();
    void stop();

signals:
    void stepped(qreal dt);

private slots:
    void tick();

private:
    QPointer<GameWorld> m_world;
    const InputState *m_input{nullptr};
    QTimer m_timer;
    QElapsedTimer m_elapsedTimer;
};
