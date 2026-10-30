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
            return task.isActive() ? Format::speed(task.speedBytesPerSecond) : QStringLiteral("—");
        case EtaColumn:
            return task.isActive() ? Format::eta(task.remainingBytes(), task.speedBytesPerSecond)
                                   : QStringLiteral("—");
        case StatusColumn:
            return displayStatus(task);
        default:
            break;
        }
        break;

    case Qt::DecorationRole:
        if (index.column() == FileNameColumn) {
            if (task.isComplete())
                return QIcon::fromTheme(QStringLiteral("dialog-ok"));
            if (task.isError())
                return QIcon::fromTheme(QStringLiteral("dialog-error"));
            if (task.isPaused())
                return QIcon::fromTheme(QStringLiteral("media-playback-pause"));
            if (task.isWaiting())
                return QIcon::fromTheme(QStringLiteral("media-playback-start"));
            return QIcon::fromTheme(QStringLiteral("folder-download"));
        }
        break;

    case Qt::ToolTipRole:
        if (!task.errorMessage.isEmpty())
            return task.errorMessage;
        return task.fileName;

    case Qt::TextAlignmentRole:
        if (index.column() == SizeColumn || index.column() == ProgressColumn
            || index.column() == SpeedColumn || index.column() == EtaColumn) {
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        }
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);

    case ProgressRole:
        return task.progressPercent();
    case ActiveRole:
        return task.isActive();
    case WaitingRole:
        return task.isWaiting();
    case PausedRole:
        return task.isPaused();
    case CompletedRole:
        return task.isComplete();
    case ErrorRole:
        return task.isError();
    case GidRole:
        return task.gid;
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
        if (task.isComplete())
            ++count;
    }
    return count;
}

int DownloadModel::incompleteCount() const
{
    return m_tasks.size() - completedCount();
}

const DownloadTask &DownloadModel::taskAt(int row) const
{
    return m_tasks.at(row);
}

void DownloadModel::syncTasks(const QList<DownloadTask> &tasks)
{
    // 删除 aria2 中已经不存在（且不在 stopped 列表中）的任务。
    for (int row = m_tasks.size() - 1; row >= 0; --row) {
        bool found = false;
        for (const DownloadTask &task : tasks) {
            if (task.gid == m_tasks.at(row).gid) {
                found = true;
                break;
            }
        }
        if (!found) {
            beginRemoveRows(QModelIndex(), row, row);
            m_tasks.removeAt(row);
            endRemoveRows();
        }
    }

    // 更新已有任务，追加新任务。
    for (const DownloadTask &task : tasks) {
        int existingRow = -1;
        for (int row = 0; row < m_tasks.size(); ++row) {
            if (m_tasks.at(row).gid == task.gid) {
                existingRow = row;
                break;
            }
        }

        if (existingRow >= 0) {
            if (m_tasks.at(existingRow) != task) {
                m_tasks[existingRow] = task;
                Q_EMIT dataChanged(index(existingRow, 0), index(existingRow, ColumnCount - 1));
            }
        } else {
            beginInsertRows(QModelIndex(), m_tasks.size(), m_tasks.size());
            m_tasks.append(task);
            endInsertRows();
        }
    }

    Q_EMIT taskCountsChanged();
}

QString DownloadModel::displayStatus(const DownloadTask &task) const
{
    if (task.isActive())
        return i18n("下载中");
    if (task.isWaiting())
        return i18n("等待中");
    if (task.isPaused())
        return i18n("已暂停");
    if (task.isComplete())
        return i18n("已完成");
    if (task.status == QLatin1String("removed"))
        return i18n("已移除");
    if (task.isError())
        return i18n("出错");
    return task.status;
}
