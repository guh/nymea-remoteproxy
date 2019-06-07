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

#include "basetest.h"

#include "engine.h"
#include "loggingcategories.h"
#include "remoteproxyconnection.h"

#include <QMetaType>
#include <QSignalSpy>
#include <QWebSocket>
#include <QJsonDocument>
#include <QWebSocketServer>

BaseTest::BaseTest(QObject *parent) :
    QObject(parent)
{

}

void BaseTest::loadConfiguration(const QString &fileName)
{
    qDebug() << "Load test configurations" << fileName;
    m_configuration->loadConfiguration(fileName);
    restartEngine();
}

void BaseTest::cleanUpEngine()
{
    if (Engine::exists()) {
        Engine::instance()->stop();
        Engine::instance()->destroy();
        QVERIFY(!Engine::exists());
    }
}

void BaseTest::restartEngine()
{
    cleanUpEngine();
    startEngine();
}

void BaseTest::startEngine()
{
    if (!Engine::exists()) {
        Engine::instance()->setAuthenticator(m_authenticator);
        Engine::instance()->setDeveloperModeEnabled(true);
        QVERIFY(Engine::exists());
    }
}

void BaseTest::startServer()
{
    startEngine();

    if (!Engine::instance()->running()) {
        QSignalSpy runningSpy(Engine::instance(), &Engine::runningChanged);
        Engine::instance()->setDeveloperModeEnabled(true);
        Engine::instance()->start(m_configuration);
        runningSpy.wait(100);
        QVERIFY(runningSpy.count() == 1);
    }

    QVERIFY(Engine::instance()->running());
    QVERIFY(Engine::instance()->developerMode());
    QVERIFY(Engine::instance()->webSocketServer()->running());
    QVERIFY(Engine::instance()->tcpSocketServer()->running());
    QVERIFY(Engine::instance()->monitorServer()->running());
}

void BaseTest::stopServer()
{
    if (!Engine::instance()->running())
        return;

    Engine::instance()->stop();
    QVERIFY(!Engine::instance()->running());
}

QVariant BaseTest::invokeWebSocketApiCall(const QString &method, const QVariantMap params, bool remainsConnected)
{
    Q_UNUSED(remainsConnected)

    QVariantMap request;
    request.insert("id", m_commandCounter);
    request.insert("method", method);
    request.insert("params", params);
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(request);

    QWebSocket *socket = new QWebSocket("proxy-testclient", QWebSocketProtocol::Version13);
    connect(socket, &QWebSocket::sslErrors, this, &BaseTest::sslErrors);
    QSignalSpy spyConnection(socket, SIGNAL(connected()));
    socket->open(Engine::instance()->webSocketServer()->serverUrl());
    spyConnection.wait();
    if (spyConnection.count() == 0) {
        return QVariant();
    }

    QSignalSpy dataSpy(socket, SIGNAL(textMessageReceived(QString)));
    socket->sendTextMessage(QString(jsonDoc.toJson(QJsonDocument::Compact)));
    dataSpy.wait();

    socket->close();
    socket->deleteLater();

    for (int i = 0; i < dataSpy.count(); i++) {
        // Make sure the response ends with '}\n'
        if (!dataSpy.at(i).last().toByteArray().endsWith("}\n")) {
            qWarning() << "JSON data does not end with \"}\n\"";
            return QVariant();
        }

        // Make sure the response it a valid JSON string
        QJsonParseError error;
        jsonDoc = QJsonDocument::fromJson(dataSpy.at(i).last().toByteArray(), &error);
        if (error.error != QJsonParseError::NoError) {
            qWarning() << "JSON parser error" << error.errorString();
            return QVariant();
        }
        QVariantMap response = jsonDoc.toVariant().toMap();

        // skip notifications
        if (response.contains("notification"))
            continue;

        if (response.value("id").toInt() == m_commandCounter) {
            m_commandCounter++;
            return jsonDoc.toVariant();
        }
    }

    m_commandCounter++;
    return QVariant();
}

