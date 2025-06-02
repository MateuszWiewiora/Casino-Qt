#include "serverwindow.h"
#include "./ui_serverwindow.h"
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

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

    connect(&tcpServer, &QTcpServer::newConnection, this, &ServerWindow::newClientConnection);

    logMessage("Server started on port " + QString::number(port));
    ui->statusbar->showMessage("Server is running on port " + QString::number(port));
}

ServerWindow::~ServerWindow()
{
    logMessage("Server shutting down...");
    if (currentClient)
    {
        currentClient->disconnectFromHost();
        currentClient->deleteLater();
    }
    tcpServer.close();
    delete ui;
}

void ServerWindow::newClientConnection()
{
    if (currentClient)
    {
        QTcpSocket *newClient = tcpServer.nextPendingConnection();
        newClient->disconnectFromHost();
        newClient->deleteLater();
        logMessage("Rejected new connection - server already has a client", true);
        return;
    }

    currentClient = tcpServer.nextPendingConnection();
    connect(currentClient, &QTcpSocket::readyRead, this, &ServerWindow::readSocket);
    connect(currentClient, &QTcpSocket::disconnected, this, &ServerWindow::socketDisconnected);
    connect(currentClient, &QTcpSocket::errorOccurred, this, &ServerWindow::handleSocketError);

    QJsonObject welcomeMsg;
    welcomeMsg["type"] = "welcome";
    welcomeMsg["message"] = "Welcome to Casino Server!";
    sendToClient(welcomeMsg);

    logMessage("New client connected from " + currentClient->peerAddress().toString());
}

void ServerWindow::socketDisconnected()
{
    if (currentClient)
    {
        logMessage("Client disconnected");
        currentClient->deleteLater();
        currentClient = nullptr;
    }
}

void ServerWindow::handleSocketError(QAbstractSocket::SocketError error)
{
    if (currentClient)
    {
        logMessage("Socket error: " + currentClient->errorString(), true);
        currentClient->disconnectFromHost();
        currentClient->deleteLater();
        currentClient = nullptr;
    }
}

void ServerWindow::readSocket()
{
    qDebug() << "Reading socket";
    if (!currentClient)
        return;

    QByteArray data = currentClient->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isObject())
    {
        QJsonObject obj = doc.object();
        QString type = obj["type"].toString();

        if (type == "login")
        {
            qDebug() << "Login request received";
            QString username = obj["username"].toString();
            QString password = obj["password"].toString();
            handleLogin(username, password);
        }
        else if (type == "register")
        {
            qDebug() << "Register request received";
            QString username = obj["username"].toString();
            QString password = obj["password"].toString();
            handleRegister(username, password);
        }
    }
}

void ServerWindow::handleLogin(const QString &username, const QString &password)
{
    QJsonObject response;
    response["type"] = "login_response";
    response["success"] = registeredUsers.contains(username) && registeredUsers[username] == password;
    response["username"] = username;
    sendToClient(response);
}

void ServerWindow::handleRegister(const QString &username, const QString &password)
{
    QJsonObject response;
    response["type"] = "register_response";

    if (registeredUsers.contains(username) && username != "" && password != "")
    {
        response["success"] = false;
    }
    else
    {
        registeredUsers[username] = password;
        response["success"] = true;
    }

    sendToClient(response);
}

void ServerWindow::sendToClient(const QJsonObject &message)
{
    if (currentClient && currentClient->state() == QAbstractSocket::ConnectedState)
    {
        QJsonDocument doc(message);
        currentClient->write(doc.toJson());
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
