#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QTimer>
#include "ethercat.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void getEthInfo(void);

private slots:
    void connectToSlavers(void);   /* 配置Ethercat从站 */
    void pdoTaskTimout(void);    /* 定时器槽函数，用于过程数据通信 */

private:
    Ui::MainWindow *ui;
    QMap<QString,QString> ethinfo;  /* 存放网卡信息 */
    QString ifname;    /* 网卡名字 */
    bool ConnectFlag;      /* 成功初始化标志位 */
    QTimer *pdotimer;
    char IOmap[256];       /* PDO映射的数组 */
};
#endif // MAINWINDOW_H
