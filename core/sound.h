#ifndef SOUND_H
#define SOUND_H

#include <QDir>
#include <QFile>
#include <QHash>
#include <QProcess>
#include <QTemporaryFile>
#include <QString>
#include <QVector>

#ifdef QUAHAG_HAS_MULTIMEDIA
#include <QSoundEffect>
#elif defined(Q_OS_WIN)
#include <thread>
#include <windows.h>
#include <mmsystem.h>
#endif

inline void playQrcSound(const QString &qrcPath)
{
    static QHash<QString, QString> tempPaths;
    static QHash<QString, bool> initFailed;

    if (!initFailed.value(qrcPath, false) && !tempPaths.contains(qrcPath)) {
        QFile qrc(qrcPath);
        if (qrc.open(QIODevice::ReadOnly)) {
            const QByteArray data = qrc.readAll();
            qrc.close();
            QTemporaryFile tmp(QDir::tempPath() + "/quahag_sfx_XXXXXX.wav");
            tmp.setAutoRemove(false);
            if (tmp.open()) {
                tmp.write(data);
                tmp.close();
                tempPaths[qrcPath] = tmp.fileName();
            }
        }
        if (!tempPaths.contains(qrcPath))
            initFailed[qrcPath] = true;
    }

    const auto it = tempPaths.constFind(qrcPath);
    if (it == tempPaths.constEnd())
        return;

#if defined(QUAHAG_HAS_MULTIMEDIA)
    // Round-robin pool for overlapping playback
    static QHash<QString, QVector<QSoundEffect *>> effectPools;
    static QHash<QString, int> effectIndices;

    auto &pool = effectPools[qrcPath];
    int &idx = effectIndices[qrcPath];

    if (pool.isEmpty()) {
        constexpr int kPoolSize = 4;
        for (int i = 0; i < kPoolSize; ++i) {
            auto *e = new QSoundEffect;
            e->setSource(QUrl::fromLocalFile(it.value()));
            e->setVolume(0.5);
            pool.append(e);
        }
    }

    pool[idx]->play();
    idx = (idx + 1) % pool.size();
#elif defined(Q_OS_WIN)
    // Isolate each playback in its own thread so SND_SYNC doesn't block the
    // game loop and multiple sounds can overlap without cutting each other off.
    std::thread([](QString path) {
        PlaySoundW(reinterpret_cast<const wchar_t *>(path.utf16()), nullptr,
                   SND_FILENAME | SND_SYNC | SND_NODEFAULT);
    }, it.value()).detach();
#else
    QProcess::startDetached("paplay", {it.value()});
#endif
}

#endif // SOUND_H
