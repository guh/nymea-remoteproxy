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

#ifndef TCPSOCKETSERVER_H
#define TCPSOCKETSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QSslConfiguration>

#include "transportinterface.h"

namespace remoteproxy {


class SslServer: public QTcpServer
{
    Q_OBJECT
public:
    SslServer(bool sslEnabled, const QSslConfiguration &config, QObject *parent = nullptr):
        QTcpServer(parent),
        m_sslEnabled(sslEnabled),
        m_config(config)
    {

    }

signals:
    void clientConnected(QSslSocket *socket);
    void clientDisconnected(QSslSocket *socket);
    void dataAvailable(QSslSocket *socket, const QByteArray &data);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onClientDisconnected();
    void onSocketReadyRead();

private:
    bool m_sslEnabled = false;
    QSslConfiguration m_config;
};


class TcpSocketServer : public TransportInterface
{
    Q_OBJECT
public:
    explicit TcpSocketServer(bool sslEnabled, const QSslConfiguration &sslConfiguration, QObject *parent = nullptr);
    ~TcpSocketServer() override;

    quint16 port() const;
    void setPort(quint16 port);

    QHostAddress hostAddress() const;
    void setHostAddress(const QHostAddress &address);

    void sendData(const QUuid &clientId, const QByteArray &data) override;
    void killClientConnection(const QUuid &clientId, const QString &killReason) override;

private:
    quint16 m_port;
    QHostAddress m_hostAddress;
    bool m_sslEnabled;
    QSslConfiguration m_sslConfiguration;

    QTcpServer *m_server = nullptr;

public slots:
    bool startServer() override;
    bool stopServer() override;

};

}

#endif // TCPSOCKETSERVER_H
