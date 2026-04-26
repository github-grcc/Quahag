#pragma once

#include "core/inputstate.h"
#include"core/tickcontext.h"

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
    void setWorldPaused(bool paused);
    void start();
    void stop();

signals:
    void stepped(qreal dt);
    void cameraShakeRequested(CameraShakeEvent cameraShakeEvent);
    void cameraZoomPulseRequested(CameraZoomPulseEvent cameraZoomPulseEvent);
    void cameraZoomPulseStopRequested();

private slots:
    void tick();

private:
    QPointer<GameWorld> m_world;
    const InputState *m_input{nullptr};
    QTimer m_timer;
    QElapsedTimer m_elapsedTimer;
    bool m_zoomPulseActive{false};
    bool m_worldPaused{false};
};
