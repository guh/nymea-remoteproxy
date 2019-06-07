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

#ifndef TRANSPORTINTERFACE_H
#define TRANSPORTINTERFACE_H

#include <QUrl>
#include <QObject>
#include <QHostAddress>

namespace remoteproxy {

class TransportInterface : public QObject
{
    Q_OBJECT
public:
    explicit TransportInterface(QObject *parent = nullptr);
    virtual ~TransportInterface() = 0;

    QString serverName() const;

    virtual void sendData(const QUuid &clientId, const QByteArray &data) = 0;
    virtual void killClientConnection(const QUuid &clientId, const QString &killReason) = 0;

    QUrl serverUrl() const;
    void setServerUrl(const QUrl &serverUrl);

    virtual bool running() const = 0;

signals:
    void clientConnected(const QUuid &clientId, const QHostAddress &address);
    void clientDisconnected(const QUuid &clientId);
    void dataAvailable(const QUuid &clientId, const QByteArray &data);

protected:
    QUrl m_serverUrl;
    QString m_serverName;

public slots:
    virtual bool startServer() = 0;
    virtual bool stopServer() = 0;

};

}

#endif // TRANSPORTINTERFACE_H
