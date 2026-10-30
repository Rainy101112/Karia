#include "aria2client.h"

#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

namespace {

DownloadTask parseTask(const QJsonObject &object)
{
    DownloadTask task;
    task.gid = object.value(QStringLiteral("gid")).toString();
    task.status = object.value(QStringLiteral("status")).toString();
    task.totalBytes = object.value(QStringLiteral("totalLength")).toString().toLongLong();
    task.downloadedBytes = object.value(QStringLiteral("completedLength")).toString().toLongLong();
    task.speedBytesPerSecond = object.value(QStringLiteral("downloadSpeed")).toString().toLongLong();
    task.uploadBytesPerSecond = object.value(QStringLiteral("uploadSpeed")).toString().toLongLong();
    task.connections = object.value(QStringLiteral("connections")).toString().toInt();
    task.dir = object.value(QStringLiteral("dir")).toString();
    task.errorMessage = object.value(QStringLiteral("errorMessage")).toString();

    // 优先使用文件路径，其次使用 URI，最后是 BT 名称。
    const QJsonArray files = object.value(QStringLiteral("files")).toArray();
    if (!files.isEmpty()) {
        const QJsonObject file = files.first().toObject();

        const QString path = file.value(QStringLiteral("path")).toString();
        if (!path.isEmpty())
            task.fileName = QFileInfo(path).fileName();

        if (task.fileName.isEmpty()) {
            const QJsonArray uris = file.value(QStringLiteral("uris")).toArray();
            if (!uris.isEmpty()) {
                const QString uri = uris.first().toObject().value(QStringLiteral("uri")).toString();
                task.fileName = QUrl(uri).fileName();
            }
        }
    }

    if (task.fileName.isEmpty()) {
        const QJsonObject info = object.value(QStringLiteral("bittorrent"))
                                     .toObject()
                                     .value(QStringLiteral("info"))
                                     .toObject();
        task.fileName = info.value(QStringLiteral("name")).toString();
    }

    if (task.fileName.isEmpty())
        task.fileName = task.gid;

    return task;
}

} // namespace

Aria2Client::Aria2Client(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
    , m_rpcUrl(QStringLiteral("http://127.0.0.1:6800/jsonrpc"))
{
}

void Aria2Client::setRpcUrl(const QUrl &url)
{
    m_rpcUrl = url;
}

void Aria2Client::setSecret(const QString &secret)
{
    m_secret = secret;
}

void Aria2Client::call(const QString &method, const QJsonArray &params, SuccessCallback onSuccess)
{
    QJsonObject request;
    request.insert(QStringLiteral("jsonrpc"), QStringLiteral("2.0"));
    request.insert(QStringLiteral("id"), QString::number(m_nextId++));
    request.insert(QStringLiteral("method"), method);
    // system.multicall 的 secret token 需要放进每个子调用的 params，
    // 放在顶层会得到 "The parameter at 0 has wrong type"。
    request.insert(QStringLiteral("params"),
                   method == QLatin1String("system.multicall") ? params : withSecret(params));

    QNetworkRequest networkRequest(m_rpcUrl);
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    networkRequest.setTransferTimeout(5000);

    QNetworkReply *reply = m_manager->post(
        networkRequest, QJsonDocument(request).toJson(QJsonDocument::Compact));

    connect(reply, &QNetworkReply::finished, this, [this, reply, method, onSuccess]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            Q_EMIT errorOccurred(tr("无法连接 aria2（%1）：%2").arg(method, reply->errorString()));
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            Q_EMIT errorOccurred(tr("aria2 响应解析失败：%1").arg(parseError.errorString()));
            return;
        }

        const QJsonObject response = document.object();
        if (response.contains(QStringLiteral("error"))) {
            const QJsonObject error = response.value(QStringLiteral("error")).toObject();
            const int code = error.value(QStringLiteral("code")).toInt();
            const QString message = error.value(QStringLiteral("message")).toString();
            Q_EMIT errorOccurred(tr("aria2 错误 %1：%2").arg(code).arg(message));
            return;
        }

        if (onSuccess)
            onSuccess(response.value(QStringLiteral("result")));
    });
}

QJsonArray Aria2Client::withSecret(const QJsonArray &params) const
{
    if (m_secret.isEmpty())
        return params;

    QJsonArray result;
    result.append(QStringLiteral("token:") + m_secret);
    for (const QJsonValue &value : params)
        result.append(value);
    return result;
}

void Aria2Client::fetchAllTasks()
{
    const auto subCall = [this](const QString &method, const QJsonArray &params) {
        return QJsonObject{{QStringLiteral("methodName"), method},
                           {QStringLiteral("params"), withSecret(params)}};
    };

    const QJsonArray multicall = {
        subCall(QStringLiteral("aria2.tellActive"), QJsonArray{}),
        subCall(QStringLiteral("aria2.tellWaiting"), QJsonArray{0, 1000}),
        subCall(QStringLiteral("aria2.tellStopped"), QJsonArray{0, 1000}),
    };

    call(QStringLiteral("system.multicall"), QJsonArray{multicall},
         [this](const QJsonValue &result) {
             QList<DownloadTask> tasks;
             const QJsonArray sections = result.toArray();

             for (const QJsonValue &sectionValue : sections) {
                 // system.multicall 会把每个方法的返回值再包一层数组。
                 const QJsonArray section = sectionValue.toArray();
                 if (section.isEmpty())
                     continue;

                 const QJsonArray taskArray = section.first().toArray();
                 for (const QJsonValue &taskValue : taskArray)
                     tasks.append(parseTask(taskValue.toObject()));
             }

             Q_EMIT tasksFetched(tasks);
         });
}

void Aria2Client::getGlobalStat()
{
    call(QStringLiteral("aria2.getGlobalStat"), QJsonArray{},
         [this](const QJsonValue &result) {
             const QJsonObject object = result.toObject();
             Q_EMIT globalStatFetched(
                 object.value(QStringLiteral("downloadSpeed")).toString().toLongLong(),
                 object.value(QStringLiteral("uploadSpeed")).toString().toLongLong(),
                 object.value(QStringLiteral("numActive")).toString().toInt(),
                 object.value(QStringLiteral("numWaiting")).toString().toInt(),
                 object.value(QStringLiteral("numStopped")).toString().toInt());
         });
}

void Aria2Client::addUri(const QStringList &uris, const QString &dir)
{
    QJsonArray uriArray;
    for (const QString &uri : uris)
        uriArray.append(uri);

    QJsonArray params;
    params.append(uriArray);
    if (!dir.isEmpty())
        params.append(QJsonObject{{QStringLiteral("dir"), dir}});

    call(QStringLiteral("aria2.addUri"), params, [this](const QJsonValue &) {
        Q_EMIT operationFinished();
    });
}

void Aria2Client::pause(const QString &gid)
{
    call(QStringLiteral("aria2.pause"), QJsonArray{gid}, [this](const QJsonValue &) {
        Q_EMIT operationFinished();
    });
}

void Aria2Client::unpause(const QString &gid)
{
    call(QStringLiteral("aria2.unpause"), QJsonArray{gid}, [this](const QJsonValue &) {
        Q_EMIT operationFinished();
    });
}

void Aria2Client::remove(const QString &gid)
{
    call(QStringLiteral("aria2.remove"), QJsonArray{gid}, [this](const QJsonValue &) {
        Q_EMIT operationFinished();
    });
}

void Aria2Client::removeDownloadResult(const QString &gid)
{
    call(QStringLiteral("aria2.removeDownloadResult"), QJsonArray{gid},
         [this](const QJsonValue &) { Q_EMIT operationFinished(); });
}
