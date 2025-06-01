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
}

void MainWindow::initializeRandomSeed()
{
    srand(time(0));
}

void MainWindow::setupMinigames()
{
    minigame1 = new Minigame1(this, playerBalance);
    minigame2 = new Minigame2(this, playerBalance);

    setupMinigame1Connections();
    setupMinigame2Connections();

    minigame1->createAndSetupClickableLabels(this, ui);
    minigame1->setupInitialGameImages();
}

void MainWindow::setupMinigame1Connections()
{
    connect(minigame1, &Minigame1::moneyUpdated, this, &MainWindow::updateMoneyDisplay);
    connect(minigame1, &Minigame1::betButtonEnabled, ui->betButton, &QPushButton::setEnabled);
    connect(minigame1, &Minigame1::selectionChanged, this, &MainWindow::updateBetButtonState);
    connect(minigame1, &Minigame1::option1NameUpdated, ui->option1NameLabel, &QLabel::setText);
    connect(minigame1, &Minigame1::option2NameUpdated, ui->option2NameLabel, &QLabel::setText);
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
    loginWindow = new LoginWindow();
    connect(loginWindow, &LoginWindow::returnToMain, this, &MainWindow::show);
    connect(loginWindow, &LoginWindow::returnToMain, loginWindow, &QWidget::close);

    loginWindow->show();
    this->hide();
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
}
