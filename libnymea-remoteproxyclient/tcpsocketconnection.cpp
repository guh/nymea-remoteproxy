#include "tcpsocketconnection.h"
Q_LOGGING_CATEGORY(dcRemoteProxyClientTcpSocket, "RemoteProxyClientTcpSocket")

namespace remoteproxyclient {

TcpSocketConnection::TcpSocketConnection(QObject *parent) :
    ProxyConnection(parent)
{
    m_tcpSocket = new QSslSocket(this);

    connect(m_tcpSocket, &QSslSocket::disconnected, this, &TcpSocketConnection::onDisconnected);
    connect(m_tcpSocket, &QSslSocket::encrypted, this, &TcpSocketConnection::onEncrypted);
    connect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(m_tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onStateChanged(QAbstractSocket::SocketState)));
    connect(m_tcpSocket, SIGNAL(sslErrors(QList<QSslError>)), this, SIGNAL(sslErrors(QList<QSslError>)));
}

TcpSocketConnection::~TcpSocketConnection()
{
    m_tcpSocket->close();
}

void TcpSocketConnection::sendData(const QByteArray &data)
{
    m_tcpSocket->write(data);
}

void TcpSocketConnection::ignoreSslErrors()
{
    m_tcpSocket->ignoreSslErrors();
}

void TcpSocketConnection::ignoreSslErrors(const QList<QSslError> &errors)
{
    m_tcpSocket->ignoreSslErrors(errors);
}

void TcpSocketConnection::onDisconnected()
{
    qCDebug(dcRemoteProxyClientTcpSocket()) << "Disconnected from" << serverUrl().toString();
    setConnected(false);
}

void TcpSocketConnection::onEncrypted()
{
    qCDebug(dcRemoteProxyClientTcpSocket()) << "Connection encrypted";
}

void TcpSocketConnection::onError(QAbstractSocket::SocketError error)
{
    qCDebug(dcRemoteProxyClientTcpSocket()) << "Socket error occured" << error << m_tcpSocket->errorString();
    emit errorOccured(error);
}

void TcpSocketConnection::onStateChanged(QAbstractSocket::SocketState state)
{
    qCDebug(dcRemoteProxyClientTcpSocket()) << "Socket state changed" << state;

    switch (state) {
    case QAbstractSocket::ConnectedState:
        qCDebug(dcRemoteProxyClientTcpSocket()) << "Connected with" << serverUrl().toString();
        setConnected(true);
        break;
    default:
        setConnected(false);
        break;
    }

    emit stateChanged(state);
}

void TcpSocketConnection::onReadyRead()
{
    emit dataReceived(m_tcpSocket->readAll());
}

void TcpSocketConnection::connectServer(const QUrl &serverUrl)
{
    qCDebug(dcRemoteProxyClientTcpSocket()) << "Connecting to" << this->serverUrl().toString();
    setServerUrl(serverUrl);

    if (serverUrl.scheme() == "tcp") {
        m_tcpSocket->connectToHost(QHostAddress(this->serverUrl().host()), static_cast<quint16>(this->serverUrl().port()));
    } else {
        m_tcpSocket->connectToHostEncrypted(this->serverUrl().host(), static_cast<quint16>(this->serverUrl().port()));
    }
}

void TcpSocketConnection::disconnectServer()
{
    qCDebug(dcRemoteProxyClientTcpSocket()) << "Disconnecting from" << serverUrl().toString();
    m_tcpSocket->close();
}

}
