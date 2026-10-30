#include "mainwindow.h"

#include "downloadfilterproxymodel.h"
#include "downloadmodel.h"
#include "formatutils.h"
#include "progressbardelegate.h"

#include <QAction>
#include <QApplication>
#include <QFrame>
#include <QHeaderView>
#include <QIcon>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QSplitter>
#include <QStatusBar>
#include <QTableView>
#include <QToolBar>
#include <QTreeWidget>
#include <QUrl>
#include <QVBoxLayout>

#include <KAboutApplicationDialog>
#include <KAboutData>
#include <KActionCollection>
#include <KGuiItem>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardGuiItem>
#include <KToolBar>

MainWindow::MainWindow(QWidget *parent)
    : KXmlGuiWindow(parent)
    , m_model(new DownloadModel(this))
    , m_proxy(new DownloadFilterProxyModel(this))
{
    setWindowTitle(i18n("Karia"));
    setWindowIcon(QIcon::fromTheme(QStringLiteral("folder-download")));

    m_proxy->setSourceModel(m_model);

    setupActions();
    setupCentralWidget();
    populateSampleTasks();

    // 从内嵌资源加载 XMLGUI 布局，生成菜单栏、工具栏与状态栏。
    setupGUI(Default, QStringLiteral(":/kariaui.rc"));

    if (KToolBar *bar = toolBar(QStringLiteral("mainToolBar"))) {
        bar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        bar->setIconSize(QSize(32, 32));
    }

    setupStatusBar();

    connect(m_model, &DownloadModel::taskCountsChanged, this, &MainWindow::updateCategoryCounts);
    connect(m_model, &DownloadModel::taskCountsChanged, this, &MainWindow::updateStatusBar);
    connect(m_model, &QAbstractItemModel::dataChanged, this, &MainWindow::updateActions);

    updateCategoryCounts();
    updateStatusBar();
    updateActions();
}

void MainWindow::setupActions()
{
    const auto createAction = [this](const QString &name, const QString &text, const QString &icon,
                                     const QKeySequence &shortcut) {
        auto *action = new QAction(this);
        action->setText(text);
        action->setIcon(QIcon::fromTheme(icon));
        actionCollection()->addAction(name, action);
        if (!shortcut.isEmpty())
            KActionCollection::setDefaultShortcut(action, shortcut);
        return action;
    };

    QAction *newAction = createAction(QStringLiteral("new_download"), i18n("新建"),
                                      QStringLiteral("list-add"), QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::newDownload);

    m_resumeAction = createAction(QStringLiteral("resume_download"), i18n("继续"),
                                  QStringLiteral("media-playback-start"),
                                  QKeySequence(QStringLiteral("Ctrl+R")));
    connect(m_resumeAction, &QAction::triggered, this, &MainWindow::resumeSelected);

    m_pauseAction = createAction(QStringLiteral("pause_download"), i18n("停止"),
                                 QStringLiteral("media-playback-pause"),
                                 QKeySequence(QStringLiteral("Ctrl+P")));
    connect(m_pauseAction, &QAction::triggered, this, &MainWindow::pauseSelected);

    m_deleteAction = createAction(QStringLiteral("delete_download"), i18n("删除"),
                                  QStringLiteral("edit-delete"), QKeySequence::Delete);
    connect(m_deleteAction, &QAction::triggered, this, &MainWindow::deleteSelected);

    QAction *settingsAction = createAction(QStringLiteral("preferences"), i18n("设置"),
                                           QStringLiteral("settings-configure"),
                                           QKeySequence(QStringLiteral("Ctrl+,")));
    connect(settingsAction, &QAction::triggered, this, &MainWindow::showSettings);

    QAction *aboutAction = createAction(QStringLiteral("about"), i18n("关于"),
                                        QStringLiteral("help-about"), QKeySequence());
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);

    QAction *quitAction = createAction(QStringLiteral("quit"), i18n("退出"),
                                       QStringLiteral("application-exit"), QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, []() { QCoreApplication::quit(); });
}

