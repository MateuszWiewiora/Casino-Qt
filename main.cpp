#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QDirIterator>
#include <QDir>
#include "client/mainwindow.h"
#include "server/serverwindow.h"

class LauncherWindow : public QMainWindow
{
public:
    LauncherWindow()
    {
        setWindowTitle("Casino Launcher");
        setFixedSize(300, 150);

        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        QVBoxLayout *layout = new QVBoxLayout(centralWidget);

        QPushButton *clientButton = new QPushButton("Launch Client", this);

        layout->addWidget(clientButton);

        connect(clientButton, &QPushButton::clicked, this, &LauncherWindow::launchClient);
    }

    void launchServer()
    {
        ServerWindow *server = new ServerWindow();
        server->show();
    }

    void launchClient()
    {
        MainWindow *client = new MainWindow();
        client->show();
    }
};

QPixmap getPixmap(const QString &path)
{
    return QPixmap(path);
}

QDirIterator getImageIterator()
{
    return QDirIterator(":/assets", QDirIterator::Subdirectories);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    LauncherWindow launcher;
    launcher.launchServer();
    launcher.launchClient();
    launcher.show();
    return app.exec();
}
