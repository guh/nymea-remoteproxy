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

#ifndef REMOTEPROXYCONNECTOR_H
#define REMOTEPROXYCONNECTOR_H

#include <QUuid>
#include <QDebug>
#include <QObject>
#include <QHostInfo>
#include <QWebSocket>
#include <QHostAddress>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(dcRemoteProxyClientConnection)
Q_DECLARE_LOGGING_CATEGORY(dcRemoteProxyClientConnectionTraffic)

namespace remoteproxyclient {

class JsonRpcClient;
class ProxyConnection;

class RemoteProxyConnection : public QObject
{
    Q_OBJECT
public:
    enum ConnectionType {
        ConnectionTypeWebSocket,
        ConnectionTypeTcpSocket
    };
    Q_ENUM(ConnectionType)

    enum State {
        StateHostLookup,
        StateConnecting,
        StateConnected,
        StateInitializing,
        StateReady,
        StateAuthenticating,
        StateAuthenticated,
        StateRemoteConnected,
        StateDiconnecting,
        StateDisconnected,
    };
    Q_ENUM(State)

    explicit RemoteProxyConnection(const QUuid &clientUuid, const QString &clientName, QObject *parent = nullptr);
    RemoteProxyConnection(const QUuid &clientUuid, const QString &clientName, ConnectionType connectionType, QObject *parent = nullptr);
    ~RemoteProxyConnection();

    RemoteProxyConnection::State state() const;
    QAbstractSocket::SocketError error() const;

    void ignoreSslErrors();
    void ignoreSslErrors(const QList<QSslError> &errors);

    bool isConnected() const;
    bool isAuthenticated() const;
    bool isRemoteConnected() const;

    RemoteProxyConnection::ConnectionType connectionType() const;

    QUrl serverUrl() const;

    QString serverName() const;
    QString proxyServerName() const;
    QString proxyServerVersion() const;
    QString proxyServerApiVersion() const;

    QString tunnelPartnerName() const;
    QString tunnelPartnerUuid() const;

private:
    QUuid m_clientUuid;
    QString m_clientName;
    ConnectionType m_connectionType = ConnectionTypeWebSocket;

    QUrl m_serverUrl;

    State m_state = StateDisconnected;
    QAbstractSocket::SocketError m_error = QAbstractSocket::UnknownSocketError;

    bool m_insecureConnection = false;
    bool m_remoteConnected = false;

    JsonRpcClient *m_jsonClient = nullptr;
    ProxyConnection *m_connection = nullptr;

    // Server information
    QString m_serverName;
    QString m_proxyServerName;
    QString m_proxyServerVersion;
    QString m_proxyServerApiVersion;

    // Tunnel
    QString m_tunnelPartnerName;
    QString m_tunnelPartnerUuid;

    void cleanUp();

    void setState(State state);
    void setError(QAbstractSocket::SocketError error);

signals:
    void connected();
    void ready();
    void authenticated();
    void remoteConnectionEstablished();
    void disconnected();

    void stateChanged(RemoteProxyConnection::State state);
    void errorOccured(QAbstractSocket::SocketError error);
    void sslErrors(const QList<QSslError> &errors);

    void dataReady(const QByteArray &data);

private slots:
    void onConnectionChanged(bool isConnected);
    void onConnectionDataAvailable(const QByteArray &data);
    void onConnectionSocketError(QAbstractSocket::SocketError error);
    void onConnectionStateChanged(QAbstractSocket::SocketState state);

    void onHelloFinished();
    void onAuthenticateFinished();
    void onTunnelEstablished(const QString &clientName, const QString &clientUuid);

public slots:
    bool connectServer(const QUrl &url);
    bool authenticate(const QString &token, const QString &nonce = QString());
    void disconnectServer();
    bool sendData(const QByteArray &data);

};

}

Q_DECLARE_METATYPE(remoteproxyclient::RemoteProxyConnection::State);
Q_DECLARE_METATYPE(remoteproxyclient::RemoteProxyConnection::ConnectionType);

#endif // REMOTEPROXYCONNECTOR_H
