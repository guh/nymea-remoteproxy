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

#ifndef BASETEST_H
#define BASETEST_H

#include <QObject>
#include <QtTest>
#include <QSslKey>
#include <QObject>
#include <QWebSocket>
#include <QHostAddress>
#include <QSslCertificate>
#include <QSslConfiguration>

#include "jsonrpc/jsontypes.h"
#include "mockauthenticator.h"
#include "proxyconfiguration.h"
#include "remoteproxyconnection.h"
#include "authentication/aws/awsauthenticator.h"
#include "authentication/dummy/dummyauthenticator.h"

using namespace remoteproxy;
using namespace remoteproxyclient;

class BaseTest : public QObject
{
    Q_OBJECT
public:
    explicit BaseTest(QObject *parent = nullptr);

protected:
    ProxyConfiguration *m_configuration = nullptr;

    QUrl m_serverUrl = QUrl("wss://127.0.0.1:1212");

    QSslConfiguration m_sslConfiguration;

    Authenticator *m_authenticator = nullptr;
    MockAuthenticator *m_mockAuthenticator = nullptr;
    DummyAuthenticator *m_dummyAuthenticator  = nullptr;
    AwsAuthenticator *m_awsAuthenticator  = nullptr;

    QString m_testToken;

    int m_commandCounter = 0;

    void loadConfiguration(const QString &fileName);
    void setAuthenticator(Authenticator *authenticator);

    void cleanUpEngine();
    void restartEngine();
    void startEngine();
    void startServer();
    void stopServer();

    QVariant invokeWebSocketApiCall(const QString &method, const QVariantMap params = QVariantMap(), bool remainsConnected = true);
    QVariant injectWebSocketData(const QByteArray &data);

protected slots:
    void initTestCase();
    void cleanupTestCase();

public slots:
    inline void sslErrors(const QList<QSslError> &) {
        QWebSocket *socket = static_cast<QWebSocket*>(sender());
        socket->ignoreSslErrors();
    }

    inline void ignoreConnectionSslError(const QList<QSslError> &errors) {
        RemoteProxyConnection *connection = static_cast<RemoteProxyConnection *>(sender());
        connection->ignoreSslErrors(errors);
        connection->ignoreSslErrors();
    }

    inline void verifyError(const QVariant &response, const QString &fieldName, const QString &error)
    {
        QJsonDocument jsonDoc = QJsonDocument::fromVariant(response);
        QVERIFY2(response.toMap().value("status").toString() == QString("success"),
                 QString("\nExpected status: \"success\"\nGot: %2\nFull message: %3")
                 .arg(response.toMap().value("status").toString())
                 .arg(jsonDoc.toJson().data())
                 .toLatin1().data());
        QVERIFY2(response.toMap().value("params").toMap().value(fieldName).toString() == error,
                 QString("\nExpected: %1\nGot: %2\nFull message: %3\n")
                 .arg(error)
                 .arg(response.toMap().value("params").toMap().value(fieldName).toString())
                 .arg(jsonDoc.toJson().data())
                 .toLatin1().data());
    }

    inline void verifyAuthenticationError(const QVariant &response, Authenticator::AuthenticationError error = Authenticator::AuthenticationErrorNoError) {
        verifyError(response, "authenticationError", JsonTypes::authenticationErrorToString(error));
    }

};

#endif // BASETEST_H
