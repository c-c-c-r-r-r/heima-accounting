// ==========================================
// 黑马记账 - 「记一笔」输入弹窗（新增 / 修改共用，支持支出与收入）
// ==========================================
#include "addexpensedialog.h"
#include "categories.h"
#include "theme.h"

#include <QButtonGroup>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace {

// 收入固定用绿色（与支出颜色区分）
const QColor kIncomeColor = QColor(QStringLiteral("#16a34a"));
const QColor kIncomeHover = QColor(QStringLiteral("#22b95c"));
const QColor kIncomePressed = QColor(QStringLiteral("#128a3f"));

// 类型切换按钮样式：选中时填充对应的颜色
QString typeButtonStyle(const QColor &fill, const QColor &hover, const QColor &pressed)
{
    return QStringLiteral(
        "QPushButton { background: #ffffff; color: #55504a; font-size: 13pt;"
        "               padding: 9px 30px; border: 1px solid #d9cfc0; border-radius: 8px; }"
        "QPushButton:checked { background: %1; color: white; font-weight: bold;"
        "                       border: 1px solid %1; }"
        "QPushButton:checked:hover { background: %2; }"
        "QPushButton:checked:pressed { background: %3; }")
        .arg(fill.name(), hover.name(), pressed.name());
}

} // namespace

AddExpenseDialog::AddExpenseDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("💰 记一笔"));
    setMinimumWidth(480);

    // 整体样式：暖色背景 + 大号控件
    setStyleSheet(QStringLiteral(
        "QDialog { background: #fffdf9; }"
        "QLabel { font-size: 12pt; }"
        "QComboBox { font-size: 13pt; padding: 9px 12px; min-height: 28px; }"
        "QComboBox QAbstractItemView { font-size: 13pt; }" // 下拉列表也加大
        "QDoubleSpinBox { font-size: 15pt; font-weight: bold; padding: 9px 12px;"
        "                 min-height: 28px; }"
        "QDateEdit { font-size: 13pt; padding: 9px 12px; min-height: 28px; }"
        "QLineEdit { font-size: 13pt; padding: 9px 12px; min-height: 28px; }"));

    // —— 类型切换：支出 / 收入 ——
    m_expenseButton = new QPushButton(tr("💸 支出"), this);
    m_incomeButton = new QPushButton(tr("💰 收入"), this);
    m_expenseButton->setCheckable(true);
    m_incomeButton->setCheckable(true);
    m_expenseButton->setChecked(true);
    m_expenseButton->setCursor(Qt::PointingHandCursor);
    m_incomeButton->setCursor(Qt::PointingHandCursor);
    m_expenseButton->setStyleSheet(typeButtonStyle(
        Theme::primary(), Theme::primaryHover(), Theme::primaryPressed()));
    m_incomeButton->setStyleSheet(typeButtonStyle(kIncomeColor, kIncomeHover, kIncomePressed));

    m_typeGroup = new QButtonGroup(this);
    m_typeGroup->setExclusive(true);
    m_typeGroup->addButton(m_expenseButton, 1);
    m_typeGroup->addButton(m_incomeButton, 2);
    connect(m_typeGroup, &QButtonGroup::idClicked,
            this, &AddExpenseDialog::reloadCategories);

    auto *typeRow = new QHBoxLayout;
    typeRow->addWidget(m_expenseButton);
    typeRow->addWidget(m_incomeButton);
    typeRow->addStretch();

    // 金额输入框：范围 0 ~ 9999999.99，两位小数，带 ¥ 前缀
    m_amount = new QDoubleSpinBox(this);
    m_amount->setRange(0.0, 9999999.99);
    m_amount->setDecimals(2);
    m_amount->setPrefix(QStringLiteral("¥ "));
    m_amount->setAlignment(Qt::AlignRight);

    // 一级大类下拉框：显示"图标 名称"，实际取值为名称
    m_topCategory = new QComboBox(this);

    // 二级小类下拉框（内容跟随一级大类）
    m_subCategory = new QComboBox(this);
    reloadCategories(currentType());

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
        .arg(Theme::primary().name(), Theme::primaryHover().name(),
             Theme::primaryPressed().name()));
    auto *cancelButton = buttons->button(QDialogButtonBox::Cancel);
    cancelButton->setText(tr("取消"));
    cancelButton->setStyleSheet(QStringLiteral(
        "QPushButton { font-size: 14pt; padding: 12px 36px; background: #ffffff;"
        "               border: 1px solid %1; border-radius: 8px; }"
        "QPushButton:hover { background: %2; }")
        .arg(Theme::border().name(), Theme::lightBg().name()));
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
    layout->addLayout(typeRow);
    layout->addSpacing(12);
    layout->addLayout(form);
    layout->addWidget(buttons);

    // 一级大类改变时刷新二级小类
    connect(m_topCategory, &QComboBox::currentIndexChanged,
            this, &AddExpenseDialog::onTopCategoryChanged);
}

int AddExpenseDialog::currentType() const
{
    return m_typeGroup->checkedId();
}

void AddExpenseDialog::reloadCategories(int type)
{
    // 一级大类列表按类型切换
    m_topCategory->clear();
    for (const auto &entry : Categories::topCategories(type))
        m_topCategory->addItem(entry.emoji + QStringLiteral(" ") + entry.name, entry.name);
    onTopCategoryChanged();

    // 金额颜色跟随类型：支出用主题色，收入用绿色
    const QColor color = (type == 2) ? kIncomeColor : Theme::amountText();
    m_amount->setStyleSheet(QStringLiteral(
        "QDoubleSpinBox { font-size: 15pt; font-weight: bold; color: %1;"
        "                 padding: 9px 12px; min-height: 28px; }")
        .arg(color.name()));
}

void AddExpenseDialog::setExpense(const Expense &e)
{
    setWindowTitle(tr("✏️ 修改账单"));

    // 选中类型（程序性切换不触发 idClicked，需手动刷新分类）
    if (e.type == 2)
        m_incomeButton->setChecked(true);
    else
        m_expenseButton->setChecked(true);
    reloadCategories(e.type);

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
    m_subCategory->addItems(Categories::subcategories(top, currentType()));
}

Expense AddExpenseDialog::expense() const
{
    Expense e;
    e.type = currentType();
    // 金额换算成"分"存储，避免小数误差
    e.amountCents = qRound64(m_amount->value() * 100.0);
    e.category = m_topCategory->currentData().toString(); // 存纯名称，不带图标
    e.subcategory = m_subCategory->currentText();
    e.date = m_date->date().toString(QStringLiteral("yyyy-MM-dd"));
    e.note = m_note->text().trimmed();
    return e;
}
