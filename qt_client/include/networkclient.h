#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QThread>
#include <QMutex>

class NetworkClient : public QObject
{
    Q_OBJECT

public:
    explicit NetworkClient(QObject *parent = nullptr);
    virtual ~NetworkClient();

    bool connectToServer(const QString &host, quint16 port);
    void disconnectFromServer();
    void sendMessage(const QString &message);

signals:
    void messageReceived(const QString &message);
    void connectionStatusChanged(bool connected);
    void errorOccurred(const QString &error);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket *m_socket;
    QMutex m_mutex;
    QThread *m_networkThread;
};

#endif // NETWORKCLIENT_H