QVariant BaseTest::injectWebSocketData(const QByteArray &data)
{
    QWebSocket *socket = new QWebSocket("proxy-testclient", QWebSocketProtocol::Version13);
    connect(socket, &QWebSocket::sslErrors, this, &BaseTest::sslErrors);

    QSignalSpy spyConnection(socket, SIGNAL(connected()));
    socket->open(Engine::instance()->webSocketServer()->serverUrl());
    spyConnection.wait();
    if (spyConnection.count() == 0) {
        return QVariant();
    }

    QSignalSpy spy(socket, SIGNAL(textMessageReceived(QString)));
    socket->sendTextMessage(QString(data));
    spy.wait();

    socket->close();
    socket->deleteLater();

    for (int i = 0; i < spy.count(); i++) {
        // Make sure the response ends with '}\n'
        if (!spy.at(i).last().toByteArray().endsWith("}\n")) {
            qWarning() << "JSON data does not end with \"}\n\"";
            return QVariant();
        }

        // Make sure the response it a valid JSON string
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(spy.at(i).last().toByteArray(), &error);
        if (error.error != QJsonParseError::NoError) {
            qWarning() << "JSON parser error" << error.errorString();
            return QVariant();
        }
        return jsonDoc.toVariant();
    }
    m_commandCounter++;
    return QVariant();
}

void BaseTest::initTestCase()
{
    qRegisterMetaType<RemoteProxyConnection::State>();
    qRegisterMetaType<RemoteProxyConnection::ConnectionType>();

    m_configuration = new ProxyConfiguration(this);
    m_configuration->loadConfiguration(":/test-configuration.conf");

    m_mockAuthenticator = new MockAuthenticator(this);
    m_dummyAuthenticator = new DummyAuthenticator(this);
    //m_awsAuthenticator = new AwsAuthenticator(m_configuration->awsCredentialsUrl(), this);

    m_authenticator = qobject_cast<Authenticator *>(m_mockAuthenticator);

    m_testToken = "eyJraWQiOiJXdnFFT3prVVh5VDlINzFyRUpoNWdxRnkxNFhnR2l3SFAzVEIzUFQ1V3ZrPSIsImFsZyI6IlJT"
                  "MjU2In0.eyJzdWIiOiJmZTJmZDNlNC1hMGJhLTQ1OTUtOWRiZS00ZDkxYjRiMjFlMzUiLCJhdWQiOiI4cmpoZ"
                  "mRsZjlqZjFzdW9rMmpjcmx0ZDZ2IiwiZW1haWxfdmVyaWZpZWQiOnRydWUsImV2ZW50X2lkIjoiN2Y5NTRiNm"
                  "ItOTYyZi0xMWU4LWI0ZjItMzU5NWRiZmRiODVmIiwidG9rZW5fdXNlIjoiaWQiLCJhdXRoX3RpbWUiOjE1MzM"
                  "xOTkxMzgsImlzcyI6Imh0dHBzOlwvXC9jb2duaXRvLWlkcC5ldS13ZXN0LTEuYW1hem9uYXdzLmNvbVwvZXUt"
                  "d2VzdC0xXzZlWDZZam1YciIsImNvZ25pdG86dXNlcm5hbWUiOiIyYzE3YzUwZC0xZDFlLTRhM2UtYmFjOS0zZ"
                  "DI0YjQ1MTFiYWEiLCJleHAiOjE1MzMyMDI3MzgsImlhdCI6MTUzMzE5OTEzOCwiZW1haWwiOiJqZW5raW5zQG"
                  "d1aC5pbyJ9.hMMSvZMx7pMvV70PaUmTZOZgdez5WGX5yagRFPZojBm8jNWZND1lUmi0RFkybeD4HonDiKHxTF"
                  "_psyJoBVndgHbxYBBl3Np4gn0MxECWjvLxYzGxVBBkN24SqNUyAGkr0uFcZKkBecdtJlqNQnZN8Uk49twmODf"
                  "raRaRmGmKmRBAK1qDITpUgP6AWqH9xoJWaoDzt0kwJ3EtPxS7vL1PHqOaN8ggXA8Eq4iTCSfXU1HAXhIWJH9Y"
                  "pQbj58v1vktaAEATdmKmlzmcix-HJK9wWHRSuv3TsNa8DGxvcPOoeTu8Vql7krZ-y7Zu-s2WsgeP4VxyT80VE"
                  "T_xh6pMkOhE6g";

    qCDebug(dcApplication()) << "Init test case.";
    restartEngine();
}

void BaseTest::cleanupTestCase()
{
    qCDebug(dcApplication()) << "Clean up test case.";
    delete m_configuration;
    delete m_mockAuthenticator;
    delete m_dummyAuthenticator;
    //delete m_awsAuthenticator;

    m_authenticator = nullptr;

    cleanUpEngine();
}

