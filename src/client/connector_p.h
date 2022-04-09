/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KUNIFIEDPUSH_CONNECTOR_P_H
#define KUNIFIEDPUSH_CONNECTOR_P_H

#include "connector.h"

#include <QObject>

class OrgUnifiedpushDistributor1Interface;

namespace KUnifiedPush {
class ConnectorPrivate : public QObject
{
    Q_OBJECT
public:
    explicit ConnectorPrivate(Connector *qq);

    void Message(const QString &token, const QString &message, const QString &messageIdentifier);
    void NewEndpoint(const QString &token, const QString &endpoint);
    void Unregistered(const QString &token);

    QString stateFile() const;
    void loadState();
    void storeState() const;
    void selectDistributor();
    void setDistributor(const QString &distServiceName);

    void setState(Connector::State state);

    Connector *q = nullptr;
    QString m_serviceName;
    QString m_token;
    QString m_endpoint;
    OrgUnifiedpushDistributor1Interface *m_distributor = nullptr;
    Connector::State m_state = Connector::Unregistered;
};
}

#endif