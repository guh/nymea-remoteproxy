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

#ifndef MOCKAUTHENTICATOR_H
#define MOCKAUTHENTICATOR_H

#include <QTimer>
#include <QObject>

#include "authentication/authenticator.h"

using namespace remoteproxy;

class MockAuthenticationReply : public QObject
{
    Q_OBJECT
public:
    explicit MockAuthenticationReply(int timeout, Authenticator::AuthenticationError error, AuthenticationReply *authenticationReply, QObject *parent = nullptr);
    ~MockAuthenticationReply();

    AuthenticationReply *authenticationReply() const { return m_authenticationReply; }
    Authenticator::AuthenticationError error() const { return m_error; }

private:
    Authenticator::AuthenticationError m_error;
    AuthenticationReply *m_authenticationReply = nullptr;

signals:
    void finished();

};


class MockAuthenticator : public Authenticator
{
    Q_OBJECT
public:
    explicit MockAuthenticator(QObject *parent = nullptr);

    QString name() const override;

    void setTimeoutDuration(int timeout);
    void setExpectedAuthenticationError(Authenticator::AuthenticationError error = AuthenticationErrorNoError);

private:
    int m_timeoutDuration = 500;
    Authenticator::AuthenticationError m_expectedError;

private slots:
    void replyFinished();

public slots:
    AuthenticationReply *authenticate(ProxyClient *proxyClient) override;
};

#endif // MOCKAUTHENTICATOR_H
