#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "cameracontrolshelp.h"
#include "playerinfo.h"


namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    int scene;

private slots:
    void on_actionQuit_triggered();

    void on_actionCamera_Controls_triggered();

    //scene
    void slot_setSceneSinglePlayer();
    void slot_setSceneMultiPlayer();
    void slot_setSceneHostServer();
    void slot_setSceneJoinServer();
    void slot_setSeed();
    void slot_startGame();

private:
    Ui::MainWindow *ui;
    CameraControlsHelp cHelp;
    PlayerInfo playerInfoWindow;
};


#endif // MAINWINDOW_H
