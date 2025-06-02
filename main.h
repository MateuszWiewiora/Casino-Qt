#ifndef MAIN_H
#define MAIN_H

#include <QMainWindow>
#include <QPixmap>
#include <QString>
#include <QDirIterator>
#include <QDir>

class QPushButton;
class QVBoxLayout;
class QWidget;
class ServerWindow;
class MainWindow;

class LauncherWindow : public QMainWindow
{
    Q_OBJECT

public:
    LauncherWindow();
    void launchServer();
    void launchClient();
};

QPixmap getPixmap(const QString &path);
QDirIterator getImageIterator();

#endif
