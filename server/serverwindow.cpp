#include "serverwindow.h"
#include "./ui_serverwindow.h"
#include "../client/mainwindow.h"
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QProcess>
#include <QDir>
#include <QScrollBar>
#include <QCoreApplication>

ServerWindow::ServerWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::ServerWindow)
{
    ui->setupUi(this);
    logMessage("Server initializing...");

    if (!tcpServer.listen(QHostAddress::Any, port))
    {
        logMessage("Unable to start the server: " + tcpServer.errorString(), true);
        close();
        return;
    }

    statusUpdateTimer.start(5000);
    logMessage("Server started on port " + QString::number(port));
    ui->statusbar->showMessage("Server is running on port " + QString::number(port));
}

ServerWindow::~ServerWindow()
{
    logMessage("Server shutting down...");
    foreach (QTcpSocket *socket, clients.keys())
    {
        removeClient(socket);
    }
    foreach (QProcess *process, clientProcesses)
    {
        process->terminate();
        process->waitForFinished();
        delete process;
    }
    tcpServer.close();
    delete ui;
}

void ServerWindow::newClientConnection()
{
    QTcpSocket *clientSocket = tcpServer.nextPendingConnection();
    connect(clientSocket, &QTcpSocket::readyRead, this, &ServerWindow::readSocket);
    connect(clientSocket, &QTcpSocket::disconnected, this, &ServerWindow::socketDisconnected);
    connect(clientSocket, &QTcpSocket::errorOccurred, this, &ServerWindow::handleSocketError);

    clients.insert(clientSocket, "Unknown");
    setupPingTimer(clientSocket);
    connectedClients++;

    QJsonObject welcomeMsg;
    welcomeMsg["type"] = "welcome";
    welcomeMsg["message"] = "Welcome to Casino Server!";
    sendToClient(clientSocket, welcomeMsg);

    logMessage("New client connected from " + clientSocket->peerAddress().toString());
    updateClientList();
}

void ServerWindow::socketDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (socket)
    {
        removeClient(socket);
    }
}

void ServerWindow::handleSocketError(QAbstractSocket::SocketError error)
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (socket)
    {
        logMessage("Socket error: " + socket->errorString(), true);
        removeClient(socket);
    }
}

void ServerWindow::readSocket()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket)
        return;

    QByteArray data = socket->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isObject())
    {
        QJsonObject obj = doc.object();
        QString type = obj["type"].toString();
        QString message = obj["message"].toString();

        if (type == "login")
        {
            QString oldName = clients.value(socket);
            clients[socket] = message;
            logMessage(oldName + " is now known as " + message);
            sendToAllClients(message + " joined the casino", socket);
            updateClientList();
        }
        else if (type == "message")
        {
            QString username = clients.value(socket);
            QString fullMessage = username + ": " + message;
            logMessage(fullMessage);
            sendToAllClients(fullMessage, socket);
        }
        else if (type == "ping")
        {
            QJsonObject pong;
            pong["type"] = "pong";
            pong["timestamp"] = QDateTime::currentMSecsSinceEpoch();
            sendToClient(socket, pong);
        }
    }
}

void ServerWindow::sendToAllClients(const QString &message, QTcpSocket *exclude)
{
    QJsonObject obj;
    obj["type"] = "message";
    obj["message"] = message;
    obj["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();

    foreach (QTcpSocket *socket, clients.keys())
    {
        if (socket != exclude && socket->state() == QAbstractSocket::ConnectedState)
        {
            socket->write(data);
        }
    }
}

void ServerWindow::sendToClient(QTcpSocket *client, const QJsonObject &message)
{
    if (client && client->state() == QAbstractSocket::ConnectedState)
    {
        QJsonDocument doc(message);
        client->write(doc.toJson());
    }
}

void ServerWindow::setupPingTimer(QTcpSocket *client)
{
    QTimer *timer = new QTimer(this);
    timer->setInterval(30000);
    connect(timer, &QTimer::timeout, [this, client]()
            {
        if (client->state() == QAbstractSocket::ConnectedState) {
            QJsonObject ping;
            ping["type"] = "ping";
            ping["timestamp"] = QDateTime::currentMSecsSinceEpoch();
            sendToClient(client, ping);
        } });
    timer->start();
    clientPingTimers[client] = timer;
}

void ServerWindow::removeClient(QTcpSocket *socket)
{
    QString username = clients.value(socket);
    logMessage(username + " disconnected");

    if (clientPingTimers.contains(socket))
    {
        clientPingTimers[socket]->stop();
        clientPingTimers[socket]->deleteLater();
        clientPingTimers.remove(socket);
    }

    clients.remove(socket);
    socket->deleteLater();
    connectedClients--;
    updateClientList();
}

void ServerWindow::updateClientList()
{
    QStringList clientList;
    foreach (const QString &username, clients.values())
    {
        clientList << username;
    }
    ui->clientListWidget->clear();
    ui->clientListWidget->addItems(clientList);
}

void ServerWindow::broadcastServerStatus()
{
    QJsonObject status;
    status["type"] = "status";
    status["connectedClients"] = connectedClients;
    status["timestamp"] = QDateTime::currentMSecsSinceEpoch();

    QJsonDocument doc(status);
    QByteArray data = doc.toJson();

    foreach (QTcpSocket *socket, clients.keys())
    {
        if (socket->state() == QAbstractSocket::ConnectedState)
        {
            socket->write(data);
        }
    }
}

void ServerWindow::logMessage(const QString &message, bool isError)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString formattedMessage = QString("[%1] %2").arg(timestamp, message);

    if (isError)
    {
        ui->textBrowser->append("<span style='color: red;'>" + formattedMessage + "</span>");
    }
    else
    {
        ui->textBrowser->append(formattedMessage);
    }

    ui->textBrowser->verticalScrollBar()->setValue(ui->textBrowser->verticalScrollBar()->maximum());
}
