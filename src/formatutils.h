#pragma once

#include <QString>
#include <QtGlobal>

namespace Format {

inline QString bytes(qint64 value)
{
    static const char *const units[] = {"B", "KiB", "MiB", "GiB", "TiB"};

    double amount = static_cast<double>(value);
    int unit = 0;
    while (amount >= 1024.0 && unit < 4) {
        amount /= 1024.0;
        ++unit;
    }

    const int precision = (unit == 0 || amount >= 100.0) ? 0 : 1;
    return QString::number(amount, 'f', precision) + QLatin1Char(' ') + QLatin1String(units[unit]);
}

inline QString speed(qint64 bytesPerSecond)
{
    if (bytesPerSecond <= 0)
        return QStringLiteral("—");
    return bytes(bytesPerSecond) + QStringLiteral("/s");
}

inline QString eta(qint64 remainingBytes, qint64 bytesPerSecond)
{
    if (remainingBytes <= 0 || bytesPerSecond <= 0)
        return QStringLiteral("—");

    const qint64 totalSeconds = remainingBytes / bytesPerSecond;
    const qint64 hours = totalSeconds / 3600;
    const qint64 minutes = (totalSeconds % 3600) / 60;
    const qint64 seconds = totalSeconds % 60;

    if (hours > 0) {
        return QStringLiteral("%1:%2:%3")
            .arg(hours)
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds, 2, 10, QLatin1Char('0'));
    }

    return QStringLiteral("%1:%2")
        .arg(minutes)
        .arg(seconds, 2, 10, QLatin1Char('0'));
}

} // namespace Format
