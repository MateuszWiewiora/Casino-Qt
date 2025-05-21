#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QFile>
#include <QTimer>
#include <random>
#include <QDirIterator>
#include <ctime>

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

    // Set properties for labels to work well with background images
    option1Label->setAlignment(Qt::AlignCenter);
    option2Label->setAlignment(Qt::AlignCenter);

    // Set size policy to make labels resize with containers
    option1Label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    option2Label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Set minimum size to avoid collapse
    option1Label->setMinimumSize(100, 100);
    option2Label->setMinimumSize(100, 100);

    // Connect the click signals
    connect(option1Label, &ClickableLabel::clicked, this, &MainWindow::on_option1Label_clicked);
    connect(option2Label, &ClickableLabel::clicked, this, &MainWindow::on_option2Label_clicked);
    connect(option1Label, &ClickableLabel::selectionChanged, this, &MainWindow::on_selectionChanged);
    connect(option2Label, &ClickableLabel::selectionChanged, this, &MainWindow::on_selectionChanged);

    // Connect bet amount changes
    connect(ui->doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::on_betAmountChanged);

    // Replace the original labels in the layout
    QLayout *layout1 = ui->option1Label->parentWidget()->layout();
    QLayout *layout2 = ui->option2Label->parentWidget()->layout();

    if (layout1)
    {
        layout1->replaceWidget(ui->option1Label, option1Label);
        QString style1 = QString("QLabel { background-image: url(qrc:/assets/bombardiro_crocodillo.png); "
                                 "background-position: center; "
                                 "background-repeat: no-repeat; "
                                 "background-origin: content; "
                                 "background-clip: content; "
                                 "background-size: contain; "
                                 "border: %1 %2; }")
                             .arg(ClickableLabel::SELECTION_BORDER_STYLE)
                             .arg(ClickableLabel::UNSELECTED_BORDER_COLOR);
        option1Label->setStyleSheet(style1);
    }

    if (layout2)
    {
        layout2->replaceWidget(ui->option2Label, option2Label);
        QString style2 = QString("QLabel { background-image: url(qrc:/assets/tung_tung_tung_sahur.png); "
                                 "background-position: center; "
                                 "background-repeat: no-repeat; "
                                 "background-origin: content; "
                                 "background-clip: content; "
                                 "background-size: contain; "
                                 "border: %1 %2; }")
                             .arg(ClickableLabel::SELECTION_BORDER_STYLE)
                             .arg(ClickableLabel::UNSELECTED_BORDER_COLOR);
        option2Label->setStyleSheet(style2);
    }

    delete ui->option1Label;
    delete ui->option2Label;

    // Initially disable the bet button
    ui->betButton->setEnabled(false);

    // Set up bet amount limits
    updateBetAmountLimits();

    QStringList availableImages = getAvailableImages();
    if (availableImages.size() >= 2)
    {
        std::random_device randomDevice;
        std::mt19937 randomGenerator(randomDevice());
        std::shuffle(availableImages.begin(), availableImages.end(), randomGenerator);
        updateOptionDisplay(availableImages[0], availableImages[1]);
    }
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
}

void MainWindow::on_homeMinigame3_Button_clicked()
{
    ui->stackedWidget->setCurrentIndex(3);
}

void MainWindow::updateMoneyDisplay(double money)
{
    ui->cashLabel->setText("💰" + QString::number(money, 'f', 2) + "$");
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
    updateBetButtonState();
}

void MainWindow::updateBetButtonState()
{
    bool hasSelection = getSelectedOption() != 0;
    bool hasValidBet = ui->doubleSpinBox->value() > 0.0;
    ui->betButton->setEnabled(hasSelection && hasValidBet);
}

