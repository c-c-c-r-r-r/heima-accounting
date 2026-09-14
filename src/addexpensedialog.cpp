// ==========================================
// 黑马记账 - 「记一笔」输入弹窗（新增 / 修改共用）
// ==========================================
#include "addexpensedialog.h"
#include "categories.h"
#include "theme.h"

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
    setWindowTitle(tr("💰 记一笔"));
    setMinimumWidth(480);

    // 整体样式：暖色背景 + 大号控件（颜色跟随主题）
    const QString primary = Theme::primary().name();
    const QString hover = Theme::primaryHover().name();
    const QString pressed = Theme::primaryPressed().name();
    const QString border = Theme::border().name();
    setStyleSheet(QStringLiteral(
        "QDialog { background: #fffdf9; }"
        "QLabel { font-size: 12pt; }"
        "QComboBox { font-size: 13pt; padding: 9px 12px; min-height: 28px; }"
        "QComboBox QAbstractItemView { font-size: 13pt; }" // 下拉列表也加大
        "QDoubleSpinBox { font-size: 15pt; font-weight: bold; color: %1;"
        "                 padding: 9px 12px; min-height: 28px; }"
        "QDateEdit { font-size: 13pt; padding: 9px 12px; min-height: 28px; }"
        "QLineEdit { font-size: 13pt; padding: 9px 12px; min-height: 28px; }")
        .arg(pressed));

    // 金额输入框：范围 0 ~ 999999.99，两位小数，带 ¥ 前缀
    m_amount = new QDoubleSpinBox(this);
    m_amount->setRange(0.0, 999999.99);
    m_amount->setDecimals(2);
    m_amount->setPrefix(QStringLiteral("¥ "));
    m_amount->setAlignment(Qt::AlignRight);

    // 一级大类下拉框：显示"图标 名称"，实际取值为名称
    m_topCategory = new QComboBox(this);
    for (const auto &entry : Categories::topCategories())
        m_topCategory->addItem(entry.emoji + QStringLiteral(" ") + entry.name, entry.name);

    // 二级小类下拉框（内容跟随一级大类）
    m_subCategory = new QComboBox(this);
    onTopCategoryChanged();

    // 日期选择框（默认今天，可点日历修改）
    m_date = new QDateEdit(QDate::currentDate(), this);
    m_date->setCalendarPopup(true);
    m_date->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));

    // 备注输入框
    m_note = new QLineEdit(this);
    m_note->setPlaceholderText(tr("选填，例如：和朋友聚餐"));

    // 表单布局（行距加大）
    auto *form = new QFormLayout;
    form->setVerticalSpacing(16);
    form->addRow(tr("金额"), m_amount);
    form->addRow(tr("一级分类"), m_topCategory);
    form->addRow(tr("二级分类"), m_subCategory);
    form->addRow(tr("日期"), m_date);
    form->addRow(tr("备注"), m_note);

    // 保存 / 取消按钮（大号、好点击）
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    auto *saveButton = buttons->button(QDialogButtonBox::Save);
    saveButton->setText(tr("保存"));
    saveButton->setCursor(Qt::PointingHandCursor);
    saveButton->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: %1; color: white; font-size: 14pt;"
        "               font-weight: bold; padding: 12px 36px; border: none;"
        "               border-radius: 8px; }"
        "QPushButton:hover { background-color: %2; }"
        "QPushButton:pressed { background-color: %3; }")
        .arg(primary, hover, pressed));
    auto *cancelButton = buttons->button(QDialogButtonBox::Cancel);
    cancelButton->setText(tr("取消"));
    cancelButton->setStyleSheet(QStringLiteral(
        "QPushButton { font-size: 14pt; padding: 12px 36px; background: #ffffff;"
        "               border: 1px solid %1; border-radius: 8px; }"
        "QPushButton:hover { background: %2; }")
        .arg(border, Theme::lightBg().name()));
    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        if (m_amount->value() <= 0.0) { // 金额必须大于 0
            QMessageBox::information(this, tr("提示"), tr("请先输入金额。"));
            return;
        }
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 24, 28, 20);
    layout->addLayout(form);
    layout->addWidget(buttons);

    // 一级大类改变时刷新二级小类
    connect(m_topCategory, &QComboBox::currentIndexChanged,
            this, &AddExpenseDialog::onTopCategoryChanged);
}

void AddExpenseDialog::setExpense(const Expense &e)
{
    setWindowTitle(tr("✏️ 修改账单"));
    m_amount->setValue(e.amountCents / 100.0);

    // 选中一级大类（用 data 里的纯名称匹配）
    const int topIndex = m_topCategory->findData(e.category);
    if (topIndex >= 0)
        m_topCategory->setCurrentIndex(topIndex);

    // 选中二级小类
    const int subIndex = m_subCategory->findText(e.subcategory);
    if (subIndex >= 0)
        m_subCategory->setCurrentIndex(subIndex);

    m_date->setDate(QDate::fromString(e.date, QStringLiteral("yyyy-MM-dd")));
    m_note->setText(e.note);
}

void AddExpenseDialog::onTopCategoryChanged()
{
    // 注意用 currentData（纯名称）而不是 currentText（带图标）
    const QString top = m_topCategory->currentData().toString();
    m_subCategory->clear();
    m_subCategory->addItems(Categories::subcategories(top));
}

Expense AddExpenseDialog::expense() const
{
    Expense e;
    // 金额换算成"分"存储，避免小数误差
    e.amountCents = qRound64(m_amount->value() * 100.0);
    e.category = m_topCategory->currentData().toString(); // 存纯名称，不带图标
    e.subcategory = m_subCategory->currentText();
    e.date = m_date->date().toString(QStringLiteral("yyyy-MM-dd"));
    e.note = m_note->text().trimmed();
    return e;
}
