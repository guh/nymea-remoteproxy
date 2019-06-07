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

#include "loggingcategories.h"

Q_LOGGING_CATEGORY(dcApplication, "Application")
Q_LOGGING_CATEGORY(dcEngine, "Engine")
Q_LOGGING_CATEGORY(dcJsonRpc, "JsonRpc")
Q_LOGGING_CATEGORY(dcTunnel, "Tunnel")
Q_LOGGING_CATEGORY(dcJsonRpcTraffic, "JsonRpcTraffic")
Q_LOGGING_CATEGORY(dcTcpSocketServer, "TcpSocketServer")
Q_LOGGING_CATEGORY(dcTcpSocketServerTraffic, "TcpSocketServerTraffic")
Q_LOGGING_CATEGORY(dcWebSocketServer, "WebSocketServer")
Q_LOGGING_CATEGORY(dcWebSocketServerTraffic, "WebSocketServerTraffic")
Q_LOGGING_CATEGORY(dcAuthentication, "Authentication")
Q_LOGGING_CATEGORY(dcAuthenticationProcess, "AuthenticationProcess")
Q_LOGGING_CATEGORY(dcProxyServer, "ProxyServer")
Q_LOGGING_CATEGORY(dcProxyServerTraffic, "ProxyServerTraffic")
Q_LOGGING_CATEGORY(dcMonitorServer, "MonitorServer")
Q_LOGGING_CATEGORY(dcAwsCredentialsProvider, "AwsCredentialsProvider")
Q_LOGGING_CATEGORY(dcAwsCredentialsProviderTraffic, "AwsCredentialsProviderTraffic")
