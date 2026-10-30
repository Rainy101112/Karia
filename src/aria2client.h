#pragma once

#include <QJsonArray>
#include <QJsonValue>
#include <QList>
#include <QObject>
#include <QString>
#include <QUrl>

#include <functional>

#include "downloadtask.h"

class QNetworkAccessManager;

// aria2 JSON-RPC 客户端（HTTP）。
class Aria2Client : public QObject
{
    Q_OBJECT

public:
    explicit Aria2Client(QObject *parent = nullptr);

    QUrl rpcUrl() const { return m_rpcUrl; }
    void setRpcUrl(const QUrl &url);
    QString secret() const { return m_secret; }
    void setSecret(const QString &secret);

    void fetchAllTasks();
    void getGlobalStat();

    void addUri(const QStringList &uris, const QString &dir = QString());
    void pause(const QString &gid);
    void unpause(const QString &gid);
    void remove(const QString &gid);
    void removeDownloadResult(const QString &gid);

Q_SIGNALS:
    void tasksFetched(const QList<DownloadTask> &tasks);
    void globalStatFetched(qint64 downloadSpeed, qint64 uploadSpeed,
                           int active, int waiting, int stopped);
    void operationFinished();
    void errorOccurred(const QString &message);

private:
    using SuccessCallback = std::function<void(const QJsonValue &)>;

    void call(const QString &method, const QJsonArray &params, SuccessCallback onSuccess);
    QJsonArray withSecret(const QJsonArray &params) const;

    QNetworkAccessManager *m_manager;
    QUrl m_rpcUrl;
    QString m_secret;
    int m_nextId = 1;
};
