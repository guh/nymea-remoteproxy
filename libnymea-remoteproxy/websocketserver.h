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

#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include <QUrl>
#include <QUuid>
#include <QObject>
#include <QWebSocket>
#include <QWebSocketServer>
#include <QSslConfiguration>

#include "transportinterface.h"

namespace remoteproxy {

class WebSocketServer : public TransportInterface
{
    Q_OBJECT
public:
    explicit WebSocketServer(bool sslEnabled, const QSslConfiguration &sslConfiguration, QObject *parent = nullptr);
    ~WebSocketServer() override;

    bool running() const override;

    QSslConfiguration sslConfiguration() const;

    void sendData(const QUuid &clientId, const QByteArray &data) override;
    void killClientConnection(const QUuid &clientId, const QString &killReason) override;

private:
    QWebSocketServer *m_server = nullptr;
    bool m_sslEnabled;
    QSslConfiguration m_sslConfiguration;

    QHash<QUuid, QWebSocket *> m_clientList;

private slots:
    void onClientConnected();
    void onClientDisconnected();
    void onTextMessageReceived(const QString &message);
    void onBinaryMessageReceived(const QByteArray &data);
    void onClientError(QAbstractSocket::SocketError error);
    void onAcceptError(QAbstractSocket::SocketError error);
    void onServerError(QWebSocketProtocol::CloseCode closeCode);

public slots:
    bool startServer() override;
    bool stopServer() override;

};

}

#endif // WEBSOCKETSERVER_H
