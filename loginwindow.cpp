#include "loginwindow.h"
#include "ui_loginwindow.h"

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWindow)
{
    ui->setupUi(this);

    connect(ui->backButton, &QPushButton::clicked, this, &LoginWindow::returnToMain);
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::on_goregisterButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}


void LoginWindow::on_gologinButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

