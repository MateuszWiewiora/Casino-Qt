#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QFile>
#include <QTimer>
#include <random>
#include <QDirIterator>
#include <ctime>
#include <QRandomGenerator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initializeRandomSeed();
    updateMoneyDisplay(playerBalance);

    createAndSetupClickableLabels();
    setupBetAmountConnections();
    setupInitialGameImages();
    disableBettingUntilSelection();
    updateBetAmountLimits();
}

void MainWindow::initializeRandomSeed()
{
    srand(time(0));
}

void MainWindow::createAndSetupClickableLabels()
{
    option1Label = new ClickableLabel(this);
    option2Label = new ClickableLabel(this);

    option1Label->setGeometry(ui->option1Label->geometry());
    option2Label->setGeometry(ui->option2Label->geometry());

    connectLabelSignals();
    replaceLabelWidgets();
}

void MainWindow::connectLabelSignals()
{
    connect(option1Label, &ClickableLabel::clicked, this, [this]()
            {
        if (ANIMATION_MODE) return;
        toggleOptionSelection(option1Label); });
    connect(option2Label, &ClickableLabel::clicked, this, [this]()
            {
        if (ANIMATION_MODE) return;
        toggleOptionSelection(option2Label); });
    connect(option1Label, &ClickableLabel::selectionChanged, this, &MainWindow::on_selectionChanged);
    connect(option2Label, &ClickableLabel::selectionChanged, this, &MainWindow::on_selectionChanged);
}

void MainWindow::setupBetAmountConnections()
{
    connect(ui->doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::on_betAmountChanged);
    connect(ui->doubleSpinBox2, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::on_betAmountChanged);
}

void MainWindow::replaceLabelWidgets()
{
    QLayout *layout1 = ui->option1Label->parentWidget()->layout();
    QLayout *layout2 = ui->option2Label->parentWidget()->layout();

    if (layout1)
    {
        layout1->replaceWidget(ui->option1Label, option1Label);
        QPixmap pixmap(":/assets/bombardiro_crocodillo.png");
        option1Label->setAlignment(Qt::AlignCenter);
        option1Label->setPixmap(pixmap);
    }
    if (layout2)
    {
        layout2->replaceWidget(ui->option2Label, option2Label);
        QPixmap pixmap(":/assets/tung_tung_tung_sahur.png");
        option2Label->setAlignment(Qt::AlignCenter);
        option2Label->setPixmap(pixmap);
    }

    delete ui->option1Label;
    delete ui->option2Label;
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
    resetMinigame2();
    updateBetAmountLimits(); // Ensure bet limits are updated
    updateRollButtonState(); // Update roll button state
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
    resetMinigame2();
    updateBetAmountLimits(); // Ensure bet limits are updated
    updateRollButtonState(); // Update roll button state
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
    this->hide(); // ukryj MainWindow, nie zamykaj
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
    bool hasSelection = getSelectedOption() != 0;
    bool hasValidBet = ui->doubleSpinBox->value() > 0.0;
    ui->betButton->setEnabled(hasSelection && hasValidBet);
}

bool MainWindow::placeBet(double amount)
{
    long amountInCents = std::floor(amount * 100);

    if (!isValidBetAmount(amountInCents))
    {
        qDebug() << amountInCents << playerBalance;
        return false;
    }

    processGameOutcome(amountInCents);
    startResultAnimation();
    return true;
}

bool MainWindow::isValidBetAmount(long amountInCents)
{
    return amountInCents > 0 && amountInCents <= playerBalance;
}

void MainWindow::processGameOutcome(long amountInCents)
{
    bool playerWins = rand() % 2;
    if (playerWins)
    {
        handleWin(amountInCents);
    }
    else
    {
        handleLoss(amountInCents);
    }
    updateMoneyDisplay(playerBalance);
}

void MainWindow::handleWin(long amountInCents)
{
    playerBalance += amountInCents * 99 / 100;
    showWinAnimation(getSelectedOption() == 1 ? option1Label : option2Label);
}

void MainWindow::handleLoss(long amountInCents)
{
    playerBalance -= amountInCents;
    showLoseAnimation(getSelectedOption() == 1 ? option1Label : option2Label);
    showWinAnimation(getSelectedOption() == 1 ? option2Label : option1Label);
}

void MainWindow::startResultAnimation()
{
    ANIMATION_MODE = true;
    ui->betButton->setEnabled(false);

    initTimer = new QTimer(this);
    initTimer->setSingleShot(true);
    connect(initTimer, &QTimer::timeout, this, &MainWindow::animationFinished);
    initTimer->start(1000);
}

void MainWindow::animationFinished()
{
    ANIMATION_MODE = false;
    resetWinAnimation();
    resetLoseAnimation();
    deselectAll();
    displayRandomGameOptions();
}

