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

#ifndef TCPSOCKETCONNECTION_H
#define TCPSOCKETCONNECTION_H

#include <QObject>
#include <QTcpSocket>
#include <QSslSocket>
#include <QLoggingCategory>

#include "proxyconnection.h"

Q_DECLARE_LOGGING_CATEGORY(dcRemoteProxyClienTcpSocket)

namespace remoteproxyclient {

class TcpSocketConnection : public ProxyConnection
{
    Q_OBJECT

public:
    explicit TcpSocketConnection(QObject *parent = nullptr);
    ~TcpSocketConnection() override;

    void sendData(const QByteArray &data) override;

    void ignoreSslErrors() override;
    void ignoreSslErrors(const QList<QSslError> &errors) override;

private:
    QSslSocket *m_tcpSocket = nullptr;
    bool m_ssl = false;

private slots:
    void onDisconnected();
    void onEncrypted();
    void onError(QAbstractSocket::SocketError error);
    void onStateChanged(QAbstractSocket::SocketState state);
    void onReadyRead();

public slots:
    void connectServer(const QUrl &serverUrl) override;
    void disconnectServer() override;

};

}

#endif // TCPSOCKETCONNECTION_H
