#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Interface/Interfaces.h"
#include <memory>

#include <future>
#include<thread>
#include<QDebug>
#include<utility>
#include<functional>
#include<chrono>

//class ReconClientSocket;
//class ReconServer;

namespace Ui {
class MainWindow;

}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


private slots:
    //void on_loadpushButton_clicked();

    void on_actionOpen_triggered();
    //void slotDataReceived(QByteArray dataArray);
    void slotDataReceived(int length);
    void slotDisconnected(int);    
    void on_buttonStart_clicked();

    void on_buttonBrowsePipeline_clicked();

    void on_buttonFinish_clicked();

private:
    Ui::MainWindow *ui;

    void SetImportedProcessors();
    //std::shared_ptr<ReconServer> reconServer;

};

#endif // MAINWINDOW_H
