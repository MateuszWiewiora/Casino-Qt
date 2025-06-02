#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class LoginWindow;
}
QT_END_NAMESPACE

class MainWindow;

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();
    void setMainWindow(MainWindow *window);

signals:
    void returnToMain();

private slots:
    void on_gologinButton_clicked();
    void on_goregisterButton_clicked();
    void on_loginButton_clicked();
    void on_registerButton_clicked();

private:
    Ui::LoginWindow *ui;
    MainWindow *mainWindow = nullptr;
};

#endif // LOGINWINDOW_H