void MainWindow::displayRandomGameOptions()
{
    QStringList availableImages = getAvailableImages();
    std::random_device randomDevice;
    std::mt19937 randomGenerator(randomDevice());
    std::shuffle(availableImages.begin(), availableImages.end(), randomGenerator);

    updateOptionDisplay(availableImages[0], availableImages[1]);
}

void MainWindow::on_betButton_clicked()
{
    int selectedOption = getSelectedOption();
    if (!selectedOption)
        return;

    double betAmount = ui->doubleSpinBox->value();
    if (!placeBet(betAmount))
        return;

    qDebug() << "Betting" << betAmount << "on option" << selectedOption;
}

QStringList MainWindow::getAvailableImages()
{
    QStringList images;
    QDirIterator imageIterator(":/assets", QDirIterator::Subdirectories);
    while (imageIterator.hasNext())
    {
        QString imagePath = imageIterator.next();
        if (imagePath.endsWith(".png"))
        {
            images.append(imagePath);
        }
    }
    return images;
}

void MainWindow::updateOptionDisplay(const QString &firstImagePath, const QString &secondImagePath)
{
    displayFirstOption(firstImagePath);
    displaySecondOption(secondImagePath);
}

void MainWindow::displayFirstOption(const QString &imagePath)
{
    QPixmap image(imagePath);
    option1Label->setPixmap(image);
    ui->option1NameLabel->setText(formatImageName(imagePath));
}

void MainWindow::displaySecondOption(const QString &imagePath)
{
    QPixmap image(imagePath);
    option2Label->setPixmap(image);
    ui->option2NameLabel->setText(formatImageName(imagePath));
}

QString MainWindow::formatImageName(const QString &imagePath)
{
    return imagePath.split("/").last().split(".").first().replace("_", " ").toUpper();
}

void MainWindow::toggleOptionSelection(ClickableLabel *selectedLabel)
{
    if (ANIMATION_MODE)
        return;

    if (selectedLabel->isSelected())
    {
        selectedLabel->setSelected(false);
    }
    else
    {
        deselectAll();
        selectedLabel->setSelected(true);
    }
}

void MainWindow::on_selectionChanged(bool selected)
{
    updateBetButtonState();
}

void MainWindow::deselectAll()
{
    option1Label->setSelected(false);
    option2Label->setSelected(false);
}

