// ==========================================
// 黑马记账 - 回收站（恢复误删的账单）
// ==========================================
#include "recyclebindialog.h"
#include "categories.h"
#include "theme.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace {

// 金额格式化：分 → "¥ 12.34"
QString formatAmount(qint64 cents)
{
    return QStringLiteral("¥ %1.%2")
        .arg(cents / 100)
        .arg(cents % 100, 2, 10, QLatin1Char('0'));
}

} // namespace

RecycleBinDialog::RecycleBinDialog(Database *db, QWidget *parent)
    : QDialog(parent)
    , m_db(db)
{
    setWindowTitle(tr("🗑️ 回收站"));
    setMinimumWidth(560);
    setStyleSheet(QStringLiteral(
        "QDialog { background: #fffdf9; }"
        "QLabel { font-size: 12pt; }"
        "QPushButton { font-size: 13pt; padding: 9px 22px; background: #ffffff;"
        "               color: #55504a; border: 1px solid %1; border-radius: 8px; }"
        "QPushButton:hover { background: %2; }")
        .arg(Theme::border().name(), Theme::lightBg().name()));

    // 说明文字
    auto *hintLabel = new QLabel(
        tr("删除的账单会先放在这里，可随时恢复；彻底删除后无法找回。"), this);
    hintLabel->setWordWrap(true);
    hintLabel->setStyleSheet(QStringLiteral("color: #8a7a66; font-size: 11pt;"));

    // 空回收站提示
    m_emptyLabel = new QLabel(tr("回收站是空的 🎉"), this);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet(QStringLiteral("color: #c9b8a3; font-size: 14pt;"));

    // 列表：日期 / 分类 / 金额
    m_table = new QTableWidget(this);
    m_table->setColumnCount(3);
    m_table->setHorizontalHeaderLabels({tr("日期"), tr("分类"), tr("金额")});
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setDefaultSectionSize(42);
    m_table->setColumnWidth(0, 120);
    m_table->setColumnWidth(1, 220);
    m_table->setStyleSheet(QStringLiteral(
        "QTableWidget { font-size: 11pt; background: #ffffff;"
        "                alternate-background-color: %1; border: none; }"
        "QHeaderView::section { background: %1; font-weight: bold; font-size: 11pt;"
        "                       padding: 8px; border: none; border-bottom: 2px solid %2; }")
        .arg(Theme::lightBg().name(), Theme::border().name()));

    // 操作按钮
    auto *restoreButton = new QPushButton(tr("♻️ 恢复所选"), this);
    restoreButton->setCursor(Qt::PointingHandCursor);
    connect(restoreButton, &QPushButton::clicked, this, &RecycleBinDialog::restoreSelected);

    auto *purgeButton = new QPushButton(tr("彻底删除"), this);
    purgeButton->setCursor(Qt::PointingHandCursor);
    purgeButton->setStyleSheet(QStringLiteral(
        "QPushButton { background: #fffaf8; color: #c0392b; font-size: 13pt;"
        "               padding: 9px 22px; border: 1px solid #e6b3ae; border-radius: 8px; }"
        "QPushButton:hover { background: #fdece9; }"));
    connect(purgeButton, &QPushButton::clicked, this, &RecycleBinDialog::purgeSelected);

    auto *purgeAllButton = new QPushButton(tr("清空回收站"), this);
    purgeAllButton->setCursor(Qt::PointingHandCursor);
    connect(purgeAllButton, &QPushButton::clicked, this, &RecycleBinDialog::purgeAll);

    auto *closeButton = new QPushButton(tr("关闭"), this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);

    auto *buttonRow = new QHBoxLayout;
    buttonRow->addWidget(restoreButton);
    buttonRow->addWidget(purgeButton);
    buttonRow->addWidget(purgeAllButton);
    buttonRow->addStretch();
    buttonRow->addWidget(closeButton);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 16);
    layout->addWidget(hintLabel);
    layout->addSpacing(8);
    layout->addWidget(m_table);
    layout->addWidget(m_emptyLabel);
    layout->addLayout(buttonRow);
    resize(620, 480);

    refresh();
}

void RecycleBinDialog::restoreSelected()
{
    const int row = m_table->currentRow();
    if (row < 0 || row >= m_list.size())
        return;
    if (m_db->restoreExpense(m_list[row].id))
        refresh();
}

void RecycleBinDialog::purgeSelected()
{
    const int row = m_table->currentRow();
    if (row < 0 || row >= m_list.size())
        return;

    // 彻底删除必须确认（不可恢复）
    QMessageBox box(QMessageBox::Question, tr("彻底删除"),
                    tr("确定要彻底删除选中的这笔账吗？此操作无法恢复。"),
                    QMessageBox::NoButton, this);
    QPushButton *purgeButton = box.addButton(tr("彻底删除"), QMessageBox::DestructiveRole);
    box.addButton(tr("取消"), QMessageBox::RejectRole);
    box.exec();
    if (box.clickedButton() == purgeButton && m_db->purgeExpense(m_list[row].id))
        refresh();
}

void RecycleBinDialog::purgeAll()
{
    if (m_list.isEmpty())
        return;

    QMessageBox box(QMessageBox::Question, tr("清空回收站"),
                    tr("确定要清空回收站吗？所有账单将永久删除。"),
                    QMessageBox::NoButton, this);
    QPushButton *purgeButton = box.addButton(tr("清空"), QMessageBox::DestructiveRole);
    box.addButton(tr("取消"), QMessageBox::RejectRole);
    box.exec();
    if (box.clickedButton() == purgeButton && m_db->purgeAllDeleted())
        refresh();
}

void RecycleBinDialog::refresh()
{
    m_list = m_db->deletedExpenses();

    m_table->setRowCount(m_list.size());
    for (int i = 0; i < m_list.size(); ++i) {
        const Expense &e = m_list[i];

        auto *dateItem = new QTableWidgetItem(e.date);
        dateItem->setData(Qt::UserRole, e.id);
        auto *catItem = new QTableWidgetItem(
            Categories::emojiForTop(e.category) + QStringLiteral(" ") + e.category
            + QStringLiteral(" / ") + e.subcategory);
        auto *amountItem = new QTableWidgetItem(formatAmount(e.amountCents));
        amountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        amountItem->setForeground(Theme::amountText());

        m_table->setItem(i, 0, dateItem);
        m_table->setItem(i, 1, catItem);
        m_table->setItem(i, 2, amountItem);
    }

    // 空状态
    m_emptyLabel->setVisible(m_list.isEmpty());
    m_table->setVisible(!m_list.isEmpty());
}
