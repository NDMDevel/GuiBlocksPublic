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
    connect(ui->btnAddItem,SIGNAL(clicked()),this,SLOT(btnAddItem()));
    connect(ui->btnFilpH  ,SIGNAL(clicked()),this,SLOT(btnFlipH()));
    connect(ui->btnForceC ,SIGNAL(clicked()),this,SLOT(btnForceConnected()));
    connect(ui->lineEdit  ,SIGNAL(returnPressed()),this,SLOT(lineEditUpdated()));

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
    ui->MainView->printSeq();
}

void MainWindow::btnForceConnected()
{
//    ui->MainView->forcedConnectedLastBlock();
}

void MainWindow::lineEditUpdated()
{
//    ui->MainView->setDebugText(ui->lineEdit->text());
}


