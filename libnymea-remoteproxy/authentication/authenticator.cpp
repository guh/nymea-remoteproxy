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
#include "authenticator.h"
#include "authenticationreply.h"

namespace remoteproxy {

Authenticator::Authenticator(QObject *parent) :
    QObject(parent)
{

}

Authenticator::~Authenticator()
{

}

void Authenticator::setReplyError(AuthenticationReply *reply, Authenticator::AuthenticationError error)
{
    reply->setError(error);
}

void Authenticator::setReplyFinished(AuthenticationReply *reply)
{
    reply->setFinished();
}

AuthenticationReply *Authenticator::createAuthenticationReply(ProxyClient *proxyClient, QObject *parent)
{
    return new AuthenticationReply(proxyClient, parent);
}

}
