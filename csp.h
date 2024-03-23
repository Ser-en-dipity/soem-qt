#ifndef CSP_H
#define CSP_H

#include <QWidget>
#include "ethercat.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class CspWindow;
}
QT_END_NAMESPACE

class CspWindow : public QWidget
{
    Q_OBJECT

public:
    CspWindow(QWidget *parent = nullptr);
    ~CspWindow();
    void connectToSlavers(void);   /* 配置Ethercat从站 */
    static int Cspsetup(uint16);

private slots:
    void recvEthInfo(QString);
private:
    Ui::CspWindow *ui;
    QMap<QString,QString> ethinfo;  /* 存放网卡信息 */
    QString ifname;    /* 网卡名字 */
    bool ConnectFlag;      /* 成功初始化标志位 */
    char IOmap[256];       /* PDO映射的数组 */
};
#endif
