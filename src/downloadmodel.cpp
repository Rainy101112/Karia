#include "downloadmodel.h"

#include "formatutils.h"

#include <QIcon>

#include <KLocalizedString>

DownloadModel::DownloadModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int DownloadModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_tasks.size();
}

int DownloadModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : ColumnCount;
}

QVariant DownloadModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_tasks.size())
        return {};

    const DownloadTask &task = m_tasks.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case FileNameColumn:
            return task.fileName;
        case SizeColumn:
            return Format::bytes(task.totalBytes);
        case ProgressColumn:
            return QString::number(task.progressPercent()) + QLatin1Char('%');
        case SpeedColumn:
            return task.running ? Format::speed(task.speedBytesPerSecond) : QStringLiteral("—");
        case EtaColumn:
            return task.running ? Format::eta(task.remainingBytes(), task.speedBytesPerSecond)
                                : QStringLiteral("—");
        case StatusColumn:
            return task.status;
        default:
            break;
        }
        break;

    case Qt::DecorationRole:
        if (index.column() == FileNameColumn) {
            if (task.completed)
                return QIcon::fromTheme(QStringLiteral("dialog-ok"));
            if (task.running)
                return QIcon::fromTheme(QStringLiteral("folder-download"));
            return QIcon::fromTheme(QStringLiteral("media-playback-pause"));
        }
        break;

    case Qt::TextAlignmentRole:
        if (index.column() == SizeColumn || index.column() == ProgressColumn
            || index.column() == SpeedColumn || index.column() == EtaColumn) {
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        }
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);

    case ProgressRole:
        return task.progressPercent();
    case RunningRole:
        return task.running;
    case CompletedRole:
        return task.completed;
    default:
        break;
    }

    return {};
}

QVariant DownloadModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return {};

    switch (section) {
    case FileNameColumn:
        return i18n("文件名");
    case SizeColumn:
        return i18n("大小");
    case ProgressColumn:
        return i18n("进度");
    case SpeedColumn:
        return i18n("速度");
    case EtaColumn:
        return i18n("剩余时间");
    case StatusColumn:
        return i18n("状态");
    default:
        return {};
    }
}

int DownloadModel::completedCount() const
{
    int count = 0;
    for (const DownloadTask &task : m_tasks) {
        if (task.completed)
            ++count;
    }
    return count;
}

int DownloadModel::incompleteCount() const
{
    return m_tasks.size() - completedCount();
}

qint64 DownloadModel::totalSpeed() const
{
    qint64 speed = 0;
    for (const DownloadTask &task : m_tasks) {
        if (task.running)
            speed += task.speedBytesPerSecond;
    }
    return speed;
}

void DownloadModel::addTask(const DownloadTask &task)
{
    beginInsertRows(QModelIndex(), m_tasks.size(), m_tasks.size());
    m_tasks.append(task);
    endInsertRows();

    Q_EMIT taskCountsChanged();
}

void DownloadModel::removeTask(int row)
{
    if (row < 0 || row >= m_tasks.size())
        return;

    beginRemoveRows(QModelIndex(), row, row);
    m_tasks.removeAt(row);
    endRemoveRows();

    Q_EMIT taskCountsChanged();
}

void DownloadModel::setRunning(int row, bool running)
{
    if (row < 0 || row >= m_tasks.size())
        return;

    DownloadTask &task = m_tasks[row];
    if (task.completed || task.running == running)
        return;

    task.running = running;
    task.status = running ? i18n("下载中") : i18n("已暂停");
    if (!running)
        task.speedBytesPerSecond = 0;

    Q_EMIT dataChanged(index(row, 0), index(row, ColumnCount - 1),
                       {Qt::DisplayRole, ProgressRole, RunningRole});
    Q_EMIT taskCountsChanged();
}
