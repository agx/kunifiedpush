/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "nextpushprovider.h"
#include "client.h"
#include "logging.h"
#include "message.h"

#include <QHostInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QUrlQuery>

using namespace KUnifiedPush;

NextPushProvider::NextPushProvider(QObject *parent)
    : AbstractPushProvider(parent)
{
    connect(&m_sseStream, &ServerSentEventsStream::messageReceived, this, [this](const SSEMessage &sse) {
        qCDebug(Log) << sse.event << sse.data;
        if (sse.event == "message") {
            QJsonObject msgObj = QJsonDocument::fromJson(sse.data).object();
            Message msg;
            msg.clientRemoteId = msgObj.value(QLatin1String("token")).toString();
            msg.content = QString::fromUtf8(QByteArray::fromBase64(msgObj.value(QLatin1String("message")).toString().toUtf8()));
            Q_EMIT messageReceived(msg);
        }
    });
}

NextPushProvider::~NextPushProvider() = default;

void NextPushProvider::loadSettings(const QSettings &settings)
{
    m_appPassword = settings.value(QStringLiteral("AppPassword"), QString()).toString();
    m_url = QUrl(settings.value(QStringLiteral("Url"), QString()).toString());
    m_userName = settings.value(QStringLiteral("Username"), QString()).toString();
    m_deviceId = settings.value(QStringLiteral("DeviceId"), QString()).toString();
    qCDebug(Log) << m_url << m_userName << m_deviceId;
}

void NextPushProvider::connectToProvider()
{
    qCDebug(Log) << m_deviceId;

    if (m_deviceId.isEmpty()) {
        auto req = prepareRequest("device");
        req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

        QJsonObject content;
        content.insert(QLatin1String("deviceName"), QHostInfo::localHostName());
        auto reply = nam()->put(req, QJsonDocument(content).toJson(QJsonDocument::Compact));
        connect(reply, &QNetworkReply::finished, this, [reply, this]() {
            reply->deleteLater();
            if (reply->error() != QNetworkReply::NoError) {
                qCWarning(Log) << reply->errorString();
                // TODO error state? retry?
                return;
            }

            const auto content = QJsonDocument::fromJson(reply->readAll()).object();
            qCDebug(Log) << QJsonDocument(content).toJson(QJsonDocument::Compact);
            // TODO check "success" field
            m_deviceId = content.value(QLatin1String("deviceId")).toString();

            QSettings settings;
            settings.setValue(QStringLiteral("NextPush/DeviceId"), m_deviceId);
            waitForMessage();
        });
    } else {
        waitForMessage();
    }
}

void NextPushProvider::registerClient(const Client &client)
{
    qCDebug(Log) << client.serviceName;
    auto req = prepareRequest("app");
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    QJsonObject content;
    content.insert(QLatin1String("deviceId"), m_deviceId);
    content.insert(QLatin1String("appName"), client.serviceName);
    auto reply = nam()->put(req, QJsonDocument(content).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [reply, this, client]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qCWarning(Log) << reply->errorString();
        } else {
            const auto content = QJsonDocument::fromJson(reply->readAll()).object();
            qCDebug(Log) << QJsonDocument(content).toJson(QJsonDocument::Compact);
            // TODO check "success" field
            auto newClient = client;
            newClient.remoteId = content.value(QLatin1String("token")).toString();

            QUrl endpointUrl = m_url;
            auto path = endpointUrl.path();
            path += QLatin1String("/index.php/apps/uppush/push/") + newClient.remoteId;
            endpointUrl.setPath(path);
            newClient.endpoint = endpointUrl.toString();

            Q_EMIT clientRegistered(newClient);
        }
    });
}

void NextPushProvider::unregisterClient(const Client &client)
{
    qCDebug(Log) << client.serviceName << client.remoteId;
    auto req = prepareRequest("app", client.remoteId);
    auto reply = nam()->deleteResource(req);
    connect(reply, &QNetworkReply::finished, this, [reply, this, client]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qCWarning(Log) << reply->errorString();
        } else {
            qCDebug(Log) << "application deleted";
            Q_EMIT clientUnregistered(client);
        }
    });
}

void NextPushProvider::waitForMessage()
{
    qCDebug(Log);
    auto req = prepareRequest("device", m_deviceId);
    auto reply = nam()->get(req);
    connect(reply, &QNetworkReply::finished, this, [reply, this]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qCWarning(Log) << reply->errorString();
        } else {
            qCDebug(Log) << "GET finished";
            // TODO restart?
        }
    });
    m_sseStream.read(reply);
}

QNetworkRequest NextPushProvider::prepareRequest(const char *restCmd, const QString &restArg) const
{
    QUrl url = m_url;
    auto path = url.path();
    path += QLatin1String("/index.php/apps/uppush/") + QLatin1String(restCmd) + QLatin1Char('/') + restArg;
    url.setPath(path);

    QNetworkRequest req(url);
    req.setRawHeader("Authorization", "Basic " + QByteArray(m_userName.toUtf8() + ':' + m_appPassword.toUtf8()).toBase64());
    return req;
}
