#include "minigame3.h"
#include "mainwindow.h"
#include <QRandomGenerator>
#include <numeric>
#include <algorithm>
#include <QDebug>

Minigame3::Minigame3(QWidget *parent, Ui::MainWindow *uiRef, long &playerBalanceRef)
    : QObject(parent), playerBalance(playerBalanceRef), ui(uiRef)
{
    createAndSetupClickableLabels(ui->minigame3Widget);
    presentContentsInternal.resize(TOTAL_PRESENTS);
    revealedPresentsMask.resize(TOTAL_PRESENTS);

    bombMultipliers[1] = 1.249 * (1 - MainWindow::CASINO_EDGE);
    bombMultipliers[3] = 2.017 * (1 - MainWindow::CASINO_EDGE);
    bombMultipliers[5] = 3.426 * (1 - MainWindow::CASINO_EDGE);
    bombMultipliers[10] = 17.69 * (1 - MainWindow::CASINO_EDGE);

    connectPresentSignals();
    connectBombSelectionSignals();

    revealAllEndTimer = new QTimer(this);
    revealAllEndTimer->setSingleShot(true);
    connect(revealAllEndTimer, &QTimer::timeout, this, &Minigame3::finalizeRound);

    setBombCount(3);
    initializeNewRound();
}

Minigame3::~Minigame3() {}

void Minigame3::createAndSetupClickableLabels(QWidget *parentWidget)
{
    for (int i = 1; i <= TOTAL_PRESENTS; ++i)
    {
        QString labelName = QString("Minigame3Label%1").arg(i);
        QLabel *oldLabel = parentWidget->findChild<QLabel *>(labelName);
        if (oldLabel)
        {
            ClickableLabel *newLabel = new ClickableLabel(parentWidget);
            newLabel->setObjectName(labelName + "_clickable");
            newLabel->setGeometry(oldLabel->geometry());
            newLabel->setAlignment(Qt::AlignCenter);
            QLayout *parentLayout = oldLabel->parentWidget()->layout();
            if (parentLayout)
                parentLayout->replaceWidget(oldLabel, newLabel);
            else
            {
                newLabel->setParent(oldLabel->parentWidget());
                newLabel->show();
            }
            presentLabels.append(newLabel);
            delete oldLabel;
        }
        else
        {
            qWarning() << "Minigame3: Could not find QLabel:" << labelName;
        }
    }
}

void Minigame3::connectPresentSignals()
{
    for (ClickableLabel *label : presentLabels)
    {
        connect(label, &ClickableLabel::clicked, this, [this, label]()
                { handlePresentClicked(label); });
    }
}

void Minigame3::connectBombSelectionSignals()
{
    connect(ui->bombButton1, &QPushButton::clicked, this, [this]()
            { setBombCount(1); });
    connect(ui->bombButton3, &QPushButton::clicked, this, [this]()
            { setBombCount(3); });
    connect(ui->bombButton5, &QPushButton::clicked, this, [this]()
            { setBombCount(5); });
    connect(ui->bombButton10, &QPushButton::clicked, this, [this]()
            { setBombCount(10); });
}

void Minigame3::setBombCount(int bombs)
{
    if (isBetActive)
        return;
    currentNumberOfBombs = bombs;
    qDebug() << "Minigame3: Number of bombs set to:" << currentNumberOfBombs;
    updateBombButtonsUI();
    initializeNewRound();
}

void Minigame3::updateBombButtonsUI()
{
    ui->bombButton1->setChecked(currentNumberOfBombs == 1);
    ui->bombButton3->setChecked(currentNumberOfBombs == 3);
    ui->bombButton5->setChecked(currentNumberOfBombs == 5);
    ui->bombButton10->setChecked(currentNumberOfBombs == 10);
}

void Minigame3::adjustLabelFont(ClickableLabel *label)
{
    QFont font = label->font();
    int pointSize = static_cast<int>(label->height() * 0.60);
    if (pointSize < 8)
        pointSize = 8;
    font.setPointSize(pointSize);
    label->setFont(font);
}

void Minigame3::updatePresentLabelAppearance(ClickableLabel *label, PresentContent contentType,
                                             bool isInitialSelection, bool isRevealed,
                                             bool isWinningHit, bool isLosingHit)
{
    if (!label)
        return;
    QString textToShow = GIFT_EMOJI;
    QString borderColor = ClickableLabel::UNSELECTED_BORDER_COLOR;

    if (isRevealed)
    {
        if (contentType == PresentContent::STAR)
            textToShow = STAR_EMOJI;
        else if (contentType == PresentContent::BOMB)
            textToShow = BOMB_EMOJI;
    }

    if (isLosingHit)
        borderColor = ClickableLabel::LOSE_BORDER_COLOR;
    else if (isWinningHit)
        borderColor = ClickableLabel::WIN_BORDER_COLOR;
    else if (isInitialSelection && !isBetActive)
        borderColor = ClickableLabel::SELECTED_BORDER_COLOR;

    label->setText(textToShow);
    adjustLabelFont(label);
    label->setStyleSheet(QString("QLabel { border: %1 %2; }")
                             .arg(ClickableLabel::SELECTION_BORDER_STYLE)
                             .arg(borderColor));
}

