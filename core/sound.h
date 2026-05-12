#ifndef SOUND_H
#define SOUND_H

#include <QDir>
#include <QFile>
#include <QHash>
#include <QProcess>
#include <QTemporaryFile>
#include <QString>

#ifdef Q_OS_WIN
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

#ifdef Q_OS_WIN
    PlaySoundW(reinterpret_cast<const wchar_t *>(it.value().utf16()), nullptr,
               SND_ASYNC | SND_FILENAME | SND_NODEFAULT);
#else
    QProcess::startDetached("paplay", {it.value()});
#endif
}

#endif // SOUND_H
