#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "GuiBlocks/Editor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

using GuiBlocks::Editor;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    //------------ User Section INIT
public:

private slots:
    void btnAddItem();
    void btnFlipH();
    void btnForceConnected();
    void lineEditUpdated();

private:
    //------------ User Section END
private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
