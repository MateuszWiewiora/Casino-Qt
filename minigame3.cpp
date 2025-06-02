#include "minigame3.h"
#include "mainwindow.h"
#include <QDebug>

Minigame3::Minigame3(QWidget *parent, long &playerBalanceRef)
    : QObject(parent), playerBalance(playerBalanceRef)
{
}

bool Minigame3::placeBet(double amount)
{
    long amountInCents = std::floor(amount * 100);

    if (!isValidBetAmount(amountInCents))
    {
        qDebug() << "Invalid bet amount:" << amountInCents << "Balance:" << playerBalance;
        return false;
    }

    // Tu będzie logika gry - na razie placeholder
    processGameOutcome(amountInCents);
    return true;
}

bool Minigame3::isValidBetAmount(long amountInCents)
{
    return amountInCents > 0 && amountInCents <= playerBalance;
}

void Minigame3::processGameOutcome(long amountInCents)
{
    // Placeholder - losowy wynik 50/50
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

void Minigame3::handleWin(long amountInCents)
{
    playerBalance += amountInCents * (1 - MainWindow::CASINO_EDGE);
    qDebug() << "Player won!" << amountInCents << "New balance:" << playerBalance;
}

void Minigame3::handleLoss(long amountInCents)
{
    playerBalance -= amountInCents;
    qDebug() << "Player lost!" << amountInCents << "New balance:" << playerBalance;
}

void Minigame3::resetGame()
{
    // Reset stanu gry - na razie pusty
    qDebug() << "Minigame3 reset";
}

Minigame3::~Minigame3()
{
}