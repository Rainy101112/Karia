#pragma once

#include <KXmlGuiWindow>

class DownloadFilterProxyModel;
class DownloadModel;
class QAction;
class QLabel;
class QTableView;
class QTreeWidget;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private:
    void setupActions();
    void setupCentralWidget();
    void setupStatusBar();
    void populateSampleTasks();
    void updateActions();
    void updateCategoryCounts();
    void updateStatusBar();
    int selectedSourceRow() const;

    void newDownload();
    void resumeSelected();
    void pauseSelected();
    void deleteSelected();
    void showSettings();
    void showAbout();
    void currentCategoryChanged();

    DownloadModel *m_model = nullptr;
    DownloadFilterProxyModel *m_proxy = nullptr;
    QTableView *m_taskView = nullptr;
    QTreeWidget *m_categoryTree = nullptr;
    QLabel *m_statusLabel = nullptr;
    QLabel *m_speedLabel = nullptr;

    QAction *m_resumeAction = nullptr;
    QAction *m_pauseAction = nullptr;
    QAction *m_deleteAction = nullptr;
};
