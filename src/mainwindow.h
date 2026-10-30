#pragma once

#include <KXmlGuiWindow>

#include "downloadtask.h"

class Aria2Client;
class DownloadFilterProxyModel;
class DownloadModel;
class QAction;
class QLabel;
class QTableView;
class QTimer;
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
    void setupBackend();
    void loadSettings();
    void updateActions();
    void updateCategoryCounts();
    void updateStatusBar();
    void updateConnectionStatus();
    int selectedSourceRow() const;

    void newDownload();
    void resumeSelected();
    void pauseSelected();
    void deleteSelected();
    void showSettings();
    void showAbout();
    void currentCategoryChanged();

    void refresh();
    void onTasksFetched(const QList<DownloadTask> &tasks);
    void onGlobalStat(qint64 downloadSpeed, qint64 uploadSpeed, int active, int waiting,
                      int stopped);
    void onBackendError(const QString &message);

    DownloadModel *m_model = nullptr;
    DownloadFilterProxyModel *m_proxy = nullptr;
    Aria2Client *m_client = nullptr;
    QTimer *m_pollTimer = nullptr;

    QTableView *m_taskView = nullptr;
    QTreeWidget *m_categoryTree = nullptr;
    QLabel *m_statusLabel = nullptr;
    QLabel *m_speedLabel = nullptr;
    QLabel *m_connectionLabel = nullptr;

    QAction *m_resumeAction = nullptr;
    QAction *m_pauseAction = nullptr;
    QAction *m_deleteAction = nullptr;

    bool m_connected = false;
    qint64 m_downloadSpeed = 0;
    qint64 m_uploadSpeed = 0;
    QString m_lastError;
};
