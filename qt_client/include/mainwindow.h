#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "networkclient.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();

private slots:
    void onConnectButtonClicked();
    void onDisconnectButtonClicked();
    void onSendButtonClicked();
    void onMessageReceived(const QString &message);
    void onConnectionStatusChanged(bool connected);
    void onErrorOccurred(const QString &error);

private:
    Ui::MainWindow *ui;
    NetworkClient *m_networkClient;
    QThread *m_clientThread;
};

#endif // MAINWINDOW_H