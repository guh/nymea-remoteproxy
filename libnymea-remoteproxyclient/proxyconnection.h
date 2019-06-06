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

#ifndef SOCKETCONNECTOR_H
#define SOCKETCONNECTOR_H

#include <QUrl>
#include <QObject>
#include <QSslError>
#include <QHostAddress>

namespace remoteproxyclient {

class ProxyConnection : public QObject
{
    Q_OBJECT
public:
    explicit ProxyConnection(QObject *parent = nullptr);
    virtual ~ProxyConnection() = 0;

    virtual void sendData(const QByteArray &data) = 0;

    QUrl serverUrl() const;

    virtual void ignoreSslErrors() = 0;
    virtual void ignoreSslErrors(const QList<QSslError> &errors) = 0;

    bool connected();

private:
    bool m_connected = false;
    QUrl m_serverUrl;

protected:
    void setConnected(bool connected);
    void setServerUrl(const QUrl &serverUrl);

signals:
    void connectedChanged(bool connected);
    void dataReceived(const QByteArray &data);

    void stateChanged(QAbstractSocket::SocketState state);
    void errorOccured(QAbstractSocket::SocketError error);

    void sslErrors(const QList<QSslError> &errors);

public slots:
    virtual void connectServer(const QUrl &serverUrl) = 0;
    virtual void disconnectServer() = 0;

};

}

#endif // SOCKETCONNECTOR_H
