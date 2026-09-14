// ==========================================
// 黑马记账 - 主窗口
// ==========================================
#pragma once

#include <QMainWindow>

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QResizeEvent;
class QTableWidget;
class QTableWidgetItem;

#include "database.h"

// 主窗口：工具栏（记一笔/统计/设置/修改/删除）+ 搜索栏 + 账单列表 + 底部收支汇总
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(Database *db, QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override; // 让空状态提示跟随窗口大小

private slots:
    void onAddExpense();     // 点击「记一笔」
    void onShowStats();      // 点击「统计」
    void onShowSettings();   // 点击「设置」
    void onEditSelected();   // 点击「修改」
    void onDeleteButton();   // 点击「删除」/「确认删除」
    void onCancelDelete();   // 取消勾选模式
    void onItemChanged(QTableWidgetItem *item); // 勾选数量变化
    void refresh();          // 重新加载账单列表和汇总

private:
    void applyTheme();       // 把当前主题颜色应用到主窗口各控件
    QString dateFormat() const; // 当前日期显示格式（设置页可改）
    void editRow(int row);   // 打开修改弹窗（预填第 row 行账单）
    int checkedCount() const; // 当前勾选的账单数量
    void enterDeleteMode();  // 进入勾选删除模式
    void exitDeleteMode();   // 退出勾选删除模式
    void confirmDelete();    // 确认删除勾选的账单

    Database *m_db;
    QTableWidget *m_table;   // 账单列表（第 0 列是勾选框，平时隐藏）
    QLabel *m_emptyLabel;    // 没有账单时的提示文字

    // 底部状态栏汇总
    QLabel *m_countLabel;    // 共 N 笔
    QLabel *m_expenseLabel;  // 支出
    QLabel *m_incomeLabel;   // 收入
    QLabel *m_balanceLabel;  // 结余

    // 工具栏控件
    QPushButton *m_addButton;
    QPushButton *m_statsButton;
    QPushButton *m_settingsButton;
    QPushButton *m_editButton;
    QPushButton *m_deleteButton;
    QPushButton *m_cancelDeleteButton;

    // 搜索栏
    QLineEdit *m_searchEdit;     // 关键词搜索（金额/日期/分类/备注）
    QComboBox *m_categoryFilter; // 分类筛选（全部 + 支出/收入大类）
    QPushButton *m_clearButton;  // 清空搜索

    // 状态
    QList<Expense> m_currentList; // 当前列表显示的账单（与表格行一一对应）
    bool m_deleteMode = false;    // 是否处于勾选删除模式
    bool m_updating = false;      // 刷新列表时屏蔽勾选信号
};
