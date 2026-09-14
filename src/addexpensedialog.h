// ==========================================
// 黑马记账 - 「记一笔」输入弹窗（新增 / 修改共用）
// ==========================================
#pragma once

#include "database.h"

#include <QDialog>

class QComboBox;
class QDateEdit;
class QDoubleSpinBox;
class QLineEdit;

// 输入一笔账的弹窗：金额、两级分类、日期、备注
class AddExpenseDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddExpenseDialog(QWidget *parent = nullptr);

    // 预填已有账单（用于「修改账单」），标题会变成「修改账单」
    void setExpense(const Expense &e);

    // 用户点「保存」后取出这笔账的数据
    Expense expense() const;

private slots:
    // 一级大类改变时，刷新二级小类列表
    void onTopCategoryChanged();

private:
    QDoubleSpinBox *m_amount;   // 金额输入框
    QComboBox *m_topCategory;   // 一级大类下拉框
    QComboBox *m_subCategory;   // 二级小类下拉框
    QDateEdit *m_date;          // 日期选择框
    QLineEdit *m_note;          // 备注输入框
};
