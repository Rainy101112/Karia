#pragma once

#include <QSortFilterProxyModel>

class DownloadFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    enum Category {
        All = 0,
        Completed = 1,
        Incomplete = 2
    };

    explicit DownloadFilterProxyModel(QObject *parent = nullptr);

    void setCategory(Category category);
    Category category() const { return m_category; }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    Category m_category = All;
};
