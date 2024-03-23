#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "connect.h"
#include "./ui_connect.h"

#include "csp.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // QObject::connect(ui->connectBtn, &QPushButton::clicked, this, &MainWindow::on_connectBtn_clicked);
    createFileActions();
    createFileMenus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_connectBtn_clicked()
{
    ConnectWindow *cw = new ConnectWindow(this);
    cw->show();
}

void MainWindow::openSlot(){}
void MainWindow::exitSlot(){
    this->close();
}
void MainWindow::createFileActions()
{
    open = new QAction ( QIcon ( ":D:/Qt-project/test/images/open.png" ), tr ( "&Open" ), this );
    open->setShortcuts ( QKeySequence::Open );
    open->setStatusTip ( tr ( "open a new file" ) );
    connect ( open, SIGNAL ( triggered() ), this, SLOT ( openSlot() ) );
    exit = new QAction ( tr ( "&exit" ), this );
    exit->setShortcuts ( QKeySequence::Close );
    exit->setStatusTip ( tr ( "exit a new file" ) );
    connect ( exit, SIGNAL ( triggered() ), this, SLOT ( exitSlot() ) ); /* 点击exit事件调用test */
}

void MainWindow::createFileMenus()
{
    fileMenu = menuBar()->addMenu ( tr ( "&File" ) ); /* 创建一个name为file的菜单栏 */
    /* 在这个菜单栏添加action即下拉菜单 */
    fileMenu->addAction ( open );
    fileMenu->addSeparator(); /* 添加一个分割器 */
    fileMenu->addAction ( exit ); /* 退出下拉菜单 */
}

void MainWindow::on_paraBtn_clicked()
{
    CspWindow *csw = new CspWindow(this);
    csw->show();
}

