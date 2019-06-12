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

#include "jsontypes.h"
#include "loggingcategories.h"
#include "authenticationhandler.h"

#include "engine.h"

namespace remoteproxy {

AuthenticationHandler::AuthenticationHandler(QObject *parent) :
    JsonHandler(parent)
{
    // Methods
    QVariantMap params; QVariantMap returns;

    setDescription("Authenticate", "Authenticate this connection. The returned AuthenticationError informs "
                   "about the result. If the authentication was not successfull, the server will close the "
                   "connection immediatly after sending the error response. The given id should be a unique "
                   "id the other tunnel client can understand. Once the authentication was successfull, you "
                   "can wait for the RemoteProxy.TunnelEstablished notification. If you send any data before "
                   "getting this notification, the server will close the connection. If the tunnel client does "
                   "not show up within 10 seconds, the server will close the connection.");
    params.insert("uuid", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("name", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("token", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("o:nonce", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("Authenticate", params);
    returns.insert("authenticationError", JsonTypes::authenticationErrorRef());
    setReturns("Authenticate", returns);
}

QString AuthenticationHandler::name() const
{
    return "Authentication";
}

JsonReply *AuthenticationHandler::Authenticate(const QVariantMap &params, ProxyClient *proxyClient)
{
    QString uuid = params.value("uuid").toString();
    QString name = params.value("name").toString();
    QString token = params.value("token").toString();
    QString nonce = params.value("nonce").toString();

    qCDebug(dcJsonRpc()) << "Authenticate:" << name << uuid << token << nonce;
    JsonReply *jsonReply = createAsyncReply("Authenticate");

    // Set the token for this proxy client
    proxyClient->setUuid(uuid);
    proxyClient->setName(name);
    proxyClient->setToken(token);
    proxyClient->setNonce(nonce);

    AuthenticationReply *authReply = Engine::instance()->authenticator()->authenticate(proxyClient);
    connect(authReply, &AuthenticationReply::finished, this, &AuthenticationHandler::onAuthenticationFinished);

    m_runningAuthentications.insert(authReply, jsonReply);

    return jsonReply;
}

void AuthenticationHandler::onAuthenticationFinished()
{
    AuthenticationReply *authenticationReply = static_cast<AuthenticationReply *>(sender());
    authenticationReply->deleteLater();

    qCDebug(dcJsonRpc()) << "Authentication reply finished";
    JsonReply *jsonReply = m_runningAuthentications.take(authenticationReply);

    if (authenticationReply->error() != Authenticator::AuthenticationErrorNoError) {
        qCWarning(dcJsonRpc()) << "Authentication error occured" << authenticationReply->error();
        jsonReply->setSuccess(false);
    } else {
        // Successfully authenticated
        jsonReply->setSuccess(true);
    }

    // Set client authenticated if still there
    if (!authenticationReply->proxyClient().isNull()) {
        authenticationReply->proxyClient()->setAuthenticated(authenticationReply->error() == Authenticator::AuthenticationErrorNoError);
        jsonReply->setData(errorToReply(authenticationReply->error()));
    }

    jsonReply->finished();
}

}
