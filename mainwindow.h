#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "loginwindow.h"
#include "clickablelabel.h"
#include "minigame3.h"
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class Minigame1;
class Minigame2;
class Minigame3;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static constexpr float CASINO_EDGE = 0.03;
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    int getSelectedOption() const;

private slots:
    void on_homeButton_clicked();
    void on_Minigame1_Button_clicked();
    void on_Minigame2_Button_clicked();
    void on_Minigame3_Button_clicked();
    void on_homeMinigame1_Button_clicked();
    void on_homeMinigame2_Button_clicked();
    void on_homeMinigame3_Button_clicked();
    void on_LoginButton_clicked();
    void on_betAmountChanged(double value);
    void on_betButton_clicked();
    void on_doubleButton_clicked();
    void on_halfButton_clicked();
    void on_workButton_clicked();
    void on_rollButton_clicked();
    void on_halfButton2_clicked();
    void on_doubleButton2_clicked();
    void on_moreButton_clicked();
    void on_sameButton_clicked();
    void on_lessButton_clicked();
    void on_selectionChanged(bool selected);
    void rollDices();
    void resetMinigame2();
    void on_playButton3_clicked();
    void on_halfButton3_clicked();
    void on_doubleButton3_clicked();

private:
    void initializeRandomSeed();
    void setupMinigames();
    void setupMinigame1Connections();
    void setupMinigame2Connections();
    void setupBetAmountConnections();
    void disableBettingUntilSelection();
    void updateMoneyDisplay(long money);
    void updateBetButtonState();
    void updateBetAmountLimits();
    void addWorkEarnings();
    void setGuessButtonsEnabled(bool enabled);
    void updateRollButtonState();
    void setupMinigame3Connections();
    void updatePlayButtonState();

    Ui::MainWindow *ui;
    LoginWindow *loginWindow = nullptr;
    Minigame1 *minigame1 = nullptr;
    Minigame2 *minigame2 = nullptr;
    Minigame3 *minigame3;
    long playerBalance = 10000;
    bool ANIMATION_MODE = false;
    int playerRoll = 0;
    int casinoRoll = 0;
    long currentDiceBet = 0;
    bool minigame2WaitingForGuess = false;
};
#endif // MAINWINDOW_H
