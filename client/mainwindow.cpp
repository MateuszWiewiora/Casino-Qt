#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QFile>
#include <QTimer>
#include <random>
#include <QDirIterator>
#include <ctime>
#include <QRandomGenerator>
#include "minigame1.h"
#include "minigame2.h"
#include "minigame3.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QMessageBox>
#include <QCryptographicHash>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initializeRandomSeed();
    updateMoneyDisplay(playerBalance);

    setupMinigames();
    setupBetAmountConnections();
    disableBettingUntilSelection();
    updateBetAmountLimits();

    minigame2->initializeChances();

    connect(&socket, &QTcpSocket::connected, this, &MainWindow::connected);
    connect(&socket, &QTcpSocket::disconnected, this, &MainWindow::disconnected);
    connect(&socket, &QTcpSocket::errorOccurred, this, &MainWindow::handleSocketError);
    connect(&socket, &QTcpSocket::readyRead, this, &MainWindow::readSocket);

    connectToServer();
}

void MainWindow::connectToServer()
{
    if (!isConnected)
    {
        updateConnectionStatus(false);
        socket.connectToHost(serverAddress, serverPort);
    }
}

void MainWindow::connected()
{
    isConnected = true;
    updateConnectionStatus(true);
}

void MainWindow::disconnected()
{
    isConnected = false;
    updateConnectionStatus(false);
}

void MainWindow::handleSocketError(QAbstractSocket::SocketError error)
{
    QString errorMessage = "Connection error: " + socket.errorString();
    ui->statusbar->showMessage(errorMessage);
    qDebug() << errorMessage;
}

void MainWindow::readSocket()
{
    QByteArray data = socket.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isObject())
    {
        QJsonObject obj = doc.object();
        QString type = obj["type"].toString();
        processServerMessage(obj);
    }
}

void MainWindow::processServerMessage(const QJsonObject &message)
{
    QString type = message["type"].toString();

    if (type == "login_response")
    {
        qDebug() << "Login response received";
        bool success = message["success"].toBool();
        if (success)
        {
            loginWindow->hide();
            this->show();
            username = message["username"].toString();
            ui->usernameLabel->setText(username);
            ui->LoginButton->setText("Log out");
        }
        else
        {
            QMessageBox::warning(this, "Login failed", "Invalid username or password");
        }
    }
    else if (type == "register_response")
    {
        qDebug() << "Register response received";
        bool success = message["success"].toBool();
        if (success)
        {
            QMessageBox::information(this, "Registration successful", "Registration successful");
        }
        else
        {
            QMessageBox::warning(this, "Registration failed", "Username already exists");
        }
    }
}

void MainWindow::login(const QString &username, const QString &password)
{
    qDebug() << "Logging in2";
    if (!isConnected)
        return;

    QByteArray passwordBytes = password.toUtf8();
    QByteArray hashedPassword = QCryptographicHash::hash(passwordBytes, QCryptographicHash::Sha256).toHex();

    QJsonObject obj;
    obj["type"] = "login";
    obj["username"] = username;
    obj["password"] = QString(hashedPassword);
    QJsonDocument doc(obj);
    qDebug() << "Sending login request";
    socket.write(doc.toJson());
}

void MainWindow::registerUser(const QString &username, const QString &password)
{
    qDebug() << "Registering2";
    QByteArray passwordBytes = password.toUtf8();
    QByteArray hashedPassword = QCryptographicHash::hash(passwordBytes, QCryptographicHash::Sha256).toHex();

    QJsonObject obj;
    obj["type"] = "register";
    obj["username"] = username;
    obj["password"] = QString(hashedPassword);
    QJsonDocument doc(obj);
    qDebug() << "Sending register request";
    socket.write(doc.toJson());
}

void MainWindow::updateConnectionStatus(bool connected)
{
    if (connected)
    {
        ui->statusbar->showMessage("Connected to server");
    }
    else
    {
        ui->statusbar->showMessage("Disconnected from server");
    }
}

void MainWindow::initializeRandomSeed()
{
    srand(time(0));
}

void MainWindow::setupMinigames()
{
    minigame1 = new Minigame1(this, playerBalance);
    minigame2 = new Minigame2(this, playerBalance);
    minigame3 = new Minigame3(this, ui, playerBalance);

    setupMinigame1Connections();
    setupMinigame2Connections();
    setupMinigame3Connections();

    minigame1->createAndSetupClickableLabels(this, ui);
    minigame1->setupInitialGameImages();

    if (minigame3)
    {
        minigame3->resetGame();
    }
}

