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

#ifndef JSONRPCCLIENT_H
#define JSONRPCCLIENT_H

#include <QUuid>
#include <QObject>
#include <QVariantMap>
#include <QLoggingCategory>

#include "jsonreply.h"
#include "proxyconnection.h"

Q_DECLARE_LOGGING_CATEGORY(dcRemoteProxyClientJsonRpc)
Q_DECLARE_LOGGING_CATEGORY(dcRemoteProxyClientJsonRpcTraffic)

namespace remoteproxyclient {

class JsonRpcClient : public QObject
{
    Q_OBJECT
public:
    explicit JsonRpcClient(ProxyConnection *connection, QObject *parent = nullptr);

    JsonReply *callHello();
    JsonReply *callAuthenticate(const QUuid &clientUuid, const QString &clientName, const QString &token, const QString &nonce);

private:
    ProxyConnection *m_connection = nullptr;

    int m_commandId = 0;
    QByteArray m_dataBuffer;

    QHash<int, JsonReply *> m_replies;

    void sendRequest(const QVariantMap &request);
    void processDataPackage(const QByteArray &data);

signals:
    void tunnelEstablished(const QString clientName, const QString &clientUuid);

public slots:
    void processData(const QByteArray &data);

};

}

#endif // JSONRPCCLIENT_H
