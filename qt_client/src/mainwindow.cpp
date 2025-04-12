#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

// Явные определения для метаобъектов
Q_DECLARE_METATYPE(QString)
Q_DECLARE_METATYPE(bool)

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_networkClient(new NetworkClient())
    , m_clientThread(new QThread(this))
{
    qRegisterMetaType<QString>();
    qRegisterMetaType<bool>();
    
    ui->setupUi(this);
    
    m_networkClient->moveToThread(m_clientThread);
    m_clientThread->start();

    connect(ui->connectButton, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);
    connect(ui->disconnectButton, &QPushButton::clicked, this, &MainWindow::onDisconnectButtonClicked);
    connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::onSendButtonClicked);
    
    connect(m_networkClient, &NetworkClient::messageReceived, this, &MainWindow::onMessageReceived);
    connect(m_networkClient, &NetworkClient::connectionStatusChanged, this, &MainWindow::onConnectionStatusChanged);
    connect(m_networkClient, &NetworkClient::errorOccurred, this, &MainWindow::onErrorOccurred);
}

MainWindow::~MainWindow()
{
    m_clientThread->quit();
    m_clientThread->wait();
    delete m_networkClient;
    delete ui;
}

void MainWindow::onConnectButtonClicked()
{
    QString host = ui->hostLineEdit->text();
    quint16 port = ui->portSpinBox->value();
    
    if (m_networkClient->connectToServer(host, port)) {
        ui->statusBar->showMessage(tr("Connected to server"));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Failed to connect to server"));
    }
}

void MainWindow::onDisconnectButtonClicked()
{
    m_networkClient->disconnectFromServer();
    ui->statusBar->showMessage(tr("Disconnected from server"));
}

void MainWindow::onSendButtonClicked()
{
    QString message = ui->messageLineEdit->text();
    if (!message.isEmpty()) {
        m_networkClient->sendMessage(message);
        ui->messageLineEdit->clear();
    }
}

void MainWindow::onMessageReceived(const QString &message)
{
    ui->chatTextEdit->append(tr("Server: %1").arg(message));
}

void MainWindow::onConnectionStatusChanged(bool connected)
{
    ui->connectButton->setEnabled(!connected);
    ui->disconnectButton->setEnabled(connected);
    ui->sendButton->setEnabled(connected);
}

void MainWindow::onErrorOccurred(const QString &error)
{
    QMessageBox::critical(this, tr("Error"), error);
}