void MainWindow::setupMinigame1Connections()
{
    connect(minigame1, &Minigame1::moneyUpdated, this, &MainWindow::updateMoneyDisplay);
    connect(minigame1, &Minigame1::betButtonEnabled, ui->betButton, &QPushButton::setEnabled);
    connect(minigame1, &Minigame1::selectionChanged, this, &MainWindow::updateBetButtonState);
    connect(minigame1, &Minigame1::option1NameUpdated, ui->option1NameLabel, &QLabel::setText);
    connect(minigame1, &Minigame1::option2NameUpdated, ui->option2NameLabel, &QLabel::setText);
}

void MainWindow::setupMinigame3Connections()
{
    connect(minigame3, &Minigame3::moneyUpdated, this, &MainWindow::updateMoneyDisplay);
    connect(minigame3, &Minigame3::gameStatusUpdated, ui->gameStatusLabel3, &QLabel::setText);
    connect(minigame3, &Minigame3::playButtonEnabled, ui->playButton3, &QPushButton::setEnabled);
    connect(ui->playButton3, &QPushButton::clicked, minigame3, &Minigame3::confirmBetAndStartGame);
}

void MainWindow::setupMinigame2Connections()
{
    connect(minigame2, &Minigame2::moneyUpdated, this, &MainWindow::updateMoneyDisplay);
    connect(minigame2, &Minigame2::playerRollUpdated, ui->playerRollLabel, &QLabel::setText);
    connect(minigame2, &Minigame2::resultUpdated, ui->resultLabel, &QLabel::setText);
    connect(minigame2, &Minigame2::guessButtonsEnabled, this, &MainWindow::setGuessButtonsEnabled);
    connect(minigame2, &Minigame2::rollButtonEnabled, ui->rollButton, &QPushButton::setEnabled);
    connect(minigame2, &Minigame2::lessChanceUpdated, ui->lessChanceLabel, &QLabel::setText);
    connect(minigame2, &Minigame2::sameChanceUpdated, ui->sameChanceLabel, &QLabel::setText);
    connect(minigame2, &Minigame2::moreChanceUpdated, ui->moreChanceLabel, &QLabel::setText);
    connect(minigame2, &Minigame2::lessMultiplierUpdated, ui->lessMultiplierLabel, &QLabel::setText);
    connect(minigame2, &Minigame2::sameMultiplierUpdated, ui->sameMultiplierLabel, &QLabel::setText);
    connect(minigame2, &Minigame2::moreMultiplierUpdated, ui->moreMultiplierLabel, &QLabel::setText);
}

void MainWindow::setupBetAmountConnections()
{
    connect(ui->doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::on_betAmountChanged);
    connect(ui->doubleSpinBox2, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::on_betAmountChanged);
    connect(ui->doubleSpinBox3, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::on_betAmountChanged);
}

void MainWindow::disableBettingUntilSelection()
{
    ui->betButton->setEnabled(false);
}

void MainWindow::on_homeButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_Minigame1_Button_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_Minigame2_Button_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    minigame2->resetGame();
    updateBetAmountLimits();
}

void MainWindow::on_Minigame3_Button_clicked()
{
    ui->stackedWidget->setCurrentIndex(3);
}

void MainWindow::on_homeMinigame1_Button_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_homeMinigame2_Button_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    minigame2->resetGame();
    updateBetAmountLimits();
}

void MainWindow::on_homeMinigame3_Button_clicked()
{
    ui->stackedWidget->setCurrentIndex(3);
}

void MainWindow::updateMoneyDisplay(long money)
{
    ui->cashLabel->setText("$" + QString::number(money / 100., 'f', 2));
    updateBetAmountLimits();
}

void MainWindow::on_LoginButton_clicked()
{
    if (username == "")
    {
        loginWindow = new LoginWindow();
        loginWindow->setMainWindow(this);
        connect(loginWindow, &LoginWindow::returnToMain, this, &MainWindow::show);
        connect(loginWindow, &LoginWindow::returnToMain, loginWindow, &QWidget::close);

        loginWindow->show();
        this->hide();
    }
    else
    {
        QJsonObject obj;
        obj["type"] = "logout";
        QJsonDocument doc(obj);
        qDebug() << "Sending logout request";
        socket.write(doc.toJson());

        username = "";
        ui->LoginButton->setText("Log in");
        ui->usernameLabel->setText("Guest");
    }
}

void MainWindow::on_betAmountChanged(double value)
{
    if (ui->stackedWidget->currentIndex() == 1)
    {
        updateBetButtonState();
    }
    else if (ui->stackedWidget->currentIndex() == 2)
    {
        updateRollButtonState();
    }
    else if (ui->stackedWidget->currentIndex() == 3)
    {
        updatePlayButtonState();
    }
}

void MainWindow::updateBetButtonState()
{
    bool hasSelection = minigame1->getSelectedOption() != 0;
    bool hasValidBet = ui->doubleSpinBox->value() > 0.0;
    ui->betButton->setEnabled(hasSelection && hasValidBet);
}

