#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "loginwindow.h"

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

private slots:
    void on_Minigame1_Button_clicked();

    void on_homeButton_clicked();

    void on_Minigame2_Button_clicked();

    void on_Minigame3_Button_clicked();

    void on_homeMinigame1_Button_clicked();

    void on_homeMinigame2_Button_clicked();

    void on_homeMinigame3_Button_clicked();

    void updateMoneyDisplay(int money);

    void on_LoginButton_clicked();

private:
    Ui::MainWindow *ui;
    LoginWindow *loginWindow;
};
#endif // MAINWINDOW_H