void Minigame3::updateAllLabelsToGift()
{
    for (ClickableLabel *label : presentLabels)
    {
        updatePresentLabelAppearance(label, PresentContent::HIDDEN, false, false, false, false);
    }
}

void Minigame3::initializeNewRound()
{
    if (isBetActive)
        return;

    shuffleAndAssignContents();
    revealedPresentsMask.fill(false);
    starsFoundCount = 0;
    currentBetAmountInCents = 0;
    selectedPresentForBet = nullptr;

    for (ClickableLabel *label : presentLabels)
    {
        updatePresentLabelAppearance(label, PresentContent::HIDDEN, false, false, false, false);
    }

    ui->playButton3->setText("PLAY");
    int starsAvailable = TOTAL_PRESENTS - currentNumberOfBombs;
    if (starsAvailable < TARGET_STARS_TO_WIN)
    {
        emit gameStatusUpdated(QString("Warning: Not enough stars (%1) to win! Need %2. Choose fewer bombs.")
                                   .arg(starsAvailable)
                                   .arg(TARGET_STARS_TO_WIN));
        emit playButtonEnabled(false);
    }
    else
    {
        emit gameStatusUpdated(QString("Select a present. Find %1 Stars. Bombs: %2").arg(TARGET_STARS_TO_WIN).arg(currentNumberOfBombs));
        emit playButtonEnabled(false);
    }
}

void Minigame3::shuffleAndAssignContents()
{
    presentContentsInternal.fill(PresentContent::STAR);
    int bombsToPlace = currentNumberOfBombs;
    if (bombsToPlace > TOTAL_PRESENTS)
        bombsToPlace = TOTAL_PRESENTS;
    if (bombsToPlace < 0)
        bombsToPlace = 0;

    QVector<int> indices(TOTAL_PRESENTS);
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), *QRandomGenerator::global());

    for (int i = 0; i < bombsToPlace; ++i)
    {
        presentContentsInternal[indices[i]] = PresentContent::BOMB;
    }
}

void Minigame3::handlePresentClicked(ClickableLabel *clickedLabel)
{
    if (revealAllEndTimer && revealAllEndTimer->isActive())
    {
        qDebug() << "Minigame3: Interaction blocked, round ending.";
        return;
    }

    int clickedIndex = presentLabels.indexOf(clickedLabel);
    if (clickedIndex == -1)
        return;

    if (!isBetActive)
    {
        if (selectedPresentForBet)
        {
            updatePresentLabelAppearance(selectedPresentForBet, PresentContent::HIDDEN, false, false, false, false);
        }
        selectedPresentForBet = clickedLabel;
        updatePresentLabelAppearance(selectedPresentForBet, PresentContent::HIDDEN, true, false, false, false);
        int starsAvailable = TOTAL_PRESENTS - currentNumberOfBombs;
        emit playButtonEnabled(starsAvailable >= TARGET_STARS_TO_WIN);
    }
    else
    {
        if (revealedPresentsMask[clickedIndex])
        {
            qDebug() << "Minigame3: Present already revealed.";
            return;
        }
        revealAndUpdate(clickedLabel);
    }
}

void Minigame3::confirmBetAndStartGame()
{
    if (isBetActive || !selectedPresentForBet)
        return;

    long amountInCents = static_cast<long>(std::floor(ui->doubleSpinBox3->value() * 100));
    if (!isValidBetAmount(amountInCents))
    {
        emit gameStatusUpdated("Invalid bet or insufficient funds.");
        return;
    }
    int starsAvailable = TOTAL_PRESENTS - currentNumberOfBombs;
    if (starsAvailable < TARGET_STARS_TO_WIN)
    {
        emit gameStatusUpdated(QString("Cannot start: Not enough stars (%1) to win! Need %2.").arg(starsAvailable).arg(TARGET_STARS_TO_WIN));
        emit playButtonEnabled(false);
        return;
    }

    currentBetAmountInCents = amountInCents;
    isBetActive = true;

    emit playButtonEnabled(false);
    ui->playButton3->setText("Picking...");
    ui->bombButton1->setEnabled(false);
    ui->bombButton3->setEnabled(false);
    ui->bombButton5->setEnabled(false);
    ui->bombButton10->setEnabled(false);

    emit gameStatusUpdated(QString("Game started! Find %1 stars. Good luck!").arg(TARGET_STARS_TO_WIN));
    revealAndUpdate(selectedPresentForBet);
}

