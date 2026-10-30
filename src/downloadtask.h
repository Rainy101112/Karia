#pragma once

#include <QString>

struct DownloadTask {
    QString fileName;
    qint64 totalBytes = 0;
    qint64 downloadedBytes = 0;
    qint64 speedBytesPerSecond = 0;
    QString status;
    bool running = false;
    bool completed = false;

    int progressPercent() const
    {
        if (completed)
            return 100;
        if (totalBytes <= 0)
            return 0;
        return static_cast<int>((downloadedBytes * 100) / totalBytes);
    }

    qint64 remainingBytes() const
    {
        const qint64 remaining = totalBytes - downloadedBytes;
        return remaining > 0 ? remaining : 0;
    }
};
