/*
 * Copyright (C) 2026 Chupligin Sergey <neochapay@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "snapbackendworker.h"

#include <QDebug>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLocalSocket>
#include <QTimer>
#include <QUrl>

static QByteArray readHttpResponse(QLocalSocket& socket, int& statusCode)
{
    statusCode = 0;
    QByteArray buffer;
    bool headersDone = false;
    int contentLength = -1;
    bool chunked = false;
    int headerEnd = -1;

    while (socket.state() == QLocalSocket::ConnectedState || socket.bytesAvailable() > 0) {
        if (!socket.waitForReadyRead(5000)) {
            if (socket.state() != QLocalSocket::ConnectedState && socket.bytesAvailable() == 0) {
                break;
            }
            if (socket.error() != QLocalSocket::SocketTimeoutError) {
                break;
            }
            continue;
        }
        buffer.append(socket.readAll());

        if (!headersDone) {
            headerEnd = buffer.indexOf("\r\n\r\n");
            if (headerEnd < 0) {
                continue;
            }
            headersDone = true;
            const QByteArray headerPart = buffer.left(headerEnd);
            const QList<QByteArray> lines = headerPart.split('\n');
            if (!lines.isEmpty()) {
                const QList<QByteArray> statusParts = lines.first().trimmed().split(' ');
                if (statusParts.size() >= 2) {
                    statusCode = statusParts.at(1).toInt();
                }
            }
            for (const QByteArray& line : lines) {
                const QByteArray lower = line.toLower();
                if (lower.startsWith("content-length:")) {
                    contentLength = line.mid(15).trimmed().toInt();
                }
                if (lower.startsWith("transfer-encoding:") && lower.contains("chunked")) {
                    chunked = true;
                }
            }

            if (!chunked && contentLength >= 0) {
                const int bodySize = buffer.size() - (headerEnd + 4);
                if (bodySize >= contentLength) {
                    return buffer.mid(headerEnd + 4, contentLength);
                }
            }
        } else {
            if (!chunked && contentLength >= 0) {
                const int bodySize = buffer.size() - (headerEnd + 4);
                if (bodySize >= contentLength) {
                    return buffer.mid(headerEnd + 4, contentLength);
                }
            }
        }
    }

    if (headerEnd >= 0) {
        QByteArray body = buffer.mid(headerEnd + 4);
        if (chunked) {
            QByteArray decoded;
            int pos = 0;
            while (pos < body.size()) {
                int lineEnd = body.indexOf("\r\n", pos);
                if (lineEnd < 0) {
                    break;
                }
                bool ok = false;
                const int chunkSize = body.mid(pos, lineEnd - pos).trimmed().toInt(&ok, 16);
                if (!ok) {
                    break;
                }
                if (chunkSize == 0) {
                    break;
                }
                const int dataStart = lineEnd + 2;
                if (dataStart + chunkSize > body.size()) {
                    break;
                }
                decoded.append(body.mid(dataStart, chunkSize));
                pos = dataStart + chunkSize + 2;
            }
            return decoded;
        }
        if (contentLength >= 0 && body.size() >= contentLength) {
            return body.left(contentLength);
        }
        return body;
    }

    return QByteArray();
}

SnapBackendWorker::SnapBackendWorker(QObject* parent)
    : QObject(parent)
{
}

QJsonObject SnapBackendWorker::snapRequest(const QString& method, const QString& path, const QJsonObject& body, int& statusCode)
{
    statusCode = 0;
    QLocalSocket socket;
    socket.connectToServer(QStringLiteral("/run/snapd.socket"));
    if (!socket.waitForConnected(5000)) {
        qWarning() << "snapd socket connect failed:" << socket.errorString();
        return QJsonObject();
    }

    QByteArray payload;
    if (!body.isEmpty()) {
        payload = QJsonDocument(body).toJson(QJsonDocument::Compact);
    }

    QByteArray request;
    request += method.toUtf8() + " " + path.toUtf8() + " HTTP/1.1\r\n";
    request += "Host: localhost\r\n";
    request += "Accept: application/json\r\n";
    if (!payload.isEmpty()) {
        request += "Content-Type: application/json\r\n";
        request += "Content-Length: " + QByteArray::number(payload.size()) + "\r\n";
    }
    request += "\r\n";
    request += payload;

    if (socket.write(request) == -1) {
        qWarning() << "snapd request write failed:" << socket.errorString();
        return QJsonObject();
    }
    socket.flush();

    const QByteArray responseBody = readHttpResponse(socket, statusCode);
    socket.disconnectFromServer();

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(responseBody, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        if (!responseBody.isEmpty()) {
            qWarning() << "snapd JSON parse error:" << err.errorString() << responseBody.left(200);
        }
        return QJsonObject();
    }
    return doc.object();
}

bool SnapBackendWorker::waitForChange(const QString& changeId, const QString& actionLabel)
{
    emit emitAction(actionLabel);
    for (int i = 0; i < 600; ++i) {
        int statusCode = 0;
        const QJsonObject resp = snapRequest(QStringLiteral("GET"), QStringLiteral("/v2/changes/") + changeId, QJsonObject(), statusCode);
        const QJsonObject result = resp.value(QStringLiteral("result")).toObject();
        const QString status = result.value(QStringLiteral("status")).toString();
        const QJsonArray tasks = result.value(QStringLiteral("tasks")).toArray();

        double progress = 0;
        QString taskSummary;
        if (!tasks.isEmpty()) {
            double done = 0;
            double total = 0;
            for (const QJsonValue& tv : tasks) {
                const QJsonObject task = tv.toObject();
                const QJsonObject prog = task.value(QStringLiteral("progress")).toObject();
                done += prog.value(QStringLiteral("done")).toDouble();
                total += prog.value(QStringLiteral("total")).toDouble();
                if (taskSummary.isEmpty()) {
                    taskSummary = task.value(QStringLiteral("summary")).toString();
                }
            }
            if (total > 0) {
                progress = (done / total) * 100.0;
            }
        }

        emit emitActionProgress(QStringLiteral("working"), taskSummary.isEmpty() ? actionLabel : taskSummary, progress);

        if (status == QLatin1String("Done")) {
            emit emitActionProgress(QStringLiteral("finished"), actionLabel, 100);
            return true;
        }
        if (status == QLatin1String("Error") || status == QLatin1String("Hold")) {
            qWarning() << "snap change failed:" << status << result;
            return false;
        }

        QEventLoop loop;
        QTimer::singleShot(500, &loop, &QEventLoop::quit);
        loop.exec();
    }
    qWarning() << "snap change timed out:" << changeId;
    return false;
}

GlacierPackage SnapBackendWorker::packageFromJson(const QJsonObject& obj, bool installed) const
{
    GlacierPackage pkg;
    pkg.name = obj.value(QStringLiteral("name")).toString();
    pkg.description = obj.value(QStringLiteral("summary")).toString();
    pkg.version = obj.value(QStringLiteral("version")).toString();
    pkg.repo = QStringLiteral("snap");
    pkg.icon = obj.value(QStringLiteral("icon")).toString();

    const QString status = obj.value(QStringLiteral("status")).toString();
    pkg.installed = installed || status == QLatin1String("active") || status == QLatin1String("installed");
    if (pkg.installed) {
        pkg.installedVersion = pkg.version;
    }

    return pkg;
}

void SnapBackendWorker::doSearchPackages(const QString& name)
{
    QVariantList results;
    if (name.trimmed().isEmpty()) {
        emit searchPackagesReady(results);
        return;
    }

    int statusCode = 0;
    const QJsonObject resp = snapRequest(QStringLiteral("GET"), QStringLiteral("/v2/find?q=") + QUrl::toPercentEncoding(name.trimmed()), QJsonObject(), statusCode);
    if (statusCode != 200) {
        emit searchPackagesReady(results);
        return;
    }

    const QJsonArray arr = resp.value(QStringLiteral("result")).toArray();
    for (const QJsonValue& v : arr) {
        const QJsonObject obj = v.toObject();
        GlacierPackage pkg = packageFromJson(obj, false);
        const QString status = obj.value(QStringLiteral("status")).toString();
        pkg.installed = status == QLatin1String("active") || status == QLatin1String("installed");
        if (pkg.installed) {
            pkg.installedVersion = obj.value(QStringLiteral("version")).toString();
        }
        results.append(QVariant::fromValue(pkg));
    }

    emit searchPackagesReady(results);
}

void SnapBackendWorker::doGetUpdates()
{
    QVariantList updates;
    int statusCode = 0;
    const QJsonObject resp = snapRequest(QStringLiteral("GET"), QStringLiteral("/v2/find?select=refresh"), QJsonObject(), statusCode);
    if (statusCode != 200) {
        emit getUpdatesReady(updates);
        return;
    }

    const QJsonArray arr = resp.value(QStringLiteral("result")).toArray();
    for (const QJsonValue& v : arr) {
        const QJsonObject obj = v.toObject();
        GlacierPackage pkg = packageFromJson(obj, true);
        pkg.haveUpdates = true;
        updates.append(QVariant::fromValue(pkg));
    }

    emit getUpdatesReady(updates);
}

void SnapBackendWorker::doGetOrphansPackages()
{
    emit getOrphansPackagesReady(QVariantList());
}

void SnapBackendWorker::doGetPackage(const QString& name)
{
    GlacierPackage result;
    int statusCode = 0;
    const QJsonObject resp = snapRequest(QStringLiteral("GET"), QStringLiteral("/v2/snaps/") + QUrl::toPercentEncoding(name), QJsonObject(), statusCode);
    if (statusCode == 200) {
        const QJsonObject obj = resp.value(QStringLiteral("result")).toObject();
        result = packageFromJson(obj, true);
        result.installed = true;
        if (result.installedVersion.isEmpty()) {
            result.installedVersion = result.version;
        }
        emit getPackageReady(result);
        return;
    }

    const QJsonObject findResp = snapRequest(QStringLiteral("GET"), QStringLiteral("/v2/find?name=") + QUrl::toPercentEncoding(name), QJsonObject(), statusCode);
    if (statusCode == 200) {
        const QJsonArray arr = findResp.value(QStringLiteral("result")).toArray();
        for (const QJsonValue& v : arr) {
            const QJsonObject obj = v.toObject();
            if (obj.value(QStringLiteral("name")).toString() == name) {
                result = packageFromJson(obj, false);
                break;
            }
        }
    }

    emit getPackageReady(result);
}

void SnapBackendWorker::doInstall(const QString& packageName)
{
    int statusCode = 0;
    QJsonObject body;
    body.insert(QStringLiteral("action"), QStringLiteral("install"));
    const QJsonObject resp = snapRequest(QStringLiteral("POST"), QStringLiteral("/v2/snaps/") + QUrl::toPercentEncoding(packageName), body, statusCode);
    if (statusCode == 202) {
        const QString changeId = resp.value(QStringLiteral("change")).toString();
        if (waitForChange(changeId, tr("Installing %1").arg(packageName))) {
            emit dbRefreshed();
        }
        return;
    }
    qWarning() << "snap install failed:" << statusCode << resp;
}

void SnapBackendWorker::doInstallFromLocalFile(const QString& patch)
{
    Q_UNUSED(patch)
    qWarning() << "installFromLocalFile is not supported for snap backend";
}

void SnapBackendWorker::doRemove(const QString& packageName)
{
    int statusCode = 0;
    QJsonObject body;
    body.insert(QStringLiteral("action"), QStringLiteral("remove"));
    const QJsonObject resp = snapRequest(QStringLiteral("POST"), QStringLiteral("/v2/snaps/") + QUrl::toPercentEncoding(packageName), body, statusCode);
    if (statusCode == 202) {
        const QString changeId = resp.value(QStringLiteral("change")).toString();
        if (waitForChange(changeId, tr("Removing %1").arg(packageName))) {
            emit dbRefreshed();
        }
        return;
    }
    qWarning() << "snap remove failed:" << statusCode << resp;
}

void SnapBackendWorker::doUpgrade(const QString& packageName)
{
    if (packageName.isEmpty()) {
        int statusCode = 0;
        QJsonObject body;
        body.insert(QStringLiteral("action"), QStringLiteral("refresh"));
        const QJsonObject resp = snapRequest(QStringLiteral("POST"), QStringLiteral("/v2/snaps"), body, statusCode);
        if (statusCode == 202) {
            const QString changeId = resp.value(QStringLiteral("change")).toString();
            if (waitForChange(changeId, tr("Upgrading all packages"))) {
                emit dbRefreshed();
            }
            return;
        }
        qWarning() << "snap refresh all failed:" << statusCode << resp;
        return;
    }

    int statusCode = 0;
    QJsonObject body;
    body.insert(QStringLiteral("action"), QStringLiteral("refresh"));
    const QJsonObject resp = snapRequest(QStringLiteral("POST"), QStringLiteral("/v2/snaps/") + QUrl::toPercentEncoding(packageName), body, statusCode);
    if (statusCode == 202) {
        const QString changeId = resp.value(QStringLiteral("change")).toString();
        if (waitForChange(changeId, tr("Upgrading %1").arg(packageName))) {
            emit dbRefreshed();
        }
        return;
    }
    qWarning() << "snap refresh failed:" << statusCode << resp;
}

void SnapBackendWorker::doUpdate()
{
    emit dbRefreshed();
}

void SnapBackendWorker::doListRepositories()
{
    emit listRepositoriesReady(QVariantList());
}
