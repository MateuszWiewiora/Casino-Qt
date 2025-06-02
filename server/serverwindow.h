#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QScrollBar>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class ServerWindow;
}
QT_END_NAMESPACE

class ServerWindow : public QMainWindow
{
    Q_OBJECT

public:
    ServerWindow(QWidget *parent = nullptr);
    ~ServerWindow();

private slots:
    void newClientConnection();
    void socketDisconnected();
    void handleSocketError(QAbstractSocket::SocketError error);
    void readSocket();

private:
    void handleLogin(const QString &username, const QString &password);
    void handleRegister(const QString &username, const QString &password);
    void sendToClient(const QJsonObject &message);
    void logMessage(const QString &message, bool isError = false);

    Ui::ServerWindow *ui;
    QTcpServer tcpServer;
    QTcpSocket *currentClient = nullptr;
    QString currentUsername;
    QMap<QString, QString> registeredUsers;
    const quint16 port = 45000;
};

#endif
