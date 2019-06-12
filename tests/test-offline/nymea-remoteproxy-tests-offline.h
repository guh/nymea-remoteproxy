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

#ifndef NYMEA_REMOTEPROXY_TESTS_OFFLINE_H
#define NYMEA_REMOTEPROXY_TESTS_OFFLINE_H

#include "basetest.h"

using namespace remoteproxy;
using namespace remoteproxyclient;

class RemoteProxyOfflineTests : public BaseTest
{
    Q_OBJECT
public:
    explicit RemoteProxyOfflineTests(QObject *parent = nullptr);
    ~RemoteProxyOfflineTests() = default;

private slots:
    // Basic stuff
    void startStopServer();
    void dummyAuthenticator();
    void monitorServer();

    void configuration_data();
    void configuration();

    // WebSocket connection
    void serverPortBlocked();
    void websocketBinaryData();
    void websocketPing();

    // WebSocket connection API
    void getIntrospect();
    void getHello();

    void apiBasicCalls_data();
    void apiBasicCalls();

    void authenticate_data();
    void authenticate();

    void authenticateNonce();
    void authenticateSendData();

    // Client lib
    void clientConnectionTcpSocket();
    void clientConnectionWebSocket();
    void remoteConnection();
    void trippleConnection();
    void duplicateUuid();
    void sslConfigurations();

    void inactiveTimeout();
    void jsonRpcTimeout();
    void authenticationReplyTimeout();
    void authenticationReplyConnection();

    // TCP Websocket combinations
    void tcpRemoteConnection();

};

#endif // NYMEA_REMOTEPROXY_TESTS_OFFLINE_H
