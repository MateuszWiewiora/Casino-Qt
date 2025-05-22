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
    srand(time(0));

    updateMoneyDisplay(playerBalance);

    // Replace the QLabels with our custom ClickableLabels
    option1Label = new ClickableLabel(this);
    option2Label = new ClickableLabel(this);

    // Copy properties from the original labels
    option1Label->setGeometry(ui->option1Label->geometry());
    option2Label->setGeometry(ui->option2Label->geometry());

    // Connect the click signals
    connect(option1Label, &ClickableLabel::clicked, this, &MainWindow::on_option1Label_clicked);
    connect(option2Label, &ClickableLabel::clicked, this, &MainWindow::on_option2Label_clicked);
    connect(option1Label, &ClickableLabel::selectionChanged, this, &MainWindow::on_selectionChanged);
    connect(option2Label, &ClickableLabel::selectionChanged, this, &MainWindow::on_selectionChanged);

    // Connect bet amount changes
    connect(ui->doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        this, &MainWindow::on_betAmountChanged);
    connect(ui->doubleSpinBox2, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        this, &MainWindow::on_betAmountChanged);

    // Replace the original labels in the layout
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

    // Initially disable the bet button
    ui->betButton->setEnabled(false);

    // Set up bet amount limits
    updateBetAmountLimits();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete initTimer;
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
    if (ui->stackedWidget->currentIndex() == 1) {
        // Minigame 1
        updateBetButtonState();
    } else if (ui->stackedWidget->currentIndex() == 2) {
        // Minigame 2  
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
    long amountLong = std::floor(amount * 100);

    if (amountLong <= 0 || amountLong > playerBalance)
    {
        qDebug() << amountLong << playerBalance;
        return false;
    }

    if (rand() % 2)
    {
        playerBalance += amountLong * 99 / 100;
        showWinAnimation(getSelectedOption() == 1 ? option1Label : option2Label);
    }
    else
    {
        playerBalance -= amountLong;
        showLoseAnimation(getSelectedOption() == 1 ? option1Label : option2Label);
        showWinAnimation(getSelectedOption() == 1 ? option2Label : option1Label);
    }

    updateMoneyDisplay(playerBalance);

    ANIMATION_MODE = true;
    ui->betButton->setEnabled(false);

    initTimer = new QTimer(this);
    initTimer->setSingleShot(true);
    connect(initTimer, &QTimer::timeout, this, &MainWindow::animationFinished);
    initTimer->start(1000);
    return true;
}

void MainWindow::animationFinished()
{
    ANIMATION_MODE = false;
    resetWinAnimation();
    resetLoseAnimation();

    deselectAll();

    QStringList availableImages = getAvailableImages();
    std::random_device randomDevice;
    std::mt19937 randomGenerator(randomDevice());
    std::shuffle(availableImages.begin(), availableImages.end(), randomGenerator);

    updateOptionDisplay(availableImages[0], availableImages[1]);
}

void MainWindow::on_betButton_clicked()
{
    int selectedOption = getSelectedOption();
    if (selectedOption == 0)
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
    // TODO: make it scale properly somehow

    QPixmap firstImage(firstImagePath);
    option1Label->setPixmap(firstImage);
    ui->option1NameLabel->setText(formatImageName(firstImagePath));

    QPixmap secondImage(secondImagePath);
    option2Label->setPixmap(secondImage);
    ui->option2NameLabel->setText(formatImageName(secondImagePath));
}

QString MainWindow::formatImageName(const QString &imagePath)
{
    return imagePath.split("/").last().split(".").first().replace("_", " ").toUpper();
}

void MainWindow::on_option1Label_clicked()
{
    if (ANIMATION_MODE)
        return;
    toggleOptionSelection(option1Label);
}

void MainWindow::on_option2Label_clicked()
{
    if (ANIMATION_MODE)
        return;
    toggleOptionSelection(option2Label);
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
    ui->doubleSpinBox->setMinimum(0.0);
    ui->doubleSpinBox->setMaximum(playerBalance / 100.);
    ui->doubleSpinBox2->setMinimum(0.0);  // Add this
    ui->doubleSpinBox2->setMaximum(playerBalance / 100.);
}

void MainWindow::on_doubleButton_clicked()
{
    double currentBet = ui->doubleSpinBox->value();
    double doubledBet = currentBet * 2;
    double maximumAllowedBet = static_cast<double>(playerBalance) / 100.;

    ui->doubleSpinBox->setValue(std::min(doubledBet, maximumAllowedBet));
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
    playerBalance += 10;
    updateMoneyDisplay(playerBalance);
}

