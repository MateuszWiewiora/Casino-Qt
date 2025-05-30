#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "loginwindow.h"
#include "clickablelabel.h"
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    int getSelectedOption() const;

private slots:
    void on_Minigame1_Button_clicked();
    void on_homeButton_clicked();
    void on_Minigame2_Button_clicked();
    void on_Minigame3_Button_clicked();
    void on_homeMinigame1_Button_clicked();
    void on_homeMinigame2_Button_clicked();
    void on_homeMinigame3_Button_clicked();
    void updateMoneyDisplay(long money);
    void on_LoginButton_clicked();
    void on_betButton_clicked();
    void on_selectionChanged(bool selected);
    void on_betAmountChanged(double value);
    void on_halfButton_clicked();
    void on_doubleButton_clicked();
    void animationFinished();
    void resetWinAnimation();
    void resetLoseAnimation();
    void rollDices();
    void resetMinigame2();
    void on_halfButton2_clicked();
    void on_doubleButton2_clicked();
    void on_workButton_clicked();
    void on_moreButton_clicked();
    void on_sameButton_clicked();
    void on_lessButton_clicked();
    void on_rollButton_clicked();

private:
    void initializeRandomSeed();
    void createAndSetupClickableLabels();
    void connectLabelSignals();
    void setupBetAmountConnections();
    void replaceLabelWidgets();
    void disableBettingUntilSelection();
    void setupInitialGameImages();
    void updateBetButtonState();
    void deselectAll();
    void updateBetAmountLimits();
    bool placeBet(double amount);
    QStringList getAvailableImages();
    void updateOptionDisplay(const QString &firstImagePath, const QString &secondImagePath);
    void displayFirstOption(const QString &imagePath);
    void displaySecondOption(const QString &imagePath);
    QString formatImageName(const QString &imagePath);
    void toggleOptionSelection(ClickableLabel *selectedLabel);
    void showWinAnimation(ClickableLabel *winningLabel);
    void showLoseAnimation(ClickableLabel *losingLabel);
    void displayRandomGameOptions();
    bool isValidBetAmount(long amountInCents);
    void processGameOutcome(long amountInCents);
    void handleWin(long amountInCents);
    void handleLoss(long amountInCents);
    void startResultAnimation();
    void addWorkEarnings();
    int generateDiceRoll();
    void displayPlayerRoll();
    void logRollResults();
    void checkAnswer(QString answer);
    void disableGuessButtons();
    bool validateGuess(QString answer);
    QString buildResultMessage(bool isCorrectGuess);
    void updateGameState(bool isCorrectGuess);
    void displayResult(const QString &resultMessage);
    void scheduleGameReset();
    void resetGameState();
    void resetUIElements();
    void updateGameControls();
    bool isValidBetAmount(double amount);
    void logInvalidBet();
    void logGameStartFailure();
    void logGameStart(double betAmount);
    bool startDiceGame(double amount);
    void initializeGameRound(long amountInCents);
    void promptUserForGuess();
    void enableGuessButtons();
    void disableRollButton();
    void updateRollButtonState();

    Ui::MainWindow *ui;
    LoginWindow *loginWindow;
    ClickableLabel *option1Label;
    ClickableLabel *option2Label;
    QTimer *initTimer;
    ClickableLabel *winningLabel = nullptr;
    ClickableLabel *losingLabel = nullptr;
    long playerBalance = 10000;
    bool ANIMATION_MODE = false;
    int playerRoll = 0;
    int casinoRoll = 0;
    long currentDiceBet = 0;
    bool minigame2WaitingForGuess = false;
};
#endif // MAINWINDOW_H
