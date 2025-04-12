#include "networkclient.h"
#include <QHostAddress>

// Явные определения для метаобъектов
Q_DECLARE_METATYPE(QString)
Q_DECLARE_METATYPE(bool)

NetworkClient::NetworkClient(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_networkThread(new QThread(this))
{
    qRegisterMetaType<QString>();
    qRegisterMetaType<bool>();
    
    m_socket->moveToThread(m_networkThread);
    connect(m_socket, &QTcpSocket::connected, this, &NetworkClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkClient::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &NetworkClient::onError);
    
    m_networkThread->start();
}

NetworkClient::~NetworkClient()
{
    disconnectFromServer();
    m_networkThread->quit();
    m_networkThread->wait();
}

bool NetworkClient::connectToServer(const QString &host, quint16 port)
{
    QMutexLocker locker(&m_mutex);
    m_socket->connectToHost(host, port);
    return m_socket->waitForConnected(5000);
}

void NetworkClient::disconnectFromServer()
{
    QMutexLocker locker(&m_mutex);
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
        m_socket->waitForDisconnected(5000);
    }
}

void NetworkClient::sendMessage(const QString &message)
{
    QMutexLocker locker(&m_mutex);
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        QByteArray data = message.toUtf8();
        m_socket->write(data);
        m_socket->flush();
    }
}

void NetworkClient::onConnected()
{
    emit connectionStatusChanged(true);
}

void NetworkClient::onDisconnected()
{
    emit connectionStatusChanged(false);
}

void NetworkClient::onReadyRead()
{
    QMutexLocker locker(&m_mutex);
    while (m_socket->bytesAvailable() > 0) {
        QByteArray data = m_socket->readAll();
        emit messageReceived(QString::fromUtf8(data));
    }
}

void NetworkClient::onError(QAbstractSocket::SocketError socketError)
{
    emit errorOccurred(m_socket->errorString());
} 