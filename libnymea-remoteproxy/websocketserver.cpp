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

#include "websocketserver.h"
#include "loggingcategories.h"

#include <QCoreApplication>

namespace remoteproxy {

WebSocketServer::WebSocketServer(bool sslEnabled, const QSslConfiguration &sslConfiguration, QObject *parent) :
    TransportInterface(parent),
    m_sslEnabled(sslEnabled),
    m_sslConfiguration(sslConfiguration)
{
    m_serverName = "WebSocket";
}

WebSocketServer::~WebSocketServer()
{
    stopServer();
}

bool WebSocketServer::running() const
{
    if (!m_server)
        return false;

    return m_server->isListening();
}

QSslConfiguration WebSocketServer::sslConfiguration() const
{
    return m_sslConfiguration;
}

void WebSocketServer::sendData(const QUuid &clientId, const QByteArray &data)
{
    QWebSocket *client = nullptr;
    client = m_clientList.value(clientId);
    if (client) {
        qCDebug(dcWebSocketServerTraffic()) << "--> Sending data to client:" << data;
        client->sendTextMessage(data + '\n');
    } else {
        qCWarning(dcWebSocketServer()) << "Client" << clientId << "unknown to this transport";
    }
}

void WebSocketServer::killClientConnection(const QUuid &clientId, const QString &killReason)
{
    QWebSocket *client = m_clientList.value(clientId);
    if (!client)
        return;

    qCWarning(dcWebSocketServer()) << "Killing client connection" << clientId.toString() << "Reason:" << killReason;
    client->close(QWebSocketProtocol::CloseCodeBadOperation, killReason);
}

void WebSocketServer::onClientConnected()
{
    // Got a new client connected
    QWebSocket *client = m_server->nextPendingConnection();
    if (!client) {
        qCWarning(dcWebSocketServer()) << "Next pending connection dissapeared. Doing nothing.";
        return;
    }

    // Check websocket version
    if (client->version() != QWebSocketProtocol::Version13) {
        qCWarning(dcWebSocketServer()) << "Client with invalid protocol version" << client->version() << ". Rejecting.";
        client->close(QWebSocketProtocol::CloseCodeProtocolError, QString("invalid protocol version: %1 != Supported Version 13").arg(client->version()));
        client->deleteLater();
        return;
    }

    // Create new uuid for this connection
    QUuid clientId = QUuid::createUuid();
    qCDebug(dcWebSocketServer()) << "New client connected:" << client << client->peerAddress().toString() << clientId.toString();

    // Append the new client to the client list
    m_clientList.insert(clientId, client);

    connect(client, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(onBinaryMessageReceived(QByteArray)));
    connect(client, SIGNAL(textMessageReceived(QString)), this, SLOT(onTextMessageReceived(QString)));
    connect(client, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onClientError(QAbstractSocket::SocketError)));
    connect(client, SIGNAL(disconnected()), this, SLOT(onClientDisconnected()));

    emit clientConnected(clientId, client->peerAddress());
}

void WebSocketServer::onClientDisconnected()
{
    QWebSocket *client = static_cast<QWebSocket *>(sender());
    QUuid clientId = m_clientList.key(client);

    qCDebug(dcWebSocketServer()) << "Client disconnected:" << client << client->peerAddress().toString() << clientId.toString() << client->closeReason();

    // Manually close it in any case
    client->close();

    m_clientList.take(clientId)->deleteLater();
    emit clientDisconnected(clientId);
}

void WebSocketServer::onTextMessageReceived(const QString &message)
{
    QWebSocket *client = static_cast<QWebSocket *>(sender());
    qCDebug(dcWebSocketServerTraffic()) << "Text message from" << client->peerAddress().toString() << ":" << message;
    emit dataAvailable(m_clientList.key(client), message.toUtf8());
}

void WebSocketServer::onBinaryMessageReceived(const QByteArray &data)
{
    QWebSocket *client = static_cast<QWebSocket *>(sender());
    qCWarning(dcWebSocketServerTraffic()) << "<-- Binary message from" << client->peerAddress().toString() << ":" << data;
    // Note: this is not expected, so close this client connection.
    client->close(QWebSocketProtocol::CloseCodeBadOperation, "Binary message not expected.");
}

void WebSocketServer::onClientError(QAbstractSocket::SocketError error)
{
    QWebSocket *client = static_cast<QWebSocket *>(sender());
    qCWarning(dcWebSocketServer()) << "Client error occurred:" << client << client->peerAddress().toString() << error << client->errorString() << "Closing the socket.";

    // Note: on any error which can occure, make sure the socket will be closed in any case
    client->close();
}

void WebSocketServer::onAcceptError(QAbstractSocket::SocketError error)
{
    qCWarning(dcWebSocketServer()) << "Server accept error occurred:" << error << m_server->errorString();
}

void WebSocketServer::onServerError(QWebSocketProtocol::CloseCode closeCode)
{
    qCWarning(dcWebSocketServer()) << "Server error occurred:" << closeCode << m_server->errorString();
}

bool WebSocketServer::startServer()
{
    if (m_sslEnabled) {
        m_server = new QWebSocketServer(QCoreApplication::applicationName(), QWebSocketServer::SecureMode, this);
        m_server->setSslConfiguration(sslConfiguration());
    } else {
        m_server = new QWebSocketServer(QCoreApplication::applicationName(), QWebSocketServer::NonSecureMode, this);
    }

    connect (m_server, &QWebSocketServer::newConnection, this, &WebSocketServer::onClientConnected);
    connect (m_server, &QWebSocketServer::acceptError, this, &WebSocketServer::onAcceptError);
    connect (m_server, &QWebSocketServer::serverError, this, &WebSocketServer::onServerError);

    qCDebug(dcWebSocketServer()) << "Starting server" << m_server->serverName() << serverUrl().toString();
    if (!m_server->listen(QHostAddress(m_serverUrl.host()), static_cast<quint16>(serverUrl().port()))) {
        qCWarning(dcWebSocketServer()) << "Server" << m_server->serverName() << "could not listen on" << serverUrl().toString();
        delete  m_server;
        m_server = nullptr;
        return false;
    }

    qCDebug(dcWebSocketServer()) << "Server started successfully.";
    return true;
}

bool WebSocketServer::stopServer()
{
    // Clean up client connections
    foreach (QWebSocket *client, m_clientList.values()) {
        client->close(QWebSocketProtocol::CloseCodeNormal, "Stop server");
        client->flush();
        client->abort();
    }

    // Delete the server object
    if (!m_server)
        return true;

    qCDebug(dcWebSocketServer()) << "Stop server" << m_server->serverName() << serverUrl().toString();
    m_server->close();
    delete m_server;
    m_server = nullptr;
    return true;
}

}
