// ==========================================
// 黑马记账 - 「记一笔」输入弹窗
// ==========================================
#include "addexpensedialog.h"
#include "categories.h"

#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

AddExpenseDialog::AddExpenseDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("记一笔"));
    setMinimumWidth(360);

    // 金额输入框：范围 0 ~ 999999.99，两位小数，带 ¥ 前缀
    m_amount = new QDoubleSpinBox(this);
    m_amount->setRange(0.0, 999999.99);
    m_amount->setDecimals(2);
    m_amount->setPrefix(QStringLiteral("¥ "));
    m_amount->setAlignment(Qt::AlignRight);

    // 一级大类下拉框
    m_topCategory = new QComboBox(this);
    m_topCategory->addItems(Categories::topLevels());

    // 二级小类下拉框（内容跟随一级大类）
    m_subCategory = new QComboBox(this);
    onTopCategoryChanged();

    // 日期选择框（默认今天，可点日历修改）
    m_date = new QDateEdit(QDate::currentDate(), this);
    m_date->setCalendarPopup(true);
    m_date->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));

    // 备注输入框
    m_note = new QLineEdit(this);
    m_note->setPlaceholderText(QStringLiteral("选填，例如：和朋友聚餐"));

    // 表单布局
    auto *form = new QFormLayout;
    form->addRow(QStringLiteral("金额"), m_amount);
    form->addRow(QStringLiteral("一级分类"), m_topCategory);
    form->addRow(QStringLiteral("二级分类"), m_subCategory);
    form->addRow(QStringLiteral("日期"), m_date);
    form->addRow(QStringLiteral("备注"), m_note);

    // 保存 / 取消按钮
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Save)->setText(QStringLiteral("保存"));
    buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        if (m_amount->value() <= 0.0) { // 金额必须大于 0
            QMessageBox::information(this, QStringLiteral("提示"),
                                     QStringLiteral("请先输入金额。"));
            return;
        }
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);

    // 一级大类改变时刷新二级小类
    connect(m_topCategory, &QComboBox::currentIndexChanged,
            this, &AddExpenseDialog::onTopCategoryChanged);
}

void AddExpenseDialog::onTopCategoryChanged()
{
    const QString top = m_topCategory->currentText();
    m_subCategory->clear();
    m_subCategory->addItems(Categories::subcategories(top));
}

Expense AddExpenseDialog::expense() const
{
    Expense e;
    // 金额换算成"分"存储，避免小数误差
    e.amountCents = qRound64(m_amount->value() * 100.0);
    e.category = m_topCategory->currentText();
    e.subcategory = m_subCategory->currentText();
    e.date = m_date->date().toString(QStringLiteral("yyyy-MM-dd"));
    e.note = m_note->text().trimmed();
    return e;
}
