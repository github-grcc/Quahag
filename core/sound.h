#ifndef SOUND_H
#define SOUND_H

#include <QDir>
#include <QFile>
#include <QHash>
#include <QProcess>
#include <QTemporaryFile>
#include <QString>

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
    if (it != tempPaths.constEnd())
        QProcess::startDetached("paplay", {it.value()});
}

#endif // SOUND_H
