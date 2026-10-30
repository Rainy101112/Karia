#pragma once

#include <QStyledItemDelegate>

// 在“进度”列绘制原生 QProgressBar 样式。
class ProgressBarDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ProgressBarDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};