bool MainWindow::placeBet(double amount)
{
    if (amount <= 0 || amount > playerBalance)
    {
        return false;
    }

    if (rand() % 2)
    {
        playerBalance += amount * 0.99;
        playerBalance = std::floor(playerBalance * 100) / 100;
        showWinAnimation(getSelectedOption() == 1 ? option1Label : option2Label);
    }
    else
    {
        playerBalance -= amount;
        playerBalance = std::floor(playerBalance * 100) / 100;
        showLoseAnimation(getSelectedOption() == 1 ? option1Label : option2Label);
        showWinAnimation(getSelectedOption() == 1 ? option2Label : option1Label);
    }

    updateMoneyDisplay(playerBalance);
    updateBetAmountLimits();

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
    if (availableImages.size() >= 2)
    {
        std::random_device randomDevice;
        std::mt19937 randomGenerator(randomDevice());
        std::shuffle(availableImages.begin(), availableImages.end(), randomGenerator);

        updateOptionDisplay(availableImages[0], availableImages[1]);
    }

    updateBetButtonState();
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
    // Convert paths for CSS usage
    QString firstPathForCSS = firstImagePath;
    firstPathForCSS.replace(":/", "qrc:/");

    QString secondPathForCSS = secondImagePath;
    secondPathForCSS.replace(":/", "qrc:/");

    // Set background images via stylesheet
    QString style1 = QString("QLabel { background-image: url(%1); "
                             "background-position: center; "
                             "background-repeat: no-repeat; "
                             "background-origin: content; "
                             "background-clip: content; "
                             "background-size: contain; "
                             "border: %2 %3; }")
                         .arg(firstPathForCSS)
                         .arg(ClickableLabel::SELECTION_BORDER_STYLE)
                         .arg(ClickableLabel::UNSELECTED_BORDER_COLOR);

    QString style2 = QString("QLabel { background-image: url(%1); "
                             "background-position: center; "
                             "background-repeat: no-repeat; "
                             "background-origin: content; "
                             "background-clip: content; "
                             "background-size: contain; "
                             "border: %2 %3; }")
                         .arg(secondPathForCSS)
                         .arg(ClickableLabel::SELECTION_BORDER_STYLE)
                         .arg(ClickableLabel::UNSELECTED_BORDER_COLOR);

    option1Label->setStyleSheet(style1);
    option2Label->setStyleSheet(style2);

    // Clear the pixmap to ensure stylesheet is used
    option1Label->setPixmap(QPixmap());
    option2Label->setPixmap(QPixmap());

    // Update image names
    ui->option1NameLabel->setText(formatImageName(firstImagePath));
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
    ui->doubleSpinBox->setMaximum(playerBalance);
}

void MainWindow::on_doubleButton_clicked()
{
    double currentBet = ui->doubleSpinBox->value();
    double doubledBet = currentBet * 2;
    double maximumAllowedBet = playerBalance;

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

    // Get the current stylesheet and modify only the border color
    QString currentStyle = label->styleSheet();
    currentStyle.replace(ClickableLabel::UNSELECTED_BORDER_COLOR, "#00ff00");
    label->setStyleSheet(currentStyle);
}

void MainWindow::showLoseAnimation(ClickableLabel *label)
{
    losingLabel = label;

    // Get the current stylesheet and modify only the border color
    QString currentStyle = label->styleSheet();
    currentStyle.replace(ClickableLabel::UNSELECTED_BORDER_COLOR, "#ff0000");
    label->setStyleSheet(currentStyle);
}

void MainWindow::resetWinAnimation()
{
    if (winningLabel)
    {
        // Reset border color but keep the background image
        QString currentStyle = winningLabel->styleSheet();
        currentStyle.replace("#00ff00", ClickableLabel::UNSELECTED_BORDER_COLOR);
        winningLabel->setStyleSheet(currentStyle);
        winningLabel = nullptr;
    }
}

void MainWindow::resetLoseAnimation()
{
    if (losingLabel)
    {
        // Reset border color but keep the background image
        QString currentStyle = losingLabel->styleSheet();
        currentStyle.replace("#ff0000", ClickableLabel::UNSELECTED_BORDER_COLOR);
        losingLabel->setStyleSheet(currentStyle);
        losingLabel = nullptr;
    }
}

void MainWindow::on_workButton_clicked()
{
    playerBalance += 0.1;
    ui->cashLabel->setText("💰" + QString::number(playerBalance, 'f', 2) + "$");
}