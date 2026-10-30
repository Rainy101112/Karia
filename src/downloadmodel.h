#pragma once

#include <QAbstractTableModel>
#include <QList>

#include "downloadtask.h"

class DownloadModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        FileNameColumn,
        SizeColumn,
        ProgressColumn,
        SpeedColumn,
        EtaColumn,
        StatusColumn,
        ColumnCount
    };

    enum Role {
        ProgressRole = Qt::UserRole + 1,
        ActiveRole,
        WaitingRole,
        PausedRole,
        CompletedRole,
        ErrorRole,
        GidRole
    };

    explicit DownloadModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int completedCount() const;
    int incompleteCount() const;

    const DownloadTask &taskAt(int row) const;

    // 用 aria2 返回的最新任务列表增量更新模型，尽量保留选中状态。
    void syncTasks(const QList<DownloadTask> &tasks);

Q_SIGNALS:
    void taskCountsChanged();

private:
    QString displayStatus(const DownloadTask &task) const;
    QList<DownloadTask> m_tasks;
};
