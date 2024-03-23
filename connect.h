#ifndef CONNECT_H
#define CONNECT_H

#include <QMainWindow>
#include <QMap>
#include <QTimer>
#include "ethercat.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class ConnectWindow;
}
QT_END_NAMESPACE

class ConnectWindow : public QMainWindow
{
    Q_OBJECT

public:
    ConnectWindow(QWidget *parent = nullptr);
    ~ConnectWindow();
    void getEthInfo(void);
    void emitEthInfo(void);

private slots:
    void connectToSlavers(void);   /* 配置Ethercat从站 */
    void pdoTaskTimout(void);    /* 定时器槽函数，用于过程数据通信 */

signals:
    void EthInfoSig(QString);

private:
    Ui::ConnectWindow *ui;
    QMap<QString,QString> ethinfo;  /* 存放网卡信息 */
    QString ifname;    /* 网卡名字 */
    bool ConnectFlag;      /* 成功初始化标志位 */
    QTimer *pdotimer;
    char IOmap[256];       /* PDO映射的数组 */

    static int Cspsetup(uint16);

};
#endif
