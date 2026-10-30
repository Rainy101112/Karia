#include "downloadfilterproxymodel.h"

#include "downloadmodel.h"

DownloadFilterProxyModel::DownloadFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

void DownloadFilterProxyModel::setCategory(Category category)
{
    if (m_category == category)
        return;

    beginFilterChange();
    m_category = category;
    endFilterChange();
}

bool DownloadFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (m_category == All)
        return true;

    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    const bool completed = index.data(DownloadModel::CompletedRole).toBool();

    return m_category == Completed ? completed : !completed;
}
