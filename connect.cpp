#include "connect.h"
#include "./ui_connect.h"

ConnectWindow::ConnectWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ConnectWindow)
{
    ui->setupUi(this);
    getEthInfo();
    /* 初始PDO通信定时器 设置定时器类型，提高精度，但是对于DC同步模式来说，精度还是不够 */
    pdotimer = new QTimer();
    pdotimer->setTimerType(Qt::PreciseTimer);
    /* 从机未连接 */
    ConnectFlag = false;
    /* 初始话当前选中的网卡 需要将Qstring 转换为 char* */
    QObject::connect(ui->con_pushButton,&QPushButton::clicked,this,&ConnectWindow::connectToSlavers);
    QObject::connect(pdotimer,&QTimer::timeout,this,&ConnectWindow::pdoTaskTimout);
}

ConnectWindow::~ConnectWindow()
{
    delete ui;
}

void ConnectWindow::getEthInfo(void)
{
    ec_adaptert * adapter = NULL;
    adapter = ec_find_adapters ();
    while(adapter != NULL)
    {
        ui->eth_comboBox->addItem(QString::fromUtf8(adapter->desc,-1));
        ethinfo.insert(adapter->desc,adapter->name);
        adapter = adapter->next;
    }
}

void ConnectWindow::connectToSlavers(void)
{
    int  chk, cnt, nSM, j;
    char *eth0;
    int expectedWKC;
    /* 获取网卡 */
    ifname = ui->eth_comboBox->currentText();
    QByteArray ba = ethinfo[ifname].toLatin1();
    eth0 = ba.data();
    emitEthInfo();
    /* 绑定网卡  */
    if(ConnectFlag == false)
    {
        ui->textBrowser->append("start soem\n");
        if (ec_init(eth0))
        {
            ui->textBrowser->append("ec_init on ifname "+ ifname+ " succeeded.\n");
            ui->textBrowser->append(QString::asprintf("ifname dese is %s",eth0));
            /* 扫描从站 */
            if ( ec_config_init(FALSE) > 0 )
            {
                ui->textBrowser->append(QString::asprintf("%d slaves found and configured.\n",ec_slavecount));
                ec_configdc();

                expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
                ui->textBrowser->append(QString::asprintf("Calculated workcounter %d\n", expectedWKC));

                ec_readstate();
                for( cnt = 1 ; cnt <= ec_slavecount ; cnt++)
                {
                    ec_slave[cnt].PO2SOconfig = &Cspsetup;
                    ui->textBrowser->append(QString::asprintf("\nSlave:%d\n Name:%s\n Output size: %dbits\n Input size: %dbits\n State: %d\n Delay: %d[ns]\n Has DC: %d\n",
                           cnt, ec_slave[cnt].name, ec_slave[cnt].Obits, ec_slave[cnt].Ibits,
                                                              ec_slave[cnt].state, ec_slave[cnt].pdelay, ec_slave[cnt].hasdc));
                    if (ec_slave[cnt].hasdc) ui->textBrowser->append(QString::asprintf(" DCParentport:%d\n", ec_slave[cnt].parentport));
                    ui->textBrowser->append(QString::asprintf(" Activeports:%d.%d.%d.%d\n", (ec_slave[cnt].activeports & 0x01) > 0 ,
                           (ec_slave[cnt].activeports & 0x02) > 0 ,
                           (ec_slave[cnt].activeports & 0x04) > 0 ,
                                                              (ec_slave[cnt].activeports & 0x08) > 0 ));
                    ui->textBrowser->append(QString::asprintf(" Configured address: %4.4x\n", ec_slave[cnt].configadr));
                    ui->textBrowser->append(QString::asprintf(" Man: %8.8x ID: %8.8x Rev: %8.8x\n", (int)ec_slave[cnt].eep_man, (int)ec_slave[cnt].eep_id, (int)ec_slave[cnt].eep_rev));
                    for(nSM = 0 ; nSM < EC_MAXSM ; nSM++)
                    {
                        if(ec_slave[cnt].SM[nSM].StartAddr > 0)
                            ui->textBrowser->append(QString::asprintf(" SM%1d A:%4.4x L:%4d F:%8.8x Type:%d\n",nSM, ec_slave[cnt].SM[nSM].StartAddr, ec_slave[cnt].SM[nSM].SMlength,
                                                                      (int)ec_slave[cnt].SM[nSM].SMflags, ec_slave[cnt].SMtype[nSM]));
                    }
                    for(j = 0 ; j < ec_slave[cnt].FMMUunused ; j++)
                    {
                        ui->textBrowser->append(QString::asprintf(" FMMU%1d Ls:%8.8x Ll:%4d Lsb:%d Leb:%d Ps:%4.4x Psb:%d Ty:%2.2x Act:%2.2x\n", j,
                               (int)ec_slave[cnt].FMMU[j].LogStart, ec_slave[cnt].FMMU[j].LogLength, ec_slave[cnt].FMMU[j].LogStartbit,
                               ec_slave[cnt].FMMU[j].LogEndbit, ec_slave[cnt].FMMU[j].PhysStart, ec_slave[cnt].FMMU[j].PhysStartBit,
                                                                  ec_slave[cnt].FMMU[j].FMMUtype, ec_slave[cnt].FMMU[j].FMMUactive));
                    }
                    ui->textBrowser->append(QString::asprintf(" FMMUfunc 0:%d 1:%d 2:%d 3:%d\n",
                                                              ec_slave[cnt].FMMU0func, ec_slave[cnt].FMMU1func, ec_slave[cnt].FMMU2func, ec_slave[cnt].FMMU3func));
                    ui->textBrowser->append(QString::asprintf(" MBX length wr: %d rd: %d MBX protocols : %2.2x\n", ec_slave[cnt].mbx_l, ec_slave[cnt].mbx_rl, ec_slave[cnt].mbx_proto));
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
                /* 启动PDO通信任务 */
                pdotimer->start(20);
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
                    pdotimer->stop();
                    ec_close();
                }
            }
        }
    }else{
        /* 关闭连接 */
        ui->textBrowser->append("close soem\n");
        ConnectFlag = false;
        pdotimer->stop();
        ec_close();
    }
    if(ConnectFlag ==  true)
        ui->con_pushButton->setText(QString::fromUtf8("断开"));
    else
        ui->con_pushButton->setText(QString::fromUtf8("连接"));
}

void ConnectWindow::pdoTaskTimout(void)
{
    int wkc = 0;
    ec_send_processdata();
    wkc = ec_receive_processdata(EC_TIMEOUTRET);
}

void ConnectWindow::emitEthInfo(void)
{
    emit EthInfoSig(this->ifname);
}

int ConnectWindow::Cspsetup(uint16 slave)
{
    int retval=0;
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
