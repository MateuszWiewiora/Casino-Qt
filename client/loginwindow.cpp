#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "mainwindow.h"

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent), ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
    connect(ui->backButton, &QPushButton::clicked, this, &LoginWindow::returnToMain);
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::setMainWindow(MainWindow *window)
{
    mainWindow = window;
}

void LoginWindow::on_gologinButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void LoginWindow::on_loginButton_clicked()
{
    mainWindow->login(ui->usernameLineEdit->text(), ui->passwordLineEdit->text());
}

void LoginWindow::on_goregisterButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void LoginWindow::on_registerButton_clicked()
{
    mainWindow->registerUser(ui->usernameLineEdit_2->text(), ui->passwordLineEdit_2->text());
}
