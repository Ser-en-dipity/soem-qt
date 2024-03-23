#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "connect.h"

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

private slots:
    void on_connectBtn_clicked();

    void openSlot();
    void exitSlot();

    void on_paraBtn_clicked();

private:
    Ui::MainWindow *ui;
    ConnectWindow *wc;

    void createFileActions(); // 创建file下拉行为
    void createFileMenus(); // 创建file下拉页面
    QMenu *fileMenu; // 文件菜单
    QAction *open;
    QAction *exit;
};
#endif // MAINWINDOW_H