void MainWindow::setupCentralWidget()
{
    auto *splitter = new QSplitter(Qt::Horizontal, this);

    // ---- 左侧分类栏 ----
    auto *sidebar = new QWidget(splitter);
    auto *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);
    sidebarLayout->setSpacing(0);

    auto *heading = new QLabel(i18n("分类"), sidebar);
    heading->setContentsMargins(8, 8, 8, 4);
    QFont headingFont = heading->font();
    headingFont.setBold(true);
    heading->setFont(headingFont);
    sidebarLayout->addWidget(heading);

    m_categoryTree = new QTreeWidget(sidebar);
    m_categoryTree->setHeaderHidden(true);
    m_categoryTree->setRootIsDecorated(false);
    m_categoryTree->setFrameShape(QFrame::NoFrame);
    m_categoryTree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_categoryTree->setIconSize(QSize(22, 22));

    const auto addCategory = [this](const QString &text, const QString &icon, int category) {
        auto *item = new QTreeWidgetItem(m_categoryTree);
        item->setText(0, text);
        item->setIcon(0, QIcon::fromTheme(icon));
        item->setData(0, Qt::UserRole, category);
    };
    addCategory(i18n("所有下载"), QStringLiteral("folder-download"),
                DownloadFilterProxyModel::All);
    addCategory(i18n("已完成"), QStringLiteral("dialog-ok"),
                DownloadFilterProxyModel::Completed);
    addCategory(i18n("未完成"), QStringLiteral("media-playback-start"),
                DownloadFilterProxyModel::Incomplete);

    sidebarLayout->addWidget(m_categoryTree, 1);

    // ---- 右侧任务表格 ----
    m_taskView = new QTableView(splitter);
    m_taskView->setModel(m_proxy);
    m_taskView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_taskView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_taskView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_taskView->setAlternatingRowColors(true);
    m_taskView->setWordWrap(false);
    m_taskView->verticalHeader()->setVisible(false);
    m_taskView->verticalHeader()->setDefaultSectionSize(32);
    m_taskView->horizontalHeader()->setStretchLastSection(false);
    m_taskView->horizontalHeader()->setSectionResizeMode(DownloadModel::FileNameColumn,
                                                        QHeaderView::Stretch);
    m_taskView->horizontalHeader()->setSectionResizeMode(DownloadModel::SizeColumn,
                                                        QHeaderView::ResizeToContents);
    m_taskView->horizontalHeader()->setSectionResizeMode(DownloadModel::ProgressColumn,
                                                        QHeaderView::Fixed);
    m_taskView->horizontalHeader()->resizeSection(DownloadModel::ProgressColumn, 160);
    m_taskView->horizontalHeader()->setSectionResizeMode(DownloadModel::SpeedColumn,
                                                        QHeaderView::ResizeToContents);
    m_taskView->horizontalHeader()->setSectionResizeMode(DownloadModel::EtaColumn,
                                                        QHeaderView::ResizeToContents);
    m_taskView->horizontalHeader()->setSectionResizeMode(DownloadModel::StatusColumn,
                                                        QHeaderView::ResizeToContents);
    m_taskView->setItemDelegateForColumn(DownloadModel::ProgressColumn,
                                         new ProgressBarDelegate(m_taskView));

    splitter->addWidget(sidebar);
    splitter->addWidget(m_taskView);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({220, 800});

    connect(m_categoryTree, &QTreeWidget::currentItemChanged, this,
            &MainWindow::currentCategoryChanged);
    connect(m_taskView->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            &MainWindow::updateActions);

    setCentralWidget(splitter);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel(this);
    m_speedLabel = new QLabel(this);
    m_speedLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->addPermanentWidget(m_speedLabel);
}

