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

#include "proxyclient.h"
#include "mockauthenticator.h"
#include "loggingcategories.h"
#include "authentication/authenticationreply.h"

MockAuthenticator::MockAuthenticator(QObject *parent) :
    Authenticator(parent)
{

}

QString MockAuthenticator::name() const
{
    return "Mock authenticator";
}

void MockAuthenticator::setTimeoutDuration(int timeout)
{
    m_timeoutDuration = timeout;
}

void MockAuthenticator::setExpectedAuthenticationError(Authenticator::AuthenticationError error)
{
    m_expectedError = error;
}

void MockAuthenticator::replyFinished()
{
    MockAuthenticationReply *reply = static_cast<MockAuthenticationReply *>(sender());

    qCDebug(dcAuthentication()) << name() << "Authentication finished" << reply << reply->authenticationReply();
    setReplyError(reply->authenticationReply(), reply->error());
    setReplyFinished(reply->authenticationReply());
    reply->deleteLater();
}

AuthenticationReply *MockAuthenticator::authenticate(ProxyClient *proxyClient)
{
    qCDebug(dcAuthentication()) << name() << "Start authentication for" << proxyClient << "using token" << proxyClient->token() << "Auth duration" << m_timeoutDuration << "[ms]";

    AuthenticationReply *authenticationReply = createAuthenticationReply(proxyClient, this);
    MockAuthenticationReply *reply = new MockAuthenticationReply(m_timeoutDuration, m_expectedError, authenticationReply, this);
    connect(reply, &MockAuthenticationReply::finished, this, &MockAuthenticator::replyFinished);

    return authenticationReply;
}

MockAuthenticationReply::MockAuthenticationReply(int timeout, Authenticator::AuthenticationError error, AuthenticationReply *authenticationReply, QObject *parent) :
    QObject(parent),
    m_error(error),
    m_authenticationReply(authenticationReply)

{
    QTimer::singleShot(timeout, this, &MockAuthenticationReply::finished);
}

MockAuthenticationReply::~MockAuthenticationReply()
{
    qCCritical(dcAuthentication()) << "Destroy mock authentication reply";
}

