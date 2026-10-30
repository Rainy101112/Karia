#pragma once

#include <QMetaType>
#include <QString>
#include <QtGlobal>

struct DownloadTask {
    QString gid;
    QString fileName;
    QString dir;
    qint64 totalBytes = 0;
    qint64 downloadedBytes = 0;
    qint64 speedBytesPerSecond = 0;
    qint64 uploadBytesPerSecond = 0;
    int connections = 0;
    QString status; // aria2 原始状态：active/waiting/paused/complete/error/removed
    QString errorMessage;

    bool isActive() const { return status == QLatin1String("active"); }
    bool isWaiting() const { return status == QLatin1String("waiting"); }
    bool isPaused() const { return status == QLatin1String("paused"); }
    bool isComplete() const { return status == QLatin1String("complete"); }
    bool isError() const
    {
        return status == QLatin1String("error") || status == QLatin1String("removed");
    }

    bool isRunning() const { return isActive() || isWaiting(); }

    int progressPercent() const
    {
        if (isComplete())
            return 100;
        if (totalBytes <= 0)
            return 0;
        const qint64 percent = (downloadedBytes * 100) / totalBytes;
        if (percent <= 0)
            return 0;
        if (percent >= 100)
            return 100;
        return static_cast<int>(percent);
    }

    qint64 remainingBytes() const
    {
        const qint64 remaining = totalBytes - downloadedBytes;
        return remaining > 0 ? remaining : 0;
    }

    bool operator==(const DownloadTask &other) const
    {
        return gid == other.gid
            && fileName == other.fileName
            && dir == other.dir
            && totalBytes == other.totalBytes
            && downloadedBytes == other.downloadedBytes
            && speedBytesPerSecond == other.speedBytesPerSecond
            && uploadBytesPerSecond == other.uploadBytesPerSecond
            && connections == other.connections
            && status == other.status
            && errorMessage == other.errorMessage;
    }

    bool operator!=(const DownloadTask &other) const { return !(*this == other); }
};

Q_DECLARE_METATYPE(DownloadTask)