void MainWindow::populateSampleTasks()
{
    const auto addTask = [this](const QString &fileName, qint64 total, qint64 downloaded,
                                qint64 speed, const QString &status, bool running,
                                bool completed) {
        DownloadTask task;
        task.fileName = fileName;
        task.totalBytes = total;
        task.downloadedBytes = downloaded;
        task.speedBytesPerSecond = speed;
        task.status = status;
        task.running = running;
        task.completed = completed;
        m_model->addTask(task);
    };

    addTask(QStringLiteral("ubuntu-24.04.1-desktop-amd64.iso"), 6100000000LL, 2562000000LL,
            13200000LL, i18n("下载中"), true, false);
    addTask(QStringLiteral("kde-neon-user-edition-20241031.iso"), 2800000000LL, 2184000000LL,
            8500000LL, i18n("下载中"), true, false);
    addTask(QStringLiteral("aria2-1.37.0.tar.xz"), 1450000LL, 1450000LL, 0, i18n("已完成"),
            false, true);
    addTask(QStringLiteral("qt-everywhere-src-6.8.0.tar.xz"), 1100000000LL, 165000000LL, 0,
            i18n("已暂停"), false, false);
}

void MainWindow::updateActions()
{
    const QModelIndex index = m_taskView->currentIndex();
    const bool hasSelection = index.isValid();
    const bool completed = index.data(DownloadModel::CompletedRole).toBool();
    const bool running = index.data(DownloadModel::RunningRole).toBool();

    m_resumeAction->setEnabled(hasSelection && !completed && !running);
    m_pauseAction->setEnabled(hasSelection && !completed && running);
    m_deleteAction->setEnabled(hasSelection);
}

void MainWindow::updateCategoryCounts()
{
    const int total = m_model->rowCount();
    const int completed = m_model->completedCount();

    m_categoryTree->topLevelItem(0)->setText(0, i18n("所有下载 (%1)", total));
    m_categoryTree->topLevelItem(1)->setText(0, i18n("已完成 (%1)", completed));
    m_categoryTree->topLevelItem(2)->setText(0, i18n("未完成 (%1)", total - completed));
}

void MainWindow::updateStatusBar()
{
    const int total = m_model->rowCount();
    const int completed = m_model->completedCount();

    m_statusLabel->setText(i18n("共 %1 个任务，已完成 %2 个", total, completed));
    m_speedLabel->setText(i18n("总速度：%1", Format::speed(m_model->totalSpeed())));
}

int MainWindow::selectedSourceRow() const
{
    const QModelIndex index = m_taskView->currentIndex();
    if (!index.isValid())
        return -1;
    return m_proxy->mapToSource(index).row();
}

void MainWindow::newDownload()
{
    bool ok = false;
    const QString url = QInputDialog::getText(this, i18n("新建下载"), i18n("下载链接："),
                                              QLineEdit::Normal, QString(), &ok)
                            .trimmed();
    if (!ok || url.isEmpty())
        return;

    QString fileName = QUrl(url).fileName();
    if (fileName.isEmpty())
        fileName = i18n("未命名任务");

    DownloadTask task;
    task.fileName = fileName;
    task.status = i18n("等待中");
    task.running = true;
    m_model->addTask(task);
}

void MainWindow::resumeSelected()
{
    const int row = selectedSourceRow();
    if (row >= 0)
        m_model->setRunning(row, true);
}

void MainWindow::pauseSelected()
{
    const int row = selectedSourceRow();
    if (row >= 0)
        m_model->setRunning(row, false);
}

void MainWindow::deleteSelected()
{
    const int row = selectedSourceRow();
    if (row < 0)
        return;

    const auto answer = KMessageBox::questionTwoActions(
        this,
        i18n("确定要删除选中的任务吗？"),
        i18n("删除任务"),
        KGuiItem(i18n("删除"), QStringLiteral("edit-delete")),
        KStandardGuiItem::cancel());

    if (answer == KMessageBox::PrimaryAction)
        m_model->removeTask(row);
}

void MainWindow::showSettings()
{
    KMessageBox::information(
        this,
        i18n("设置功能尚未实现。\n后续可在这里配置 aria2 RPC 地址、下载目录、最大并发数等。"),
        i18n("设置"));
}

void MainWindow::showAbout()
{
    KAboutApplicationDialog dialog(KAboutData::applicationData(), this);
    dialog.exec();
}

void MainWindow::currentCategoryChanged()
{
    QTreeWidgetItem *item = m_categoryTree->currentItem();
    if (!item)
        return;

    const auto category = static_cast<DownloadFilterProxyModel::Category>(
        item->data(0, Qt::UserRole).toInt());
    m_proxy->setCategory(category);
    updateActions();
}
