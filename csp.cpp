#include "csp.h"
#include "./ui_csp.h"
#include "connect.h"

CspWindow::CspWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CspWindow)
{
    ui->setupUi(this);
    ConnectWindow *cw = new ConnectWindow;
    QObject::connect(cw, SIGNAL(EthInfoSig(QString)), this, SLOT(recvEthInfo(QString)));
    ConnectFlag = false;
    QObject::connect(ui->startBtn,&QPushButton::clicked,this,&CspWindow::connectToSlavers);

}

CspWindow::~CspWindow()
{
    delete ui;
}

void CspWindow::recvEthInfo(QString str)
{
    this->ifname = str;
    ui->textBrowser->append(QString::asprintf("ifname dese is %s",this->ifname));
}

void CspWindow::connectToSlavers(void)
{
    int  chk, cnt, nSM, j;
    char *eth0;
    int expectedWKC;

    QByteArray ba = ethinfo[ifname].toLatin1();
    eth0 = ba.data();
    ui->textBrowser->append(QString::asprintf("ifname dese is %s",eth0));
    /* 绑定网卡  */
    if(ConnectFlag == false)
    {
        ui->textBrowser->append("start soem\n");
        if (ec_init(eth0))
        {
            ui->textBrowser->append("ec_init on ifname "+ ifname+ " succeeded.\n");

            /* 扫描从站 */
            if ( ec_config_init(FALSE) > 0 )
            {
                ui->textBrowser->append(QString::asprintf("%d slaves found and configured.\n",ec_slavecount));
                ec_configdc();

                for( cnt = 1 ; cnt <= ec_slavecount ; cnt++)
                {

                    ec_slave[cnt].PO2SOconfig = &Cspsetup;
                }

                ec_config_map(&IOmap);
                ui->textBrowser->append("Slaves mapped, state to SAFE_OP.\n");
                ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE * 4);
                ui->textBrowser->append(QString::asprintf("segments : %d : %d %d %d %d\n",ec_group[0].nsegments ,ec_group[0].IOsegment[0],ec_group[0].IOsegment[1],ec_group[0].IOsegment[2],ec_group[0].IOsegment[3]));
                ui->textBrowser->append("Request operational state \n");

                ec_slave[0].state = EC_STATE_OPERATIONAL;
                /* 发送一帧 PDO数据 准备进入OP状态 */
                ec_send_processdata();
                ec_receive_processdata(EC_TIMEOUTRET);
                /* OP状态请求 */
                ec_writestate(0);
                chk = 200;
                /* 等待所有从站进入OP状态 */
                do
                {
                    ec_send_processdata();
                    ec_receive_processdata(EC_TIMEOUTRET);
                    ec_statecheck(0, EC_STATE_OPERATIONAL, 50000);
                }
                while (chk-- && (ec_slave[0].state != EC_STATE_OPERATIONAL));
                ui->textBrowser->append(QString::asprintf("chk =%d,%d",chk,ec_slave[1].ALstatuscode));

                if (ec_slave[0].state == EC_STATE_OPERATIONAL )
                {
                    ui->textBrowser->append("Operational state reached .\n");
                    ConnectFlag = true;
                }else{
                    /* 关闭连接 */
                    ui->textBrowser->append("Operational state fail!!! connect closed\n");
                    ConnectFlag = false;
                    ec_close();
                }
            }
        }
    }else{
        /* 关闭连接 */
        ui->textBrowser->append("close soem\n");
        ConnectFlag = false;
        ec_close();
    }

}

int CspWindow::Cspsetup(uint16 slave)
{
    int retval;
    uint8 u8val;
    uint16 u16val;

    u8val = 0x08;
    retval += ec_SDOwrite(slave, 0x6060, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);
    u8val = 0x00;
    retval += ec_SDOwrite(slave, 0x607A, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);
    u8val = 0b00000110;
    retval += ec_SDOwrite(slave, 0x6040, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);
    u8val = 0b00000111;
    retval += ec_SDOwrite(slave, 0x6040, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);
    u8val = 0b00001111;
    retval += ec_SDOwrite(slave, 0x6040, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);

    return retval;
}
