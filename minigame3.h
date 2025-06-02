#ifndef MINIGAME3_H
#define MINIGAME3_H

#include <QObject>
#include <QWidget>
#include <QTimer>

class Minigame3 : public QObject
{
    Q_OBJECT

public:
    explicit Minigame3(QWidget *parent, long &playerBalanceRef);
    ~Minigame3();

    bool placeBet(double amount);
    void resetGame();

signals:
    void moneyUpdated(long money);
    void gameStatusUpdated(const QString &status);
    void resultUpdated(const QString &result);

private:
    long &playerBalance;

    bool isValidBetAmount(long amountInCents);
    void processGameOutcome(long amountInCents);
    void handleWin(long amountInCents);
    void handleLoss(long amountInCents);
};

#endif