void Minigame3::revealAndUpdate(ClickableLabel *revealedLabel)
{
    if (!isBetActive)
        return;

    int revealedIndex = presentLabels.indexOf(revealedLabel);
    if (revealedIndex == -1 || revealedPresentsMask[revealedIndex])
        return;

    revealedPresentsMask[revealedIndex] = true;
    PresentContent content = presentContentsInternal[revealedIndex];

    if (content == PresentContent::BOMB)
    {
        updatePresentLabelAppearance(revealedLabel, content, false, true, false, true);
        playerLoses();
        triggerDelayedRevealAll(false);
    }
    else if (content == PresentContent::STAR)
    {
        starsFoundCount++;
        updatePresentLabelAppearance(revealedLabel, content, false, true, true, false);

        if (starsFoundCount >= TARGET_STARS_TO_WIN)
        {
            playerWins();
            triggerDelayedRevealAll(true);
            emit gameStatusUpdated(QString("YOU WON!!! Found %1 stars!").arg(starsFoundCount));
        }
        else
        {
            emit gameStatusUpdated(QString("Star! %1/%2 found. Keep going!").arg(starsFoundCount).arg(TARGET_STARS_TO_WIN));
        }
    }
}

void Minigame3::playerLoses()
{
    playerBalance -= currentBetAmountInCents;
    emit moneyUpdated(playerBalance);
    emit gameStatusUpdated(QString("BOOM! You hit a bomb. You lost %1.")
                               .arg(currentBetAmountInCents / 100.0, 0, 'f', 2));
}

void Minigame3::playerWins()
{
    double multiplier = bombMultipliers.value(currentNumberOfBombs, 1.0);
    long winAmount = static_cast<long>(currentBetAmountInCents * multiplier * (1.0 - MainWindow::CASINO_EDGE));
    playerBalance += winAmount;
    emit moneyUpdated(playerBalance);
    emit gameStatusUpdated(QString("CONGRATULATIONS! You found all %1 stars and won %2!")
                               .arg(TARGET_STARS_TO_WIN)
                               .arg(winAmount / 100.0, 0, 'f', 2));
}

void Minigame3::triggerDelayedRevealAll(bool playerWonGame)
{
    QTimer::singleShot(1000, this, &Minigame3::revealAllPresentsFinally);
}

void Minigame3::revealAllPresentsFinally()
{
    for (int i = 0; i < TOTAL_PRESENTS; ++i)
    {
        if (!revealedPresentsMask[i])
        {
            updatePresentLabelAppearance(presentLabels[i], presentContentsInternal[i], false, true, false, false);
        }
    }
    revealAllEndTimer->start(2500);
}

void Minigame3::finalizeRound()
{
    isBetActive = false;
    ui->bombButton1->setEnabled(true);
    ui->bombButton3->setEnabled(true);
    ui->bombButton5->setEnabled(true);
    ui->bombButton10->setEnabled(true);
    initializeNewRound();
}

bool Minigame3::isValidBetAmount(long amountInCents)
{
    return amountInCents > 0 && amountInCents <= playerBalance;
}

void Minigame3::resetGame()
{
    if (revealAllEndTimer->isActive())
        revealAllEndTimer->stop();
    isBetActive = false;
    ui->bombButton1->setEnabled(true);
    ui->bombButton3->setEnabled(true);
    ui->bombButton5->setEnabled(true);
    ui->bombButton10->setEnabled(true);

    initializeNewRound();

    if (!initialFontsAdjusted)
    {
        QTimer::singleShot(0, this, &Minigame3::performInitialFontAdjustmentForAllLabels);
    }
    else
    {
    }
    qDebug() << "Minigame3: Game reset. Initial fonts adjusted:" << initialFontsAdjusted;
}

void Minigame3::performInitialFontAdjustmentForAllLabels()
{
    if (initialFontsAdjusted)
        return;

    qDebug() << "Minigame3: Performing initial font adjustment for all labels.";
    for (ClickableLabel *label : presentLabels)
    {
        if (label)
        {
            int labelIndex = presentLabels.indexOf(label);
            if (labelIndex != -1)
            {
                updatePresentLabelAppearance(label, PresentContent::HIDDEN, false, false, false, false);
            }
        }
    }
    initialFontsAdjusted = true;
}