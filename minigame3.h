#ifndef MINIGAME3_H
#define MINIGAME3_H

#include <QObject>
#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QFont>
#include <QMap>
#include "clickablelabel.h"
#include "ui_mainwindow.h"

class Minigame3 : public QObject
{
    Q_OBJECT

public:
    explicit Minigame3(QWidget *parent, Ui::MainWindow *uiRef, long &playerBalanceRef);
    ~Minigame3();

    void confirmBetAndStartGame();
    void resetGame();

public slots:
    void setBombCount(int bombs);

signals:
    void moneyUpdated(long money);
    void gameStatusUpdated(const QString &status);
    void playButtonEnabled(bool enabled);

private:
    long &playerBalance;
    Ui::MainWindow *ui;
    QVector<ClickableLabel *> presentLabels;
    ClickableLabel *selectedPresentForBet = nullptr;

    QTimer *revealAllEndTimer = nullptr;

    bool isBetActive = false;
    long currentBetAmountInCents = 0;
    int starsFoundCount = 0;
    int currentNumberOfBombs = 3;
    int clickCount = 0;
    bool isGameBlocked = false;

    QVector<bool> revealedPresentsMask;

    enum class PresentContent
    {
        HIDDEN,
        STAR,
        BOMB
    };
    QVector<PresentContent> presentContentsInternal;

    static const int TOTAL_PRESENTS = 25;
    static const int TARGET_STARS_TO_WIN = 5;

    const QString GIFT_EMOJI = "🎁";
    const QString STAR_EMOJI = "⭐";
    const QString BOMB_EMOJI = "💣";

    QMap<int, double> bombMultipliers;

    void createAndSetupClickableLabels(QWidget *parentWidget);
    void connectPresentSignals();
    void connectBombSelectionSignals();

    void handlePresentClicked(ClickableLabel *clickedLabel);
    void revealAndUpdate(ClickableLabel *revealedLabel);

    void initializeNewRound();
    void shuffleAndAssignContents();
    void updatePresentLabelAppearance(ClickableLabel *label, PresentContent contentType, bool isInitialSelection, bool isRevealed, bool isWinning, bool isLosing);
    void adjustLabelFont(ClickableLabel *label);
    void updateAllLabelsToGift();
    void updateBombButtonsUI();

    void playerLoses();
    void playerWins();
    void triggerDelayedRevealAll(bool playerWonGame);
    void revealAllPresentsFinally();
    void finalizeRound();

    bool isValidBetAmount(long amountInCents);

    bool initialFontsAdjusted = false;
    void performInitialFontAdjustmentForAllLabels();
};

#endif