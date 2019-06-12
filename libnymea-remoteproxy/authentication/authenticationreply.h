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

#ifndef AUTHENTICATIONREPLY_H
#define AUTHENTICATIONREPLY_H

#include <QUuid>
#include <QTimer>
#include <QObject>
#include <QPointer>
#include <QProcess>
#include <QElapsedTimer>

#include "authenticator.h"

namespace remoteproxy {

class AuthenticationReply : public QObject
{
    Q_OBJECT
public:
    friend class Authenticator;

    QPointer<ProxyClient> proxyClient() const;

    bool isTimedOut() const;
    bool isFinished() const;

    Authenticator::AuthenticationError error() const;

private:
    explicit AuthenticationReply(ProxyClient *proxyClient, QObject *parent = nullptr);
    ~AuthenticationReply();

    QPointer<ProxyClient> m_proxyClient;
    QTimer *m_timer = nullptr;

    bool m_timedOut = false;
    bool m_finished = false;

    Authenticator::AuthenticationError m_error = Authenticator::AuthenticationErrorUnknown;

    void setError(Authenticator::AuthenticationError error);
    void setFinished();

signals:
    void finished();

private slots:
    void onTimeout();

public slots:
    void abort();

};

}

#endif // AUTHENTICATIONREPLY_H
