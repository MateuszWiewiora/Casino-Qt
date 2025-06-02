#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDateTime>
#include <QMessageBox>
#include <QInputDialog>
#include <QLineEdit>
#include <QTextBrowser>
#include <QPushButton>
#include "loginwindow.h"
#include "clickablelabel.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class Minigame1;
class Minigame2;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static constexpr float CASINO_EDGE = 0.03f;
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    int getSelectedOption() const;

private slots:
    void on_homeButton_clicked();
    void on_Minigame1_Button_clicked();
    void on_Minigame2_Button_clicked();
    void on_Minigame3_Button_clicked();
    void on_homeMinigame1_Button_clicked();
    void on_homeMinigame2_Button_clicked();
    void on_homeMinigame3_Button_clicked();
    void on_LoginButton_clicked();
    void on_betAmountChanged(double value);
    void on_betButton_clicked();
    void on_doubleButton_clicked();
    void on_halfButton_clicked();
    void on_workButton_clicked();
    void on_rollButton_clicked();
    void on_halfButton2_clicked();
    void on_doubleButton2_clicked();
    void on_moreButton_clicked();
    void on_sameButton_clicked();
    void on_lessButton_clicked();
    void on_selectionChanged(bool selected);
    void rollDices();
    void resetMinigame2();
    void on_sendButton_clicked();
    void on_chatInput_returnPressed();

    void connectToServer();
    void connected();
    void disconnected();
    void handleSocketError(QAbstractSocket::SocketError error);
    void readSocket();
    void sendMessage(const QString &message);
    void login(const QString &username);
    void handlePing();
    void handlePong(qint64 timestamp);
    void handleServerStatus(int connectedClients);
    void handleWelcome(const QString &message);
    void reconnectToServer();
    void sendPing();

private:
    void initializeRandomSeed();
    void setupMinigames();
    void setupMinigame1Connections();
    void setupMinigame2Connections();
    void setupBetAmountConnections();
    void disableBettingUntilSelection();
    void updateMoneyDisplay(long money);
    void updateBetButtonState();
    void updateBetAmountLimits();
    void addWorkEarnings();
    void setGuessButtonsEnabled(bool enabled);
    void updateRollButtonState();
    void updateConnectionStatus(bool connected);
    void processServerMessage(const QJsonObject &message);
    void displayChatMessage(const QString &sender, const QString &message);

    Ui::MainWindow *ui;
    LoginWindow *loginWindow = nullptr;
    Minigame1 *minigame1 = nullptr;
    Minigame2 *minigame2 = nullptr;
    long playerBalance = 10000;
    bool ANIMATION_MODE = false;
    int playerRoll = 0;
    int casinoRoll = 0;
    long currentDiceBet = 0;
    bool minigame2WaitingForGuess = false;

    QTcpSocket socket;
    QString username;
    bool isConnected = false;
    QString serverAddress = "127.0.0.1";
    quint16 serverPort = 45000;
    QTimer pingTimer;
    QTimer reconnectTimer;
    int reconnectAttempts = 0;
    static const int MAX_RECONNECT_ATTEMPTS = 5;
    static const int RECONNECT_INTERVAL = 5000;
};
#endif // MAINWINDOW_H
