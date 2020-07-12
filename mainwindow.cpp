#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include "GuiBlocks/Utils.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//    qDebug() << GuiBlocks::nextEvenGridValue(80,20);

    //Test Connections
    connect(ui->btnAddItem,SIGNAL(clicked()),this,SLOT(btnAddItem()));
    connect(ui->btnFilpH  ,SIGNAL(clicked()),this,SLOT(btnFlipH()));
    connect(ui->btnForceC ,SIGNAL(clicked()),this,SLOT(btnForceConnected()));
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
    ui->MainView->filpLastBlock();
}

void MainWindow::btnForceConnected()
{
    ui->MainView->forcedConnectedLastBlock();
}


