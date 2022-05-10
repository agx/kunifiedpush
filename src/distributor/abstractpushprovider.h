/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KUNIFIEDPUSH_ABSTRACTPUSHPROVIDER_H
#define KUNIFIEDPUSH_ABSTRACTPUSHPROVIDER_H

#include <QObject>

class QNetworkAccessManager;
class QSettings;

namespace KUnifiedPush {

class Client;
class Message;

/** Base class for push provider protocol implementations.
 *  Needed to support different push providers as that part of the
 *  protocol is not part of the UnifiedPush spec.
 */
class AbstractPushProvider : public QObject
{
    Q_OBJECT
public:
    explicit AbstractPushProvider(QObject *parent);

    /** Load connection settings.
     *  @param settings can be read on the top level, the correct group is already selected.
     */
    virtual void loadSettings(const QSettings &settings) = 0;

    /** Attempt to establish a connection to the push provider. */
    virtual void connectToProvider() = 0;

    /** Register a new client with the provider. */
    virtual void registerClient(const Client &client) = 0;

    /** Unregister a client from the provider. */
    virtual void unregisterClient(const Client &client) = 0;

Q_SIGNALS:
    /** Inform about a received push notification. */
    void messageReceived(const KUnifiedPush::Message &msg);

    /** Emitted after successful client registration. */
    void clientRegistered(const KUnifiedPush::Client &client);

    /** Emitted after successful client unregistration. */
    void clientUnregistered(const KUnifiedPush::Client &client);

protected:
    QNetworkAccessManager *nam();

private:
    QNetworkAccessManager *m_nam = nullptr;
};

}

#endif // KUNIFIEDPUSH_ABSTRACTPUSHPROVIDER_H
