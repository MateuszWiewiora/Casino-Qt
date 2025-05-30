#ifndef MINIGAME2_H
#define MINIGAME2_H

#include <QObject>
#include <QWidget>
#include <QLabel>

class Minigame2 : public QObject
{
    Q_OBJECT

public:
    explicit Minigame2(QWidget *parent, long &playerBalanceRef);
    bool startDiceGame(double amount);
    void checkAnswer(QString answer);
    void resetGame();
    void initializeChances();

signals:
    void moneyUpdated(long money);
    void playerRollUpdated(const QString &text);
    void resultUpdated(const QString &text);
    void guessButtonsEnabled(bool enabled);
    void rollButtonEnabled(bool enabled);
    void lessChanceUpdated(const QString &text);
    void sameChanceUpdated(const QString &text);
    void moreChanceUpdated(const QString &text);
    void lessMultiplierUpdated(const QString &text);
    void sameMultiplierUpdated(const QString &text);
    void moreMultiplierUpdated(const QString &text);

private:
    long &playerBalance;
    int playerRoll = 0;
    int casinoRoll = 0;
    long currentDiceBet = 0;
    bool minigame2WaitingForGuess = false;
    int CHANCES[20] = {};
    float lessChance = 0.33;
    float sameChance = 0.33;
    float moreChance = 0.33;
    float lessMultiplier = 1;
    float sameMultiplier = 1;
    float moreMultiplier = 1;

    void rollDices();
    int generateDiceRoll();
    float calculateLocalCasinoEdge(float chance);
    void updateChances();
    void updateChancesUI();
    QString calculateMultiplierToShow(float multiplier);
    QString calculateProfitToShow(float multiplier);
    void displayPlayerRoll();
    void logRollResults();
    void initializeGameRound(long amountInCents);
    void promptUserForGuess();
    bool validateGuess(QString answer);
    QString buildResultMessage(bool isCorrectGuess);
    void updateGameState(bool isCorrectGuess);
    void scheduleGameReset();
    void resetGameState();
    void resetUIElements();
    void updateGameControls();
    void updateRollButtonState();
    bool isValidBetAmount(long amountInCents);
    void logInvalidBet();
    void logGameStartFailure();
    void logGameStart(double betAmount);
};

#endif