int MainWindow::getSelectedOption() const
{
    if (option1Label->isSelected())
        return 1;
    if (option2Label->isSelected())
        return 2;
    return 0;
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

void MainWindow::showWinAnimation(ClickableLabel *label)
{
    winningLabel = label;
    label->setStyleSheet(QString("QLabel { border: %1 %2; }").arg(ClickableLabel::SELECTION_BORDER_STYLE).arg("#00ff00"));
}

void MainWindow::showLoseAnimation(ClickableLabel *label)
{
    losingLabel = label;
    label->setStyleSheet(QString("QLabel { border: %1 %2; }").arg(ClickableLabel::SELECTION_BORDER_STYLE).arg("#ff0000"));
}

void MainWindow::resetWinAnimation()
{
    if (winningLabel)
    {
        winningLabel->setStyleSheet(QString("QLabel { border: %1 %2; }").arg(ClickableLabel::SELECTION_BORDER_STYLE).arg(ClickableLabel::UNSELECTED_BORDER_COLOR));
        winningLabel = nullptr;
    }
}

void MainWindow::resetLoseAnimation()
{
    if (losingLabel)
    {
        losingLabel->setStyleSheet(QString("QLabel { border: %1 %2; }").arg(ClickableLabel::SELECTION_BORDER_STYLE).arg(ClickableLabel::UNSELECTED_BORDER_COLOR));
        losingLabel = nullptr;
    }
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

void MainWindow::rollDices()
{
    playerRoll = generateDiceRoll();
    casinoRoll = generateDiceRoll();
    displayPlayerRoll();
    logRollResults();
}

int MainWindow::generateDiceRoll()
{
    return QRandomGenerator::global()->bounded(1, 11) + QRandomGenerator::global()->bounded(1, 11);
}

void MainWindow::displayPlayerRoll()
{
    ui->playerRollLabel->setText("Your roll: " + QString::number(playerRoll));
}

void MainWindow::logRollResults()
{
    qDebug() << "Player rolled:" << playerRoll << "Casino rolled:" << casinoRoll;
}

void MainWindow::on_rollButton_clicked()
{
    double betAmount = ui->doubleSpinBox2->value();
    if (!isValidBetAmount(betAmount))
    {
        logInvalidBet();
        return;
    }

    if (!startDiceGame(betAmount))
    {
        logGameStartFailure();
        return;
    }

    logGameStart(betAmount);
}

bool MainWindow::startDiceGame(double amount)
{
    long amountInCents = std::floor(amount * 100);
    if (!isValidBetAmount(amountInCents))
        return false;

    initializeGameRound(amountInCents);
    return true;
}

void MainWindow::initializeGameRound(long amountInCents)
{
    currentDiceBet = amountInCents;
    rollDices();
    promptUserForGuess();
    enableGuessButtons();
    disableRollButton();
    minigame2WaitingForGuess = true;
}

void MainWindow::promptUserForGuess()
{
    ui->resultLabel->setText("Make your guess: Will casino roll LESS, SAME, or MORE than you?");
}

void MainWindow::enableGuessButtons()
{
    ui->moreButton->setEnabled(true);
    ui->sameButton->setEnabled(true);
    ui->lessButton->setEnabled(true);
}

void MainWindow::disableRollButton()
{
    ui->rollButton->setEnabled(false);
}

void MainWindow::updateRollButtonState()
{
    if (ui->stackedWidget->currentIndex() == 2 && !minigame2WaitingForGuess)
    {
        bool hasValidBet = ui->doubleSpinBox2->value() > 0.0;
        ui->rollButton->setEnabled(hasValidBet);
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

void MainWindow::checkAnswer(QString answer)
{
    if (!minigame2WaitingForGuess)
        return;

    disableGuessButtons();
    minigame2WaitingForGuess = false;

    bool isCorrectGuess = validateGuess(answer);
    QString resultMessage = buildResultMessage(isCorrectGuess);
    updateGameState(isCorrectGuess);
    displayResult(resultMessage);
    scheduleGameReset();
}

void MainWindow::disableGuessButtons()
{
    ui->moreButton->setEnabled(false);
    ui->sameButton->setEnabled(false);
    ui->lessButton->setEnabled(false);
}

bool MainWindow::validateGuess(QString answer)
{
    if (answer == "More" && casinoRoll > playerRoll)
        return true;
    if (answer == "Same" && casinoRoll == playerRoll)
        return true;
    if (answer == "Less" && casinoRoll < playerRoll)
        return true;
    return false;
}

QString MainWindow::buildResultMessage(bool isCorrectGuess)
{
    QString resultText = "Casino rolled: " + QString::number(casinoRoll) + ". ";

    if (isCorrectGuess)
    {
        resultText += "CORRECT! You won $" + QString::number(currentDiceBet / 100.0, 'f', 2) + "!";
    }
    else
    {
        resultText += "WRONG! You lost $" + QString::number(currentDiceBet / 100.0, 'f', 2) + "!";
    }
    return resultText;
}

void MainWindow::updateGameState(bool isCorrectGuess)
{
    if (isCorrectGuess)
    {
        playerBalance += currentDiceBet;
    }
    else
    {
        playerBalance -= currentDiceBet;
    }
    updateMoneyDisplay(playerBalance);
}

void MainWindow::displayResult(const QString &resultMessage)
{
    ui->resultLabel->setText(resultMessage);
}

void MainWindow::scheduleGameReset()
{
    QTimer::singleShot(3000, this, &MainWindow::resetMinigame2);
}

void MainWindow::on_moreButton_clicked()
{
    checkAnswer("More");
}

void MainWindow::on_sameButton_clicked()
{
    checkAnswer("Same");
}

void MainWindow::on_lessButton_clicked()
{
    checkAnswer("Less");
}

void MainWindow::resetMinigame2()
{
    resetGameState();
    resetUIElements();
    updateGameControls();
}

void MainWindow::resetGameState()
{
    playerRoll = 0;
    casinoRoll = 0;
    currentDiceBet = 0;
    minigame2WaitingForGuess = false;
}

void MainWindow::resetUIElements()
{
    ui->playerRollLabel->setText("Click ROLL to play!");
    ui->resultLabel->setText("Place your bet and roll the dice!");
}

void MainWindow::updateGameControls()
{
    ui->moreButton->setEnabled(false);
    ui->sameButton->setEnabled(false);
    ui->lessButton->setEnabled(false);
    ui->rollButton->setEnabled(true);
    updateMoneyDisplay(playerBalance);
    updateRollButtonState();
}

void MainWindow::logInvalidBet()
{
    qDebug() << "Invalid bet amount";
}

void MainWindow::logGameStartFailure()
{
    qDebug() << "Failed to start dice game";
}

void MainWindow::logGameStart(double betAmount)
{
    qDebug() << "Starting dice game with bet:" << betAmount;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete initTimer;
    delete loginWindow;
    delete option1Label;
    delete option2Label;
}

void MainWindow::setupInitialGameImages()
{
    QStringList availableImages = getAvailableImages();
    if (availableImages.size() >= 2)
    {
        updateOptionDisplay(availableImages[0], availableImages[1]);
    }
}

bool MainWindow::isValidBetAmount(double amount)
{
    long amountInCents = std::floor(amount * 100);
    return amountInCents > 0 && amountInCents <= playerBalance;
}