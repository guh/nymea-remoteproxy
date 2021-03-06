/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2020, nymea GmbH
*  Contact: contact@nymea.io
*
*  This file is part of nymea.
*  This project including source code and documentation is protected by copyright law, and
*  remains the property of nymea GmbH. All rights, including reproduction, publication,
*  editing and translation, are reserved. The use of this project is subject to the terms of a
*  license agreement to be concluded with nymea GmbH in accordance with the terms
*  of use of nymea GmbH, available under https://nymea.io/license
*
*  GNU General Public License Usage
*  Alternatively, this project may be redistributed and/or modified under
*  the terms of the GNU General Public License as published by the Free Software Foundation,
*  GNU version 3. this project is distributed in the hope that it will be useful, but WITHOUT ANY
*  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
*  PURPOSE. See the GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License along with this project.
*  If not, see <https://www.gnu.org/licenses/>.
*
*  For any further details and any questions please contact us under contact@nymea.io
*  or see our FAQ/Licensing Information on https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "engine.h"
#include "loggingcategories.h"

namespace remoteproxy {

Engine *Engine::s_instance = nullptr;

Engine *Engine::instance()
{
    if (!s_instance) {
        qCDebug(dcEngine()) << "Create server engine";
        s_instance = new Engine();
    }

    return s_instance;
}

void Engine::destroy()
{
    qCDebug(dcEngine()) << "Destroy server engine";
    if (s_instance) {
        delete s_instance;
    }

    s_instance = nullptr;
}

bool Engine::exists()
{
    return s_instance != nullptr;
}

void Engine::start(ProxyConfiguration *configuration)
{
    if (!m_running)
        qCDebug(dcEngine()) << "Start server engine";

    // Clean up
    clean();

    m_configuration = configuration;
    qCDebug(dcApplication()) << "Using configuration" << m_configuration;

    // Make sure an authenticator was registered
    Q_ASSERT_X(m_authenticator != nullptr, "Engine", "There is no authenticator registerd.");

    m_proxyServer = new ProxyServer(this);
    m_webSocketServer = new WebSocketServer(m_configuration->sslConfiguration(), this);

    QUrl websocketServerUrl;
    websocketServerUrl.setScheme("wss");
    websocketServerUrl.setHost(m_configuration->webSocketServerHost().toString());
    websocketServerUrl.setPort(m_configuration->webSocketServerPort());

    m_webSocketServer->setServerUrl(websocketServerUrl);

    m_proxyServer->registerTransportInterface(m_webSocketServer);

    qCDebug(dcEngine()) << "Starting proxy server";
    m_proxyServer->startServer();

    m_monitorServer = new MonitorServer(configuration->monitorSocketFileName(), this);
    m_monitorServer->startServer();

    if (configuration->logEngineEnabled())
        m_logEngine->enable();

    // Set running true in the next event loop
    QMetaObject::invokeMethod(this, QString("setRunning").toLatin1().data(), Qt::QueuedConnection, Q_ARG(bool, true));
}

void Engine::stop()
{
    if (m_running)
        qCDebug(dcEngine()) << "Stop server engine";

    clean();
    setRunning(false);
}

bool Engine::running() const
{
    return m_running;
}

bool Engine::developerMode() const
{
    return m_developerMode;
}

QString Engine::serverName() const
{
    return m_configuration->serverName();
}

void Engine::setAuthenticator(Authenticator *authenticator)
{
    if (m_authenticator == authenticator)
        return;

    if (m_authenticator) {
        qCDebug(dcEngine()) << "There is already an authenticator registered. Unregister default authenticator";
        m_authenticator = nullptr;
    }

    m_authenticator = authenticator;
}

void Engine::setDeveloperModeEnabled(bool enabled)
{
    m_developerMode = enabled;
}

ProxyConfiguration *Engine::configuration() const
{
    return m_configuration;
}

Authenticator *Engine::authenticator() const
{
    return m_authenticator;
}

ProxyServer *Engine::proxyServer() const
{
    return m_proxyServer;
}

WebSocketServer *Engine::webSocketServer() const
{
    return m_webSocketServer;
}

MonitorServer *Engine::monitorServer() const
{
    return m_monitorServer;
}

LogEngine *Engine::logEngine() const
{
    return m_logEngine;
}

Engine::Engine(QObject *parent) :
    QObject(parent)
{
    m_lastTimeStamp = QDateTime::currentDateTime().toMSecsSinceEpoch();

    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);
    m_timer->setInterval(50);

    connect(m_timer, &QTimer::timeout, this, &Engine::onTimerTick);

    m_logEngine = new LogEngine(this);
}

Engine::~Engine()
{
    stop();
}

QVariantMap Engine::createServerStatistic()
{
    QVariantMap monitorData;
    monitorData.insert("serverName", m_configuration->serverName());
    monitorData.insert("serverVersion", SERVER_VERSION_STRING);
    monitorData.insert("apiVersion", API_VERSION_STRING);
    monitorData.insert("proxyStatistic", proxyServer()->currentStatistics());
    return monitorData;
}

void Engine::onTimerTick()
{
    qint64 timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    qint64 deltaTime = timestamp - m_lastTimeStamp;
    m_lastTimeStamp = timestamp;

    m_currentTimeCounter += deltaTime;
    if (m_currentTimeCounter >= 1000) {
        // One second passed, do second tick
        m_proxyServer->tick();

        QVariantMap serverStatistics = createServerStatistic();
        m_monitorServer->updateClients(serverStatistics);
        m_logEngine->logStatistics(serverStatistics.value("proxyStatistic").toMap().value("tunnelCount").toInt(),
                                   serverStatistics.value("proxyStatistic").toMap().value("clientCount").toInt(),
                                   serverStatistics.value("proxyStatistic").toMap().value("troughput").toInt());

        m_currentTimeCounter = 0;
    }
}

void Engine::clean()
{
    if (m_monitorServer) {
        m_monitorServer->stopServer();
        delete m_monitorServer;
        m_monitorServer = nullptr;
    }

    if (m_proxyServer) {
        m_proxyServer->stopServer();
        delete m_proxyServer;
        m_proxyServer = nullptr;
    }

    if (m_webSocketServer) {
        delete m_webSocketServer;
        m_webSocketServer = nullptr;
    }

    if (m_configuration) {
        m_configuration = nullptr;
    }
}


void Engine::setRunning(bool running)
{
    if (m_running == running)
        return;

    qCDebug(dcEngine()) << "Engine is" << (running ? "now running." : "not running any more.");

    if (running) {
        m_timer->start();
    } else {
        m_timer->stop();
    }

    m_running = running;
    emit runningChanged(m_running);
}


}
