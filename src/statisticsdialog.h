// ==========================================
// 黑马记账 - 收支统计页面
// ==========================================
#pragma once

#include <QDialog>

class QChartView;
class QComboBox;
class QDateEdit;
class QLabel;
class QTableWidget;
class QWidget;

#include "database.h"

// 统计弹窗：时间段内总收入 / 总支出 / 结余 + 饼图 + 分类明细（收支可切换）
class StatisticsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit StatisticsDialog(Database *db, QWidget *parent = nullptr);

private slots:
    void onPresetChanged(); // 预设切换（含自定义范围显示/隐藏）
    void recalc();          // 重新查询并刷新统计结果

private:
    QString rangeFrom() const; // 当前范围的开始日期 yyyy-MM-dd
    QString rangeTo() const;   // 当前范围的结束日期 yyyy-MM-dd

    Database *m_db;
    QComboBox *m_preset;       // 时间范围预设
    QComboBox *m_typeFilter;   // 统计类型：支出 / 收入
    QWidget *m_customRange;    // 自定义日期选择区（含 从/到 两个日期框）
    QDateEdit *m_fromDate;
    QDateEdit *m_toDate;
    QLabel *m_incomeLabel;     // 总收入大字（绿色）
    QLabel *m_expenseLabel;    // 总支出大字（主题色）
    QLabel *m_balanceLabel;    // 结余大字（正绿负红）
    QLabel *m_countLabel;      // 笔数
    QChartView *m_chartView;   // 分类占比饼图
    QTableWidget *m_categoryTable; // 各一级大类明细（分类 / 金额 / 占比）
};
