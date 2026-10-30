#include "progressbardelegate.h"

#include "downloadmodel.h"

#include <QApplication>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionProgressBar>

ProgressBarDelegate::ProgressBarDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void ProgressBarDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    if (index.column() != DownloadModel::ProgressColumn) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    // 先让基类画背景（包含选中高亮），但不画文字。
    QStyleOptionViewItem background = option;
    initStyleOption(&background, index);
    background.text.clear();

    const QWidget *widget = option.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &background, painter, widget);

    QStyleOptionProgressBar progressBar;
    progressBar.rect = option.rect.adjusted(4, 2, -4, -2);
    progressBar.minimum = 0;
    progressBar.maximum = 100;
    progressBar.progress = index.data(DownloadModel::ProgressRole).toInt();
    progressBar.text = QString::number(progressBar.progress) + QLatin1Char('%');
    progressBar.textVisible = true;
    progressBar.textAlignment = Qt::AlignCenter;
    progressBar.state = QStyle::State_Enabled;

    style->drawControl(QStyle::CE_ProgressBar, &progressBar, painter, widget);
}
