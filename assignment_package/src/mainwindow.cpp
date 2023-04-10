#include "mainwindow.h"
#include <ui_mainwindow.h>
#include "cameracontrolshelp.h"
#include <QResizeEvent>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), cHelp(),
    scene(0)
{
    ui->setupUi(this);
    ui->mygl->setFocus();
    this->playerInfoWindow.show();
    playerInfoWindow.move(QGuiApplication::primaryScreen()->availableGeometry().center() - this->rect().center() + QPoint(this->width() * 0.75, 0));

    //home screen
    connect(ui->singleplayerButton, SIGNAL(clicked()), SLOT(slot_setSceneSinglePlayer()));
    connect(ui->multiplayerButton, SIGNAL(clicked()), SLOT(slot_setSceneMultiPlayer()));
    connect(ui->hostServerButton, SIGNAL(clicked()), SLOT(slot_setSceneHostServer()));
    connect(ui->joinServerButton, SIGNAL(clicked()), SLOT(slot_setSceneJoinServer()));

    connect(ui->mygl, SIGNAL(sig_sendPlayerPos(QString)), &playerInfoWindow, SLOT(slot_setPosText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerVel(QString)), &playerInfoWindow, SLOT(slot_setVelText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerAcc(QString)), &playerInfoWindow, SLOT(slot_setAccText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerLook(QString)), &playerInfoWindow, SLOT(slot_setLookText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerChunk(QString)), &playerInfoWindow, SLOT(slot_setChunkText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerTerrainZone(QString)), &playerInfoWindow, SLOT(slot_setZoneText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendServerIP(QString)), &playerInfoWindow, SLOT(slot_setServerIP(QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::exit();
}

void MainWindow::on_actionCamera_Controls_triggered()
{
    cHelp.show();
}

void MainWindow::slot_setSceneSinglePlayer() {
    scene = 1;
    ui->scenesWidget->setCurrentIndex(0);
    ui->mygl->start(false);
}

void MainWindow::slot_setSceneMultiPlayer() {
    scene = 2;
    // change to something else
    ui->scenesWidget->setCurrentIndex(2);
    //ui->mygl->start(false);
}

void MainWindow::slot_setSceneHostServer() {
    scene = 1;
    ui->scenesWidget->setCurrentIndex(0);
    ui->mygl->start(false);
}

void MainWindow::slot_setSceneJoinServer() {
    scene = 1;
    ui->scenesWidget->setCurrentIndex(0);
    ui->mygl->ip = ui->serverIPText->displayText().toStdString();
    ui->mygl->start(true);
}

void MainWindow::slot_setSeed() {
    ui->scenesWidget->setCurrentIndex(1);
}

void MainWindow::slot_startGame() {
    scene = 3;
    ui->scenesWidget->setCurrentIndex(0);
}
