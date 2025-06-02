#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QTimer>
#include <QProcess>

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
    void readSocket();
    void handleSocketError(QAbstractSocket::SocketError error);
    void updateClientList();
    void broadcastServerStatus();

private:
    Ui::ServerWindow *ui;
    QTcpServer tcpServer;
    QMap<QTcpSocket *, QString> clients;
    QMap<QTcpSocket *, QTimer *> clientPingTimers;
    QList<QProcess *> clientProcesses;
    quint16 port = 45000;
    QTimer statusUpdateTimer;
    int connectedClients = 0;

    void sendToAllClients(const QString &message, QTcpSocket *exclude = nullptr);
    void sendToClient(QTcpSocket *client, const QJsonObject &message);
    void setupPingTimer(QTcpSocket *client);
    void removeClient(QTcpSocket *client);
    void logMessage(const QString &message, bool isError = false);
};
#endif // SERVERWINDOW_H
