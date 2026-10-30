#pragma once

#include <QAbstractTableModel>
#include <QVector>

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
        RunningRole,
        CompletedRole
    };

    explicit DownloadModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int completedCount() const;
    int incompleteCount() const;
    qint64 totalSpeed() const;

    void addTask(const DownloadTask &task);
    void removeTask(int row);
    void setRunning(int row, bool running);

Q_SIGNALS:
    void taskCountsChanged();

private:
    QVector<DownloadTask> m_tasks;
};
