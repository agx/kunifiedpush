/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KUNIFIEDPUSH_ABSTRACTPUSHPROVIDER_H
#define KUNIFIEDPUSH_ABSTRACTPUSHPROVIDER_H

#include <QObject>

class QSettings;

namespace KUnifiedPush {

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

Q_SIGNALS:
    /** Inform about a received push notification. */
    void messageReceived(const KUnifiedPush::Message &msg);
};

}

#endif // KUNIFIEDPUSH_ABSTRACTPUSHPROVIDER_H