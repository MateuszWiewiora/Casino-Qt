#ifndef MINIGAME1_H
#define MINIGAME1_H

#include <QObject>
#include <QTimer>
#include "clickablelabel.h"
#include "ui_mainwindow.h"

class Minigame1 : public QObject
{
    Q_OBJECT

public:
    explicit Minigame1(QWidget *parent, long &playerBalanceRef);
    ~Minigame1();

    void createAndSetupClickableLabels(QWidget *parent, Ui::MainWindow *ui);
    void setupInitialGameImages();
    bool placeBet(double amount);
    int getSelectedOption() const;

signals:
    void moneyUpdated(long money);
    void betButtonEnabled(bool enabled);
    void selectionChanged(bool hasSelection);
    void option1NameUpdated(const QString &name);
    void option2NameUpdated(const QString &name);

private:
    long &playerBalance;
    QTimer *initTimer = nullptr;
    ClickableLabel *option1Label = nullptr;
    ClickableLabel *option2Label = nullptr;
    ClickableLabel *winningLabel = nullptr;
    ClickableLabel *losingLabel = nullptr;
    bool ANIMATION_MODE = false;

    void connectLabelSignals();
    void replaceLabelWidgets(Ui::MainWindow *ui);
    bool isValidBetAmount(long amountInCents);
    void processGameOutcome(long amountInCents);
    void handleWin(long amountInCents);
    void handleLoss(long amountInCents);
    void startResultAnimation();
    void animationFinished();
    void displayRandomGameOptions();
    QStringList getAvailableImages();
    void updateOptionDisplay(const QString &firstImagePath, const QString &secondImagePath);
    void displayFirstOption(const QString &imagePath);
    void displaySecondOption(const QString &imagePath);
    QString formatImageName(const QString &imagePath);
    void toggleOptionSelection(ClickableLabel *selectedLabel);
    void onSelectionChanged(bool selected);
    void deselectAll();
    void showWinAnimation(ClickableLabel *label);
    void showLoseAnimation(ClickableLabel *label);
    void resetWinAnimation();
    void resetLoseAnimation();
};

#endif