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

#include <QSslError>
#include <QMetaType>
#include <QSignalSpy>
#include <QSslSocket>
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
    //restartEngine();
}

void BaseTest::cleanUpEngine()
{
    qDebug() << "Clean up engine";
    if (Engine::exists()) {
        Engine::instance()->stop();
        Engine::instance()->destroy();
        QVERIFY(!Engine::exists());
    }

    if (m_mockAuthenticator) {
        delete m_mockAuthenticator;
        m_mockAuthenticator = nullptr;
    }

    if (m_dummyAuthenticator) {
        delete  m_dummyAuthenticator;
        m_dummyAuthenticator = nullptr;
    }

    m_authenticator = nullptr;

    if (m_configuration) {
        delete  m_configuration;
        m_configuration = nullptr;
    }
}

void BaseTest::restartEngine()
{
    cleanUpEngine();
    startEngine();
}

void BaseTest::startEngine()
{
    m_configuration = new ProxyConfiguration(this);
    loadConfiguration(":/test-configuration.conf");

    m_dummyAuthenticator = new DummyAuthenticator(this);
    m_mockAuthenticator = new MockAuthenticator(this);
    m_authenticator = qobject_cast<Authenticator *>(m_mockAuthenticator);
    if (!Engine::exists()) {
        Engine::instance()->setAuthenticator(m_authenticator);
        Engine::instance()->setDeveloperModeEnabled(true);
        QVERIFY(Engine::exists());
        QVERIFY(!Engine::instance()->running());
    }
}

void BaseTest::startServer()
{
    restartEngine();

    if (!Engine::instance()->running()) {
        QSignalSpy runningSpy(Engine::instance(), &Engine::runningChanged);
        Engine::instance()->setDeveloperModeEnabled(true);
        Engine::instance()->start(m_configuration);
        runningSpy.wait(200);
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

    cleanUpEngine();
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

QVariant BaseTest::invokeTcpSocketApiCall(const QString &method, const QVariantMap params, bool remainsConnected)
{
    Q_UNUSED(remainsConnected)

    QVariantMap request;
    request.insert("id", m_commandCounter);
    request.insert("method", method);
    request.insert("params", params);
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(request);

    QSslSocket *socket = new QSslSocket(this);
    typedef void (QSslSocket:: *sslErrorsSignal)(const QList<QSslError> &);
    QObject::connect(socket, static_cast<sslErrorsSignal>(&QSslSocket::sslErrors), this, &BaseTest::sslSocketSslErrors);

    QSignalSpy spyConnection(socket, &QSslSocket::connected);
    socket->connectToHostEncrypted(Engine::instance()->tcpSocketServer()->serverUrl().host(),
                                   static_cast<quint16>(Engine::instance()->tcpSocketServer()->serverUrl().port()));
    spyConnection.wait();
    if (spyConnection.count() == 0) {
        return QVariant();
    }

    QSignalSpy dataSpy(socket, &QSslSocket::readyRead);
    socket->write(jsonDoc.toJson(QJsonDocument::Compact) + '\n');
    // FIXME: check why it waits the full time here
    dataSpy.wait(500);
    if (dataSpy.count() != 1) {
        qWarning() << "No data received";
        return QVariant();
    }

    QByteArray data = socket->readAll();
    socket->close();
    socket->deleteLater();

    // Make sure the response ends with '}\n'
    if (!data.endsWith("}\n")) {
        qWarning() << "JSON data does not end with \"}\n\"";
        return QVariant();
    }

    // Make sure the response it a valid JSON string
    QJsonParseError error;
    jsonDoc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parser error" << error.errorString();
        return QVariant();
    }

    QVariantMap response = jsonDoc.toVariant().toMap();

    if (response.value("id").toInt() == m_commandCounter) {
        m_commandCounter++;
        return jsonDoc.toVariant();
    }

    m_commandCounter++;
    return QVariant();
}

QVariant BaseTest::injectTcpSocketData(const QByteArray &data)
{
    QSslSocket *socket = new QSslSocket(this);
    typedef void (QSslSocket:: *sslErrorsSignal)(const QList<QSslError> &);
    QObject::connect(socket, static_cast<sslErrorsSignal>(&QSslSocket::sslErrors), this, &BaseTest::sslSocketSslErrors);

    QSignalSpy spyConnection(socket, &QSslSocket::connected);
    socket->connectToHostEncrypted(Engine::instance()->tcpSocketServer()->serverUrl().host(),
                                   static_cast<quint16>(Engine::instance()->tcpSocketServer()->serverUrl().port()));
    spyConnection.wait();
    if (spyConnection.count() == 0) {
        return QVariant();
    }

    QSignalSpy dataSpy(socket, &QSslSocket::readyRead);
    socket->write(data + '\n');
    dataSpy.wait();
    // FIXME: check why it waits the full time here
    dataSpy.wait(500);
    if (dataSpy.count() != 1) {
        qWarning() << "No data received";
        return QVariant();
    }

    QByteArray socketData = socket->readAll();
    socket->close();
    socket->deleteLater();

    // Make sure the response ends with '}\n'
    if (!socketData.endsWith("}\n")) {
        qWarning() << "JSON data does not end with \"}\n\"";
        return QVariant();
    }

    // Make sure the response it a valid JSON string
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(socketData, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parser error" << error.errorString();
        return QVariant();
    }

    m_commandCounter++;
    return jsonDoc.toVariant();
}

void BaseTest::initTestCase()
{
    qRegisterMetaType<RemoteProxyConnection::State>();
    qRegisterMetaType<RemoteProxyConnection::ConnectionType>();

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

    qCDebug(dcApplication()) << "Init test case done.";
    //restartEngine();
}

void BaseTest::cleanupTestCase()
{
    qCDebug(dcApplication()) << "Clean up test case.";
    cleanUpEngine();
}

