#include "minigame1.h"
#include "mainwindow.h"
#include <QDirIterator>
#include <random>
#include "../main.h"

Minigame1::Minigame1(QWidget *parent, long &playerBalanceRef)
    : QObject(parent), playerBalance(playerBalanceRef)
{
}

void Minigame1::createAndSetupClickableLabels(QWidget *parent, Ui::MainWindow *ui)
{
    option1Label = new ClickableLabel(parent);
    option2Label = new ClickableLabel(parent);

    option1Label->setGeometry(ui->option1Label->geometry());
    option2Label->setGeometry(ui->option2Label->geometry());

    connectLabelSignals();
    replaceLabelWidgets(ui);
}

void Minigame1::connectLabelSignals()
{
    connect(option1Label, &ClickableLabel::clicked, this, [this]()
            {
        if (ANIMATION_MODE) return;
        toggleOptionSelection(option1Label); });
    connect(option2Label, &ClickableLabel::clicked, this, [this]()
            {
        if (ANIMATION_MODE) return;
        toggleOptionSelection(option2Label); });
    connect(option1Label, &ClickableLabel::selectionChanged, this, &Minigame1::onSelectionChanged);
    connect(option2Label, &ClickableLabel::selectionChanged, this, &Minigame1::onSelectionChanged);
}

void Minigame1::replaceLabelWidgets(Ui::MainWindow *ui)
{
    QLayout *layout1 = ui->option1Label->parentWidget()->layout();
    QLayout *layout2 = ui->option2Label->parentWidget()->layout();

    if (layout1)
    {
        layout1->replaceWidget(ui->option1Label, option1Label);
        QPixmap pixmap = getPixmap(":/assets/bombardiro_crocodillo.png");
        option1Label->setAlignment(Qt::AlignCenter);
        option1Label->setPixmap(pixmap);
    }
    if (layout2)
    {
        layout2->replaceWidget(ui->option2Label, option2Label);
        QPixmap pixmap = getPixmap(":/assets/tung_tung_tung_sahur.png");
        option2Label->setAlignment(Qt::AlignCenter);
        option2Label->setPixmap(pixmap);
    }

    delete ui->option1Label;
    delete ui->option2Label;
}

bool Minigame1::placeBet(double amount)
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

bool Minigame1::isValidBetAmount(long amountInCents)
{
    return amountInCents > 0 && amountInCents <= playerBalance;
}

void Minigame1::processGameOutcome(long amountInCents)
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
    emit moneyUpdated(playerBalance);
}

void Minigame1::handleWin(long amountInCents)
{
    playerBalance += amountInCents * (1 - MainWindow::CASINO_EDGE);
    showWinAnimation(getSelectedOption() == 1 ? option1Label : option2Label);
}

void Minigame1::handleLoss(long amountInCents)
{
    playerBalance -= amountInCents;
    showLoseAnimation(getSelectedOption() == 1 ? option1Label : option2Label);
    showWinAnimation(getSelectedOption() == 1 ? option2Label : option1Label);
}

void Minigame1::startResultAnimation()
{
    ANIMATION_MODE = true;
    emit betButtonEnabled(false);

    initTimer = new QTimer(qobject_cast<QWidget *>(parent()));
    initTimer->setSingleShot(true);
    connect(initTimer, &QTimer::timeout, this, &Minigame1::animationFinished);
    initTimer->start(1000);
}

void Minigame1::animationFinished()
{
    ANIMATION_MODE = false;
    resetWinAnimation();
    resetLoseAnimation();
    deselectAll();
    displayRandomGameOptions();
}

void Minigame1::displayRandomGameOptions()
{
    QStringList availableImages = getAvailableImages();
    std::random_device randomDevice;
    std::mt19937 randomGenerator(randomDevice());
    std::shuffle(availableImages.begin(), availableImages.end(), randomGenerator);

    updateOptionDisplay(availableImages[0], availableImages[1]);
}

QStringList Minigame1::getAvailableImages()
{
    QStringList images;
    QDirIterator imageIterator = getImageIterator();
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

void Minigame1::updateOptionDisplay(const QString &firstImagePath, const QString &secondImagePath)
{
    displayFirstOption(firstImagePath);
    displaySecondOption(secondImagePath);
}

void Minigame1::displayFirstOption(const QString &imagePath)
{
    QPixmap image(imagePath);
    option1Label->setPixmap(image);
    emit option1NameUpdated(formatImageName(imagePath));
}

void Minigame1::displaySecondOption(const QString &imagePath)
{
    QPixmap image(imagePath);
    option2Label->setPixmap(image);
    emit option2NameUpdated(formatImageName(imagePath));
}

QString Minigame1::formatImageName(const QString &imagePath)
{
    return imagePath.split("/").last().split(".").first().replace("_", " ").toUpper();
}

void Minigame1::toggleOptionSelection(ClickableLabel *selectedLabel)
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

void Minigame1::onSelectionChanged(bool selected)
{
    bool hasSelection = getSelectedOption() != 0;
    emit selectionChanged(hasSelection);
}

void Minigame1::deselectAll()
{
    option1Label->setSelected(false);
    option2Label->setSelected(false);
}

int Minigame1::getSelectedOption() const
{
    if (option1Label->isSelected())
        return 1;
    if (option2Label->isSelected())
        return 2;
    return 0;
}

void Minigame1::showWinAnimation(ClickableLabel *label)
{
    winningLabel = label;
    label->setStyleSheet(QString("QLabel { border: %1 %2; }").arg(ClickableLabel::SELECTION_BORDER_STYLE).arg("#00ff00"));
}

void Minigame1::showLoseAnimation(ClickableLabel *label)
{
    losingLabel = label;
    label->setStyleSheet(QString("QLabel { border: %1 %2; }").arg(ClickableLabel::SELECTION_BORDER_STYLE).arg("#ff0000"));
}

void Minigame1::resetWinAnimation()
{
    if (winningLabel)
    {
        winningLabel->setStyleSheet(QString("QLabel { border: %1 %2; }").arg(ClickableLabel::SELECTION_BORDER_STYLE).arg(ClickableLabel::UNSELECTED_BORDER_COLOR));
        winningLabel = nullptr;
    }
}

void Minigame1::resetLoseAnimation()
{
    if (losingLabel)
    {
        losingLabel->setStyleSheet(QString("QLabel { border: %1 %2; }").arg(ClickableLabel::SELECTION_BORDER_STYLE).arg(ClickableLabel::UNSELECTED_BORDER_COLOR));
        losingLabel = nullptr;
    }
}

void Minigame1::setupInitialGameImages()
{
    QStringList availableImages = getAvailableImages();
    if (availableImages.size() >= 2)
    {
        updateOptionDisplay(availableImages[0], availableImages[1]);
    }
}

Minigame1::~Minigame1()
{
    delete initTimer;
    delete option1Label;
    delete option2Label;
}