// ==========================================
// 黑马记账 - 「记一笔」输入弹窗（新增 / 修改共用，支持支出与收入）
// ==========================================
#pragma once

#include "database.h"

#include <QDialog>

class QButtonGroup;
class QComboBox;
class QDateEdit;
class QDoubleSpinBox;
class QLineEdit;
class QPushButton;

// 输入一条收支记录的弹窗：类型、金额、两级分类、日期、备注
class AddExpenseDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddExpenseDialog(QWidget *parent = nullptr);

    // 预填已有记录（用于「修改账单」），标题会变成「修改账单」
    void setExpense(const Expense &e);

    // 用户点「保存」后取出这条记录的数据
    Expense expense() const;

private slots:
    // 一级大类改变时，刷新二级小类列表
    void onTopCategoryChanged();

private:
    int currentType() const;        // 当前类型：1 = 支出，2 = 收入
    void reloadCategories(int type); // 类型切换后刷新分类与金额颜色

    QButtonGroup *m_typeGroup; // 支出/收入单选组
    QPushButton *m_expenseButton;
    QPushButton *m_incomeButton;
    QDoubleSpinBox *m_amount;  // 金额输入框
    QComboBox *m_topCategory;  // 一级大类下拉框
    QComboBox *m_subCategory;  // 二级小类下拉框
    QDateEdit *m_date;         // 日期选择框
    QLineEdit *m_note;         // 备注输入框
};
