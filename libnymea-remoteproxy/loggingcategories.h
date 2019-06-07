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

#ifndef LOGGINGCATEGORIES_H
#define LOGGINGCATEGORIES_H

#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(dcApplication)
Q_DECLARE_LOGGING_CATEGORY(dcEngine)
Q_DECLARE_LOGGING_CATEGORY(dcJsonRpc)
Q_DECLARE_LOGGING_CATEGORY(dcTunnel)
Q_DECLARE_LOGGING_CATEGORY(dcJsonRpcTraffic)
Q_DECLARE_LOGGING_CATEGORY(dcWebSocketServer)
Q_DECLARE_LOGGING_CATEGORY(dcWebSocketServerTraffic)
Q_DECLARE_LOGGING_CATEGORY(dcTcpSocketServer)
Q_DECLARE_LOGGING_CATEGORY(dcTcpSocketServerTraffic)
Q_DECLARE_LOGGING_CATEGORY(dcAuthentication)
Q_DECLARE_LOGGING_CATEGORY(dcAuthenticationProcess)
Q_DECLARE_LOGGING_CATEGORY(dcProxyServer)
Q_DECLARE_LOGGING_CATEGORY(dcProxyServerTraffic)
Q_DECLARE_LOGGING_CATEGORY(dcMonitorServer)
Q_DECLARE_LOGGING_CATEGORY(dcAwsCredentialsProvider)
Q_DECLARE_LOGGING_CATEGORY(dcAwsCredentialsProviderTraffic)

#endif // LOGGINGCATEGORIES_H