void MainWindow::on_betButton_clicked()
{
    int selectedOption = minigame1->getSelectedOption();
    if (!selectedOption)
        return;

    double betAmount = ui->doubleSpinBox->value();
    if (!minigame1->placeBet(betAmount))
        return;

    qDebug() << "Betting" << betAmount << "on option" << selectedOption;
}

void MainWindow::updateBetAmountLimits()
{
    double maxBet = playerBalance / 100.;
    ui->doubleSpinBox->setMinimum(0.0);
    ui->doubleSpinBox->setMaximum(maxBet);
    ui->doubleSpinBox2->setMinimum(0.0);
    ui->doubleSpinBox2->setMaximum(maxBet);
    ui->doubleSpinBox3->setMinimum(0.0);
    ui->doubleSpinBox3->setMaximum(maxBet);
}

void MainWindow::on_doubleButton_clicked()
{
    double currentBet = ui->doubleSpinBox->value();
    double doubledBet = currentBet * 2;
    double maxBet = static_cast<double>(playerBalance) / 100.;
    ui->doubleSpinBox->setValue(std::min(doubledBet, maxBet));
}

void MainWindow::on_halfButton_clicked()
{
    double currentBet = ui->doubleSpinBox->value();
    double halvedBet = std::round((currentBet / 2) * 100) / 100;
    ui->doubleSpinBox->setValue(halvedBet);
}

void MainWindow::on_workButton_clicked()
{
    addWorkEarnings();
}

void MainWindow::addWorkEarnings()
{
    playerBalance += 10;
    updateMoneyDisplay(playerBalance);
}

void MainWindow::on_rollButton_clicked()
{
    double betAmount = ui->doubleSpinBox2->value();
    if (!minigame2->startDiceGame(betAmount))
    {
        qDebug() << "Invalid bet amount";
        return;
    }
}

void MainWindow::on_halfButton2_clicked()
{
    double currentBet = ui->doubleSpinBox2->value();
    double halvedBet = std::round((currentBet / 2) * 100) / 100;
    ui->doubleSpinBox2->setValue(halvedBet);
}

void MainWindow::on_doubleButton2_clicked()
{
    double currentBet = ui->doubleSpinBox2->value();
    double doubledBet = currentBet * 2;
    double maxBet = static_cast<double>(playerBalance) / 100.;
    ui->doubleSpinBox2->setValue(std::min(doubledBet, maxBet));
}

void MainWindow::on_moreButton_clicked()
{
    minigame2->checkAnswer("More");
}

void MainWindow::on_sameButton_clicked()
{
    minigame2->checkAnswer("Same");
}

void MainWindow::on_lessButton_clicked()
{
    minigame2->checkAnswer("Less");
}

void MainWindow::setGuessButtonsEnabled(bool enabled)
{
    ui->moreButton->setEnabled(enabled);
    ui->sameButton->setEnabled(enabled);
    ui->lessButton->setEnabled(enabled);
}

void MainWindow::updateRollButtonState()
{
    bool hasValidBet = ui->doubleSpinBox2->value() > 0.0;
    ui->rollButton->setEnabled(hasValidBet);
}

void MainWindow::updatePlayButtonState()
{
    bool hasValidBet = ui->doubleSpinBox3->value() > 0.0;
    ui->playButton3->setEnabled(hasValidBet);
}

void MainWindow::on_halfButton3_clicked()
{
    double currentBet = ui->doubleSpinBox3->value();
    double halvedBet = std::round((currentBet / 2) * 100) / 100;
    ui->doubleSpinBox3->setValue(halvedBet);
}

void MainWindow::on_doubleButton3_clicked()
{
    double currentBet = ui->doubleSpinBox3->value();
    double doubledBet = currentBet * 2;
    double maxBet = static_cast<double>(playerBalance) / 100.;
    ui->doubleSpinBox3->setValue(std::min(doubledBet, maxBet));
}

void MainWindow::on_selectionChanged(bool selected)
{
    updateBetButtonState();
}

void MainWindow::rollDices()
{
    playerRoll = QRandomGenerator::global()->bounded(2, 21);
    casinoRoll = QRandomGenerator::global()->bounded(2, 21);
    ui->playerRollLabel->setText(QString::number(playerRoll));
    minigame2WaitingForGuess = true;
    setGuessButtonsEnabled(true);
}

void MainWindow::resetMinigame2()
{
    minigame2WaitingForGuess = false;
    setGuessButtonsEnabled(false);
    ui->playerRollLabel->setText("-");
    // ui->resultLabel->setText("Place your bet and roll the dice!");
    // ui->resultLabel->setStyleSheet("");
    updateRollButtonState();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete loginWindow;
    delete minigame1;
    delete minigame2;
    delete minigame3;
}
