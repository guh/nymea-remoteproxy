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

#include "proxyjsonrpcclient.h"
#include "proxyconnection.h"

#include <QJsonDocument>

Q_LOGGING_CATEGORY(dcRemoteProxyClientJsonRpc, "RemoteProxyClientJsonRpc")
Q_LOGGING_CATEGORY(dcRemoteProxyClientJsonRpcTraffic, "RemoteProxyClientJsonRpcTraffic")

namespace remoteproxyclient {

JsonRpcClient::JsonRpcClient(ProxyConnection *connection, QObject *parent) :
    QObject(parent),
    m_connection(connection)
{

}

JsonReply *JsonRpcClient::callHello()
{
    JsonReply *reply = new JsonReply(m_commandId, "RemoteProxy", "Hello", QVariantMap(), this);
    qCDebug(dcRemoteProxyClientJsonRpc()) << "Calling" << QString("%1.%2").arg(reply->nameSpace()).arg(reply->method());
    sendRequest(reply->requestMap());
    m_replies.insert(m_commandId, reply);
    return reply;
}

JsonReply *JsonRpcClient::callAuthenticate(const QUuid &clientUuid, const QString &clientName, const QString &token, const QString &nonce)
{
    QVariantMap params;
    params.insert("name", clientName);
    params.insert("uuid", clientUuid.toString());
    params.insert("token", token);
    if (!nonce.isEmpty()) params.insert("nonce", nonce);

    JsonReply *reply = new JsonReply(m_commandId, "Authentication", "Authenticate", params, this);
    qCDebug(dcRemoteProxyClientJsonRpc()) << "Calling" << QString("%1.%2").arg(reply->nameSpace()).arg(reply->method());
    sendRequest(reply->requestMap());
    m_replies.insert(m_commandId, reply);
    return reply;
}

void JsonRpcClient::sendRequest(const QVariantMap &request)
{
    QByteArray data = QJsonDocument::fromVariant(request).toJson(QJsonDocument::Compact);
    qCDebug(dcRemoteProxyClientJsonRpcTraffic()) << "Sending" << data;
    m_connection->sendData(data);
}

void JsonRpcClient::processDataPackage(const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcRemoteProxyClientJsonRpc()) << "Invalid JSON data recived" << error.errorString();
        return;
    }

    QVariantMap dataMap = jsonDoc.toVariant().toMap();

    // check if this is a reply to a request
    int commandId = dataMap.value("id").toInt();
    JsonReply *reply = m_replies.take(commandId);
    if (reply) {
        qCDebug(dcRemoteProxyClientJsonRpcTraffic()) << QString("Got response for %1.%2: %3").arg(reply->nameSpace(), reply->method(), QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Indented)));

        if (dataMap.value("status").toString() == "error") {
            qCWarning(dcRemoteProxyClientJsonRpc()) << "Api error happend" << dataMap.value("error").toString();
            // FIMXME: handle json layer errors
        }

        reply->setResponse(dataMap);
        reply->finished();
        return;
    }

    // check if this is a notification
    if (dataMap.contains("notification")) {
        QStringList notification = dataMap.value("notification").toString().split(".");
        QString nameSpace = notification.first();
        QString notificationName = notification.last();
        QVariantMap notificationParams = dataMap.value("params").toMap();

        qCDebug(dcRemoteProxyClientJsonRpc()) << "Notification received" << nameSpace << notificationName;

        if (nameSpace == "RemoteProxy" && notificationName == "TunnelEstablished") {
            QString clientName = notificationParams.value("name").toString();
            QString clientUuid = notificationParams.value("uuid").toString();
            emit tunnelEstablished(clientName, clientUuid);
        }
    }
}

void JsonRpcClient::processData(const QByteArray &data)
{
    qCDebug(dcRemoteProxyClientJsonRpcTraffic()) << "Received data:" << data;

    // Handle packet fragmentation
    m_dataBuffer.append(data);
    int splitIndex = m_dataBuffer.indexOf("}\n{");
    while (splitIndex > -1) {
        processDataPackage(m_dataBuffer.left(splitIndex + 1));
        m_dataBuffer = m_dataBuffer.right(m_dataBuffer.length() - splitIndex - 2);
        splitIndex = m_dataBuffer.indexOf("}\n{");
    }
    if (m_dataBuffer.trimmed().endsWith("}")) {
        processDataPackage(m_dataBuffer);
        m_dataBuffer.clear();
    }
}

}
