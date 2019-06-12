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

#include "proxyconnection.h"
#include "proxyjsonrpcclient.h"
#include "tcpsocketconnection.h"
#include "websocketconnection.h"
#include "remoteproxyconnection.h"

Q_LOGGING_CATEGORY(dcRemoteProxyClientConnection, "RemoteProxyClientConnection")
Q_LOGGING_CATEGORY(dcRemoteProxyClientConnectionTraffic, "RemoteProxyClientConnectionTraffic")

namespace remoteproxyclient {

RemoteProxyConnection::RemoteProxyConnection(const QUuid &clientUuid, const QString &clientName, QObject *parent) :
    QObject(parent),
    m_clientUuid(clientUuid),
    m_clientName(clientName)
{

}

RemoteProxyConnection::RemoteProxyConnection(const QUuid &clientUuid, const QString &clientName, RemoteProxyConnection::ConnectionType connectionType, QObject *parent) :
    QObject(parent),
    m_clientUuid(clientUuid),
    m_clientName(clientName),
    m_connectionType(connectionType)
{

}

RemoteProxyConnection::~RemoteProxyConnection()
{

}

RemoteProxyConnection::State RemoteProxyConnection::state() const
{
    return m_state;
}

QAbstractSocket::SocketError RemoteProxyConnection::error() const
{
    return m_error;
}

void RemoteProxyConnection::ignoreSslErrors()
{
    m_connection->ignoreSslErrors();
}

void RemoteProxyConnection::ignoreSslErrors(const QList<QSslError> &errors)
{
    m_connection->ignoreSslErrors(errors);
}

bool RemoteProxyConnection::isConnected() const
{
    return m_state == StateConnected
            || m_state == StateInitializing
            || m_state == StateReady
            || m_state == StateAuthenticating
            || m_state == StateAuthenticated
            || m_state == StateRemoteConnected;
}

bool RemoteProxyConnection::isAuthenticated() const
{
    return m_state == StateAuthenticated || m_state == StateRemoteConnected;
}

bool RemoteProxyConnection::isRemoteConnected() const
{
    return m_state == StateRemoteConnected;
}

RemoteProxyConnection::ConnectionType RemoteProxyConnection::connectionType() const
{
    return m_connectionType;
}

QUrl RemoteProxyConnection::serverUrl() const
{
    return m_serverUrl;
}

QString RemoteProxyConnection::serverName() const
{
    return m_serverName;
}

QString RemoteProxyConnection::proxyServerName() const
{
    return m_proxyServerName;
}

QString RemoteProxyConnection::proxyServerVersion() const
{
    return m_proxyServerVersion;
}

QString RemoteProxyConnection::proxyServerApiVersion() const
{
    return m_proxyServerApiVersion;
}

QString RemoteProxyConnection::tunnelPartnerName() const
{
    return m_tunnelPartnerName;
}

QString RemoteProxyConnection::tunnelPartnerUuid() const
{
    return m_tunnelPartnerUuid;
}

void RemoteProxyConnection::cleanUp()
{
    if (m_jsonClient) {
        m_jsonClient->deleteLater();
        m_jsonClient = nullptr;
    }

    if (m_connection) {
        m_connection->deleteLater();
        m_connection = nullptr;
    }

    m_serverName = QString();
    m_proxyServerName = QString();
    m_proxyServerVersion = QString();
    m_proxyServerApiVersion = QString();

    setState(StateDisconnected);
}

void RemoteProxyConnection::setState(RemoteProxyConnection::State state)
{
    if (m_state == state)
        return;

    m_state = state;
    qCDebug(dcRemoteProxyClientConnection()) << "State changed" << m_state;
    emit stateChanged(m_state);

    switch (m_state) {
    case StateConnected:
        emit connected();
        break;
    case StateDisconnected:
        emit disconnected();
        break;
    case StateReady:
        emit ready();
        break;
    case StateRemoteConnected:
        emit remoteConnectionEstablished();
        break;
    default:
        break;
    }

}

void RemoteProxyConnection::setError(QAbstractSocket::SocketError error)
{
    if (m_error == error)
        return;

    m_error = error;
    qCDebug(dcRemoteProxyClientConnection()) << "Error occured" << m_error;
    emit errorOccured(m_error);
}

void RemoteProxyConnection::onConnectionChanged(bool isConnected)
{
    if (isConnected) {
        qCDebug(dcRemoteProxyClientConnection()) << "Connected to proxy server.";
        setState(StateConnected);

        setState(StateInitializing);
        JsonReply *reply = m_jsonClient->callHello();
        connect(reply, &JsonReply::finished, this, &RemoteProxyConnection::onHelloFinished);
    } else {
        qCDebug(dcRemoteProxyClientConnection()) << "Disconnected from proxy server.";
        setState(StateDisconnected);
        cleanUp();
    }
}

void RemoteProxyConnection::onConnectionDataAvailable(const QByteArray &data)
{
    switch (m_state) {
    case StateHostLookup:
    case StateConnecting:
    case StateConnected:
    case StateInitializing:
    case StateReady:
    case StateAuthenticating:
    case StateAuthenticated:
        m_jsonClient->processData(data);
        break;
    case StateRemoteConnected:
        // Remote data arrived
        emit dataReady(data);
        break;
    default:
        qCWarning(dcRemoteProxyClientConnection()) << "Unhandled: Data received" << data;
        break;
    }
}

void RemoteProxyConnection::onConnectionSocketError(QAbstractSocket::SocketError error)
{
    setError(error);
}

void RemoteProxyConnection::onConnectionStateChanged(QAbstractSocket::SocketState state)
{
    switch (state) {
    case QAbstractSocket::UnconnectedState:
        setState(StateDisconnected);
        break;
    case QAbstractSocket::HostLookupState:
        setState(StateHostLookup);
        break;
    case QAbstractSocket::ConnectingState:
        setState(StateConnecting);
        break;
    case QAbstractSocket::ConnectedState:
        setState(StateConnected);
        break;
    case QAbstractSocket::ClosingState:
        setState(StateDiconnecting);
        break;
    case QAbstractSocket::BoundState:
    case QAbstractSocket::ListeningState:
        break;
    }
}

void RemoteProxyConnection::onHelloFinished()
{
    JsonReply *reply = static_cast<JsonReply *>(sender());
    reply->deleteLater();

    QVariantMap response = reply->response();
    qCDebug(dcRemoteProxyClientConnectionTraffic()) << "Hello response ready" << reply->commandId() << response;

    if (response.value("status").toString() != "success") {
        qCWarning(dcRemoteProxyClientConnection()) << "Could not get initial information from proxy server.";
        m_connection->disconnectServer();
        return;
    }

    QVariantMap responseParams = response.value("params").toMap();

    // TODO: verify success

    m_serverName = responseParams.value("server").toString();
    m_proxyServerName = responseParams.value("name").toString();
    m_proxyServerVersion = responseParams.value("version").toString();
    m_proxyServerApiVersion = responseParams.value("apiVersion").toString();

    setState(StateReady);
}

void RemoteProxyConnection::onAuthenticateFinished()
{
    JsonReply *reply = static_cast<JsonReply *>(sender());
    reply->deleteLater();

    QVariantMap response = reply->response();
    qCDebug(dcRemoteProxyClientConnectionTraffic()) << "Authentication response ready" << reply->commandId() << response;

    QVariantMap responseParams = response.value("params").toMap();
    if (responseParams.value("authenticationError").toString() != "AuthenticationErrorNoError") {
        qCWarning(dcRemoteProxyClientConnection()) << "Authentication request finished with error" << responseParams.value("authenticationError").toString();
        setError(QAbstractSocket::ProxyConnectionRefusedError);
        m_connection->disconnectServer();
    } else {
        qCDebug(dcRemoteProxyClientConnection()) << "Successfully authenticated.";
        setState(StateAuthenticated);
        emit authenticated();
    }
}

void RemoteProxyConnection::onTunnelEstablished(const QString &clientName, const QString &clientUuid)
{
    qCDebug(dcRemoteProxyClientConnection()) << "Remote connection established successfully with" << clientName << clientUuid;
    m_tunnelPartnerName = clientName;
    m_tunnelPartnerUuid = clientUuid;
    setState(StateRemoteConnected);
}

bool RemoteProxyConnection::connectServer(const QUrl &url)
{
    m_serverUrl = url;
    m_error = QAbstractSocket::UnknownSocketError;

    cleanUp();

    switch (m_connectionType) {
    case ConnectionTypeWebSocket:
        qCDebug(dcRemoteProxyClientConnection()) << "Creating a web socket connection to" << url.toString();
        m_connection = qobject_cast<ProxyConnection *>(new WebSocketConnection(this));
        break;
    case ConnectionTypeTcpSocket:
        qCDebug(dcRemoteProxyClientConnection()) << "Creating a TCP socket connection to" << url.toString();
        m_connection = qobject_cast<ProxyConnection *>(new TcpSocketConnection(this));
        break;
    }

    connect(m_connection, &ProxyConnection::connectedChanged, this, &RemoteProxyConnection::onConnectionChanged);
    connect(m_connection, &ProxyConnection::dataReceived, this, &RemoteProxyConnection::onConnectionDataAvailable);
    connect(m_connection, &ProxyConnection::errorOccured, this, &RemoteProxyConnection::onConnectionSocketError);
    connect(m_connection, &ProxyConnection::stateChanged, this, &RemoteProxyConnection::onConnectionStateChanged);
    connect(m_connection, &ProxyConnection::sslErrors, this, &RemoteProxyConnection::sslErrors);

    m_jsonClient = new JsonRpcClient(m_connection, this);
    connect(m_jsonClient, &JsonRpcClient::tunnelEstablished, this, &RemoteProxyConnection::onTunnelEstablished);

    qCDebug(dcRemoteProxyClientConnection()) << "Connecting to" << m_serverUrl.toString();
    m_connection->connectServer(m_serverUrl);

    return true;
}

bool RemoteProxyConnection::authenticate(const QString &token, const QString &nonce)
{
    if (m_state != StateReady) {
        qCWarning(dcRemoteProxyClientConnection()) << "Could not authenticate. The connection is not ready";
        return false;
    }

    setState(StateAuthenticating);

    qCDebug(dcRemoteProxyClientConnection()) << "Start authentication using token" << token << nonce;
    JsonReply *reply = m_jsonClient->callAuthenticate(m_clientUuid, m_clientName, token, nonce);
    connect(reply, &JsonReply::finished, this, &RemoteProxyConnection::onAuthenticateFinished);
    return true;
}

void RemoteProxyConnection::disconnectServer()
{
    if (m_connection) {
        qCDebug(dcRemoteProxyClientConnection()) << "Disconnecting from" << m_connection->serverUrl().toString();
        m_connection->disconnectServer();
    }
}

bool RemoteProxyConnection::sendData(const QByteArray &data)
{
    if (!isConnected()) {
        qCWarning(dcRemoteProxyClientConnection()) << "Could not send data. Not connected.";
        return false;
    }

    if (!isRemoteConnected()) {
        qCWarning(dcRemoteProxyClientConnection()) << "Could not send data. The remote client is not connected yet.";
        return false;
    }

    m_connection->sendData(data);
    return true;
}


}
