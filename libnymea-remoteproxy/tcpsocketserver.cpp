/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                               *
 * Copyright (C) 2019 Simon Stürz <simon.stuerz@guh.io>                          *
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

#include "tcpsocketserver.h"
#include "loggingcategories.h"

namespace remoteproxy {

TcpSocketServer::TcpSocketServer(bool sslEnabled, const QSslConfiguration &sslConfiguration, QObject *parent) :
    TransportInterface(parent),
    m_sslEnabled(sslEnabled),
    m_sslConfiguration(sslConfiguration)
{
    m_serverName = "TCP";
}

TcpSocketServer::~TcpSocketServer()
{
    stopServer();
}

void TcpSocketServer::sendData(const QUuid &clientId, const QByteArray &data)
{
    QTcpSocket *client = nullptr;
    client = m_clientList.value(clientId);
    if (!client) {
        qCWarning(dcTcpSocketServer()) << "Client" << clientId << "unknown to this transport";
        return;
    }

    client->write(data + '\n');
}

void TcpSocketServer::killClientConnection(const QUuid &clientId, const QString &killReason)
{
    QTcpSocket *client = m_clientList.value(clientId);
    if (!client)
        return;

    qCWarning(dcTcpSocketServer()) << "Killing client connection" << clientId.toString() << "Reason:" << killReason;
    client->abort();
}

bool TcpSocketServer::running() const
{
    if (!m_server)
        return false;

    return m_server->isListening();
}

void TcpSocketServer::onDataAvailable(QSslSocket *client, const QByteArray &data)
{
    qCDebug(dcTcpSocketServerTraffic()) << "Emitting data available internal.";
    QUuid clientId = m_clientList.key(qobject_cast<QTcpSocket *>(client));
    emit dataAvailable(clientId, data);
}

void TcpSocketServer::onClientConnected(QSslSocket *client)
{
    QUuid clientId = QUuid::createUuid();
    qCDebug(dcTcpSocketServer()) << "New client connected:" << client << client->peerAddress().toString() << clientId.toString();
    m_clientList.insert(clientId, client);
    emit clientConnected(clientId, client->peerAddress());
}

void TcpSocketServer::onClientDisconnected(QSslSocket *client)
{
    QUuid clientId = m_clientList.key(client);
    qCDebug(dcWebSocketServer()) << "Client disconnected:" << client << client->peerAddress().toString() << clientId.toString();
    m_clientList.take(clientId);
    // Note: the SslServer is deleting the socket object
    emit clientDisconnected(clientId);
}

bool TcpSocketServer::startServer()
{
    qCDebug(dcTcpSocketServer()) << "Starting TCP server" << m_serverUrl.toString();
    m_server = new SslServer(m_sslEnabled, m_sslConfiguration);
    if(!m_server->listen(QHostAddress(m_serverUrl.host()), static_cast<quint16>(m_serverUrl.port()))) {
        qCWarning(dcTcpSocketServer()) << "Tcp server error: can not listen on" << m_serverUrl.toString();
        delete m_server;
        m_server = nullptr;
        return false;
    }

    connect(m_server, &SslServer::clientConnected, this, &TcpSocketServer::onClientConnected);
    connect(m_server, SIGNAL(clientDisconnected(QSslSocket *)), SLOT(onClientDisconnected(QSslSocket *)));
    connect(m_server, &SslServer::dataAvailable, this, &TcpSocketServer::onDataAvailable);
    qCDebug(dcTcpSocketServer()) << "Server started successfully.";
    return true;
}

bool TcpSocketServer::stopServer()
{
    // Clean up client connections
    foreach (const QUuid &clientId, m_clientList.keys()) {
        killClientConnection(clientId, "Stop server");
    }

    if (!m_server)
        return true;

    qCDebug(dcTcpSocketServer()) << "Stop server" << m_serverUrl.toString();
    m_server->close();
    delete m_server;
    m_server = nullptr;
    return true;
}

SslServer::SslServer(bool sslEnabled, const QSslConfiguration &config, QObject *parent) :
    QTcpServer(parent),
    m_sslEnabled(sslEnabled),
    m_config(config)

{

}

void SslServer::incomingConnection(qintptr socketDescriptor)
{
    QSslSocket *sslSocket = new QSslSocket(this);

    qCDebug(dcTcpSocketServer()) << "New client connected:" << sslSocket << sslSocket->peerAddress().toString();

    connect(sslSocket, &QSslSocket::encrypted, [this, sslSocket](){ emit clientConnected(sslSocket); });
    connect(sslSocket, &QSslSocket::readyRead, this, &SslServer::onSocketReadyRead);
    connect(sslSocket, &QSslSocket::disconnected, this, &SslServer::onClientDisconnected);

    if (!sslSocket->setSocketDescriptor(socketDescriptor)) {
        qCWarning(dcTcpSocketServer()) << "Failed to set SSL socket descriptor.";
        delete sslSocket;
        return;
    }

    if (m_sslEnabled) {
        sslSocket->setSslConfiguration(m_config);
        sslSocket->startServerEncryption();
    } else {
        emit clientConnected(sslSocket);
    }
}

void SslServer::onClientDisconnected()
{
    QSslSocket *sslSocket = qobject_cast<QSslSocket *>(sender());
    qCDebug(dcTcpSocketServer()) << "Client socket disconnected:" << sslSocket << sslSocket->peerAddress().toString();;
    emit clientDisconnected(sslSocket);
    sslSocket->deleteLater();
}

void SslServer::onSocketReadyRead()
{
    QSslSocket *sslSocket = qobject_cast<QSslSocket *>(sender());
    QByteArray data = sslSocket->readAll();
    qCDebug(dcTcpSocketServerTraffic()) << "Data from socket" << sslSocket->peerAddress().toString() << data;
    emit dataAvailable(sslSocket, data);
}

}
