/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                               *
 * Copyright (C) 2018 Simon Stürz <simon.stuerz@guh.io>                          *
 *                                                                               *
 * This file is part of nymea-remoteproxy.                                       *
 *                                                                               *
 * This program is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by          *
 * the Free Software Foundation, either version 3 of the License, or             *
 * (at your option) any later version.                                           *
 *                                                                               *
 * This program is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
 * GNU General Public License for more details.                                  *
 *                                                                               *
 * You should have received a copy of the GNU General Public License             *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.         *
 *                                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef PROXYCLIENT_H
#define PROXYCLIENT_H

#include <QUuid>
#include <QDebug>
#include <QObject>
#include <QTimer>
#include <QHostAddress>

#include "transportinterface.h"

namespace remoteproxy {

class ProxyClient : public QObject
{
    Q_OBJECT

public:
    enum TimerWaitState {
        TimerWaitStateInactive,
        TimerWaitStateAlone
    };
    Q_ENUM(TimerWaitState)

    explicit ProxyClient(TransportInterface *interface, const QUuid &clientId, const QHostAddress &address, QObject *parent = nullptr);

    QUuid clientId() const;
    QHostAddress peerAddress() const;

    uint creationTime() const;
    QString creationTimeString() const;

    bool isAuthenticated() const;
    void setAuthenticated(bool isAuthenticated);

    bool isTunnelConnected() const;
    void setTunnelConnected(bool isTunnelConnected);

    QString userName() const;
    void setUserName(const QString &userName);

    TransportInterface *interface() const;

    // Properties from auth request
    QString uuid() const;
    void setUuid(const QString &uuid);

    QString name() const;
    void setName(const QString &name);

    QString token() const;
    void setToken(const QString &token);

    QString nonce() const;
    void setNonce(const QString &nonce);

    quint64 rxDataCount() const;
    void addRxDataCount(int dataCount);

    quint64 txDataCount() const;
    void addTxDataCount(int dataCount);

    // Actions for this client
    TimerWaitState timerWaitState() const;
    void resetTimer();
    void sendData(const QByteArray &data);
    void killConnection(const QString &reason);

    // Json server methods
    int generateMessageId();
    QList<QByteArray> processData(const QByteArray &data);
    int bufferSize() const;

private:
    TransportInterface *m_interface = nullptr;
    QTimer *m_timer = nullptr;
    TimerWaitState m_timerWaitState = TimerWaitStateInactive;

    QUuid m_clientId;
    QHostAddress m_peerAddress;
    uint m_creationTimeStamp = 0;

    bool m_authenticated = false;
    bool m_tunnelConnected = false;

    QString m_uuid;
    QString m_name;
    QString m_token;
    QString m_nonce;

    QString m_userName;

    // Json data information
    int m_messageId = 0;
    QByteArray m_dataBuffers;
    bool m_bufferSizeViolation = false;

    quint64 m_rxDataCount = 0;
    quint64 m_txDataCount = 0;

signals:
    void authenticated();
    void tunnelConnected();
    void timeoutOccured();

};

QDebug operator<< (QDebug debug, ProxyClient *proxyClient);

}

#endif // PROXYCLIENT_H
