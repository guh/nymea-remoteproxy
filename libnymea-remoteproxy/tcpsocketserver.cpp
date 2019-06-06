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

namespace remoteproxy {

TcpSocketServer::TcpSocketServer(bool sslEnabled, const QSslConfiguration &sslConfiguration, QObject *parent) :
    TransportInterface(parent),
    m_sslEnabled(sslEnabled),
    m_sslConfiguration(sslConfiguration)
{

}

quint16 TcpSocketServer::port() const
{
    return m_port;
}

void TcpSocketServer::setPort(quint16 port)
{
    m_port = port;
}

QHostAddress TcpSocketServer::hostAddress() const
{
    return m_hostAddress;
}

void TcpSocketServer::setHostAddress(const QHostAddress &address)
{
    m_hostAddress = address;
}

void TcpSocketServer::sendData(const QUuid &clientId, const QByteArray &data)
{

}

void TcpSocketServer::killClientConnection(const QUuid &clientId, const QString &killReason)
{

}

bool TcpSocketServer::startServer()
{

}

bool TcpSocketServer::stopServer()
{

}

}
