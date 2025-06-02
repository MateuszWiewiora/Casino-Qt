#include "minigame2.h"
#include "mainwindow.h"
#include <QRandomGenerator>
#include <QTimer>
#include <QDebug>

Minigame2::Minigame2(QWidget *parent, long &playerBalanceRef)
    : QObject(parent), playerBalance(playerBalanceRef)
{
}

void Minigame2::initializeChances()
{
    for (int i = 1; i <= 10; i++)
    {
        for (int j = 1; j <= 10; j++)
        {
            CHANCES[i + j] += 1;
        }
    }
}

void Minigame2::rollDices()
{
    playerRoll = generateDiceRoll();
    updateChances();
    updateChancesUI();
    casinoRoll = generateDiceRoll();
    displayPlayerRoll();
    logRollResults();
}

int Minigame2::generateDiceRoll()
{
    return QRandomGenerator::global()->bounded(1, 11) + QRandomGenerator::global()->bounded(1, 11);
}

float Minigame2::calculateLocalCasinoEdge(float chance)
{
    float localCasinoEdge = MainWindow::CASINO_EDGE;
    if (1 - chance < localCasinoEdge)
    {
        localCasinoEdge = (1 - chance) * MainWindow::CASINO_EDGE;
    }
    return localCasinoEdge;
}

void Minigame2::updateChances()
{
    lessChance = 0;
    for (int i = 2; i < playerRoll; i++)
    {
        lessChance += CHANCES[i] / 100.0;
    }
    sameChance = CHANCES[playerRoll] / 100.0;
    moreChance = 1 - lessChance - sameChance;

    lessMultiplier = 0;
    sameMultiplier = 0;
    moreMultiplier = 0;

    if (lessChance > 0)
    {
        lessMultiplier = (1 - calculateLocalCasinoEdge(lessChance)) / lessChance;
    }
    if (sameChance > 0)
    {
        sameMultiplier = (1 - calculateLocalCasinoEdge(sameChance)) / sameChance;
    }
    if (moreChance > 0)
    {
        moreMultiplier = (1 - calculateLocalCasinoEdge(moreChance)) / moreChance;
    }
}

void Minigame2::updateChancesUI()
{
    emit lessMultiplierUpdated(calculateMultiplierToShow(lessMultiplier) + calculateProfitToShow(lessMultiplier));
    emit sameMultiplierUpdated(calculateMultiplierToShow(sameMultiplier) + calculateProfitToShow(sameMultiplier));
    emit moreMultiplierUpdated(calculateMultiplierToShow(moreMultiplier) + calculateProfitToShow(moreMultiplier));

    emit lessChanceUpdated(QString::number(lessChance * 100, 'f', 2) + "%");
    emit sameChanceUpdated(QString::number(sameChance * 100, 'f', 2) + "%");
    emit moreChanceUpdated(QString::number(moreChance * 100, 'f', 2) + "%");
}

QString Minigame2::calculateMultiplierToShow(float multiplier)
{
    if (multiplier < 1.01 && multiplier != 0)
    {
        return "<x" + QString::number(multiplier, 'f', 2);
    }
    return "x" + QString::number(multiplier, 'f', 2);
}

QString Minigame2::calculateProfitToShow(float multiplier)
{
    if (multiplier == 0)
    {
        return " (+$0.00)";
    }

    long profit = currentDiceBet;
    profit *= (multiplier - 1);
    return " (+$" + QString::number(profit / 100.0, 'f', 2) + ")";
}

void Minigame2::displayPlayerRoll()
{
    emit playerRollUpdated("Your roll: " + QString::number(playerRoll));
}

void Minigame2::logRollResults()
{
    qDebug() << "Player rolled:" << playerRoll << "Casino rolled:" << casinoRoll;
}

bool Minigame2::startDiceGame(double amount)
{
    long amountInCents = std::floor(amount * 100);
    if (!isValidBetAmount(amountInCents))
        return false;

    initializeGameRound(amountInCents);
    return true;
}

void Minigame2::initializeGameRound(long amountInCents)
{
    currentDiceBet = amountInCents;
    rollDices();
    promptUserForGuess();
    emit guessButtonsEnabled(true);
    emit rollButtonEnabled(false);
    minigame2WaitingForGuess = true;
}

void Minigame2::promptUserForGuess()
{
    emit resultUpdated("Make your guess: Will casino roll LESS, SAME, or MORE than you?");
}

void Minigame2::checkAnswer(QString answer)
{
    if (!minigame2WaitingForGuess)
        return;

    emit guessButtonsEnabled(false);
    minigame2WaitingForGuess = false;

    bool isCorrectGuess = validateGuess(answer);
    QString resultMessage = buildResultMessage(isCorrectGuess);
    updateGameState(isCorrectGuess);
    emit resultUpdated(resultMessage);
    scheduleGameReset();
}

bool Minigame2::validateGuess(QString answer)
{
    if (answer == "More" && casinoRoll > playerRoll)
    {
        currentDiceBet *= (moreMultiplier - 1);
        return true;
    }
    if (answer == "Same" && casinoRoll == playerRoll)
    {
        currentDiceBet *= (sameMultiplier - 1);
        return true;
    }
    if (answer == "Less" && casinoRoll < playerRoll)
    {
        currentDiceBet *= (lessMultiplier - 1);
        return true;
    }
    return false;
}

QString Minigame2::buildResultMessage(bool isCorrectGuess)
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

void Minigame2::updateGameState(bool isCorrectGuess)
{
    if (isCorrectGuess)
    {
        playerBalance += currentDiceBet;
    }
    else
    {
        playerBalance -= currentDiceBet;
    }
    emit moneyUpdated(playerBalance);
}

void Minigame2::scheduleGameReset()
{
    QTimer::singleShot(10, this, &Minigame2::resetGame);
}

void Minigame2::resetGame()
{
    resetGameState();
    resetUIElements();
    updateGameControls();
}

void Minigame2::resetGameState()
{
    playerRoll = 0;
    casinoRoll = 0;
    currentDiceBet = 0;
    minigame2WaitingForGuess = false;
}

void Minigame2::resetUIElements()
{
    // emit playerRollUpdated("Click ROLL to play!");
    // emit resultUpdated("Place your bet and roll the dice!");
}

void Minigame2::updateGameControls()
{
    emit guessButtonsEnabled(false);
    emit rollButtonEnabled(true);
    emit moneyUpdated(playerBalance);
    updateRollButtonState();
}

void Minigame2::updateRollButtonState()
{
    if (!minigame2WaitingForGuess)
    {
        emit rollButtonEnabled(true);
    }
}

bool Minigame2::isValidBetAmount(long amountInCents)
{
    return amountInCents > 0 && amountInCents <= playerBalance;
}

void Minigame2::logInvalidBet()
{
    qDebug() << "Invalid bet amount";
}

void Minigame2::logGameStartFailure()
{
    qDebug() << "Failed to start dice game";
}

void Minigame2::logGameStart(double betAmount)
{
    qDebug() << "Starting dice game with bet:" << betAmount;
}