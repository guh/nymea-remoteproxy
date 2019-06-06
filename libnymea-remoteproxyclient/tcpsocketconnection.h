#ifndef TCPSOCKETCONNECTION_H
#define TCPSOCKETCONNECTION_H

#include <QObject>
#include <QTcpSocket>
#include <QSslSocket>
#include <QLoggingCategory>

#include "proxyconnection.h"

Q_DECLARE_LOGGING_CATEGORY(dcRemoteProxyClienTcpSocket)

namespace remoteproxyclient {

class TcpSocketConnection : public ProxyConnection
{
    Q_OBJECT

public:
    explicit TcpSocketConnection(QObject *parent = nullptr);
    ~TcpSocketConnection() override;

    void sendData(const QByteArray &data) override;

    void ignoreSslErrors() override;
    void ignoreSslErrors(const QList<QSslError> &errors) override;

private:
    QSslSocket *m_tcpSocket = nullptr;

private slots:
    void onDisconnected();
    void onEncrypted();
    void onError(QAbstractSocket::SocketError error);
    void onStateChanged(QAbstractSocket::SocketState state);
    void onReadyRead();

public slots:
    void connectServer(const QUrl &serverUrl) override;
    void disconnectServer() override;

};

}

#endif // TCPSOCKETCONNECTION_H
