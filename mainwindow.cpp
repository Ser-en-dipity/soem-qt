#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    getEthInfo();
    /* 初始PDO通信定时器 设置定时器类型，提高精度，但是对于DC同步模式来说，精度还是不够 */
    pdotimer = new QTimer();
    pdotimer->setTimerType(Qt::PreciseTimer);
    /* 从机未连接 */
    ConnectFlag = false;
    /* 初始话当前选中的网卡 需要将Qstring 转换为 char* */
    QObject::connect(ui->con_pushButton,&QPushButton::clicked,this,&MainWindow::connectToSlavers);
    QObject::connect(pdotimer,&QTimer::timeout,this,&MainWindow::pdoTaskTimout);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::getEthInfo(void)
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

void MainWindow::connectToSlavers(void)
{
    int  chk;
    char *eth0;
    /* 获取网卡 */
    ifname = ui->eth_comboBox->currentText();
    QByteArray ba = ethinfo[ifname].toLatin1();
    eth0 = ba.data();
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
                /* 配置DC周期 周期理想的状态是和定时器的周期一样，但是由于定时器精度不够，所以周期需要增加，测试不建议使用DC同步模式 */
                ec_dcsync0(1, TRUE, 25000000,0);
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
                chk = 210;
                /* 等待所有从站进入OP状态 */
                do
                {
                    ec_statecheck(0, EC_STATE_OPERATIONAL, 50000);
                    ec_send_processdata();
                    ec_receive_processdata(EC_TIMEOUTRET);
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

void MainWindow::pdoTaskTimout(void)
{
    int wkc = 0;
    ec_send_processdata();
    wkc = ec_receive_processdata(EC_TIMEOUTRET);
}
