// ==========================================
// 黑马记账 - 主窗口
// ==========================================
#pragma once

#include <QMainWindow>

class QLabel;
class QResizeEvent;
class QTableWidget;

#include "database.h"

// 主窗口：顶部工具栏 + 账单列表 + 底部总支出
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(Database *db, QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override; // 让空状态提示跟随窗口大小

private slots:
    void onAddExpense(); // 点击「记一笔」
    void refresh();      // 重新加载账单列表和总支出

private:
    Database *m_db;
    QTableWidget *m_table; // 账单列表
    QLabel *m_totalLabel;  // 底部总支出文字
    QLabel *m_emptyLabel;  // 没有账单时的提示文字
};