void MainWindow::rollDices() {  
    playerRoll = QRandomGenerator::global()->bounded(1, 11) + QRandomGenerator::global()->bounded(1, 11);  
    casinoRoll = QRandomGenerator::global()->bounded(1, 11) + QRandomGenerator::global()->bounded(1, 11);  
    ui->playerRollLabel->setText("Your roll: " + QString::number(playerRoll));
    qDebug() << "Player rolled:" << playerRoll << "Casino rolled:" << casinoRoll;
}  

void MainWindow::checkAnswer(QString answer) {  
    if (!minigame2WaitingForGuess) {
        return;
    }
    
    // Disable buttons to prevent multiple clicks
    ui->moreButton->setEnabled(false);
    ui->sameButton->setEnabled(false);
    ui->lessButton->setEnabled(false);
    
    minigame2WaitingForGuess = false;
    
    bool correct = false;  
    if (answer == "More" && casinoRoll > playerRoll) correct = true;  
    if (answer == "Same" && casinoRoll == playerRoll) correct = true;  
    if (answer == "Less" && casinoRoll < playerRoll) correct = true;  
  
    QString resultText = "Casino rolled: " + QString::number(casinoRoll) + ". ";
    
    if (correct) {  
        // Win: get back bet + win the same amount (x2 multiplier)
        playerBalance += currentDiceBet; // Double the bet amount
        resultText += "CORRECT! You won $" + QString::number(currentDiceBet / 100.0, 'f', 2) + "!";
    } else {  
        // Lose: lose the bet amount
        playerBalance -= currentDiceBet;
        resultText += "WRONG! You lost $" + QString::number(currentDiceBet / 100.0, 'f', 2) + "!";
    }  
    
    ui->resultLabel->setText(resultText);
    updateMoneyDisplay(playerBalance);
    
    // Set up timer to reset the game after 3 seconds
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

void MainWindow::resetMinigame2() {
    // Reset the game state
    playerRoll = 0;
    casinoRoll = 0;
    currentDiceBet = 0;
    minigame2WaitingForGuess = false;
    
    // Reset UI elements
    ui->playerRollLabel->setText("Click ROLL to play!");
    ui->resultLabel->setText("Place your bet and roll the dice!");
    
    // Disable guess buttons initially
    ui->moreButton->setEnabled(false);
    ui->sameButton->setEnabled(false);
    ui->lessButton->setEnabled(false);
    
    // Enable roll button
    ui->rollButton->setEnabled(true);
    
    // Update money display and bet limits
    updateMoneyDisplay(playerBalance);
    updateRollButtonState(); // Update roll button state based on bet amount
}

void MainWindow::on_rollButton_clicked()
{
    double betAmount = ui->doubleSpinBox2->value();
    if (betAmount <= 0.0) {
        qDebug() << "Invalid bet amount";
        return;
    }
    
    if (!startDiceGame(betAmount)) {
        qDebug() << "Failed to start dice game";
        return;
    }
    
    qDebug() << "Starting dice game with bet:" << betAmount;
}

bool MainWindow::startDiceGame(double amount) {
    long amountLong = std::floor(amount * 100);

    if (amountLong <= 0 || amountLong > playerBalance) {
        qDebug() << "Invalid bet amount:" << amountLong << "Balance:" << playerBalance;
        return false;
    }

    // Store the bet amount for later use
    currentDiceBet = amountLong;
    
    // Roll the dice
    rollDices();
    
    // Update UI
    ui->resultLabel->setText("Make your guess: Will casino roll MORE, SAME, or LESS than you?");
    
    // Enable guess buttons
    ui->moreButton->setEnabled(true);
    ui->sameButton->setEnabled(true);
    ui->lessButton->setEnabled(true);
    
    // Disable bet button during the game
    ui->rollButton->setEnabled(false);
    
    minigame2WaitingForGuess = true;
    
    return true;
}

void MainWindow::updateRollButtonState() {
    // Only update if we're on minigame 2 and not waiting for a guess
    if (ui->stackedWidget->currentIndex() == 2 && !minigame2WaitingForGuess) {
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
    double maximumAllowedBet = static_cast<double>(playerBalance) / 100.;
    ui->doubleSpinBox2->setValue(std::min(doubledBet, maximumAllowedBet));
}