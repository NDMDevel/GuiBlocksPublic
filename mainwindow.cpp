#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include "GuiBlocks/Utils.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Test Connections
    connect(ui->btnAddItem, SIGNAL(clicked())        , this, SLOT(btnAddItem()));
    connect(ui->btnFilpH  , SIGNAL(clicked())        , this, SLOT(btnFlipH()));
    connect(ui->btnForceC , SIGNAL(clicked())        , this, SLOT(btnForceConnected()));
    connect(ui->lineEdit  , SIGNAL(returnPressed())  , this, SLOT(lineEditUpdated()));
    connect(ui->MainView,
            SIGNAL(updateCoords(const QPointF&)),
            this,
            SLOT(displayCoordOnStatusBar(const QPointF&)));

    //Win Icon
    setWindowIcon(QIcon("../GuiBlocks/GuiBlocks/Images/45-512.png"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::btnAddItem()
{
    ui->MainView->addBlock();
}

void MainWindow::btnFlipH()
{
//    ui->MainView->showLastTwoIndexes();
//    ui->MainView->printSeq();
    ui->MainView->flipLastBlock();
}

void MainWindow::btnForceConnected()
{
//    ui->MainView->forcedConnectedLastBlock();
    ui->MainView->showCurrentLinkData();
}

void MainWindow::lineEditUpdated()
{
    //    ui->MainView->setDebugText(ui->lineEdit->text());
}

void MainWindow::displayCoordOnStatusBar(const QPointF &coord)
{
    QString xy;
    xy = "x: " + QString::number(coord.x()) +
         " y: " + QString::number(coord.y());
    ui->statusbar->showMessage(xy);
}


