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
    void updateMoneyDisplay(double money);
    void on_LoginButton_clicked();
    void on_betButton_clicked();
    void on_option1Label_clicked();
    void on_option2Label_clicked();
    void on_selectionChanged(bool selected);
    void on_betAmountChanged(double value);
    void on_halfButton_clicked();
    void on_doubleButton_clicked();
    void animationFinished();
    void resetWinAnimation();
    void resetLoseAnimation();

    void on_workButton_clicked();

private:
    void updateBetButtonState();
    void deselectAll();
    void updateBetAmountLimits();
    bool placeBet(double amount);
    QStringList getAvailableImages();
    void updateOptionDisplay(const QString &firstImagePath, const QString &secondImagePath);
    void resizeEvent(QResizeEvent *event) override; //scale png

    QString currentImage1Path;
    QString currentImage2Path;
    QString formatImageName(const QString &imagePath);
    void toggleOptionSelection(ClickableLabel *selectedLabel);
    void showWinAnimation(ClickableLabel *winningLabel);
    void showLoseAnimation(ClickableLabel *losingLabel);


    Ui::MainWindow *ui;
    LoginWindow *loginWindow;
    ClickableLabel *option1Label;
    ClickableLabel *option2Label;
    QTimer *initTimer;
    ClickableLabel *winningLabel = nullptr;
    ClickableLabel *losingLabel = nullptr;
    double playerBalance = 100.0;
    bool ANIMATION_MODE = false;
};
#endif // MAINWINDOW_H
