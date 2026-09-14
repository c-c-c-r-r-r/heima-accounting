// ==========================================
// 黑马记账 - 主窗口
// ==========================================
#include "mainwindow.h"
#include "addexpensedialog.h"
#include "categories.h"
#include "statisticsdialog.h"

#include <QColor>
#include <QHeaderView>
#include <QKeySequence>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QResizeEvent>
#include <QShortcut>
#include <QSizePolicy>
#include <QStatusBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QToolBar>

namespace {

// 金额格式化：分 → "¥ 12.34"
QString formatAmount(qint64 cents)
{
    return QStringLiteral("¥ %1.%2")
        .arg(cents / 100)
        .arg(cents % 100, 2, 10, QLatin1Char('0'));
}

// 橙色主按钮样式
const char *kPrimaryButtonStyle =
    "QPushButton {"
    "  background-color: #ff7a1a; color: white; font-size: 17pt;"
    "  font-weight: bold; padding: 10px 36px; border: none; border-radius: 8px;"
    "}"
    "QPushButton:hover { background-color: #ff8f3d; }"
    "QPushButton:pressed { background-color: #e56a10; }";

} // namespace

MainWindow::MainWindow(Database *db, QWidget *parent)
    : QMainWindow(parent)
    , m_db(db)
{
    setWindowTitle(QStringLiteral("黑马记账"));
    resize(960, 640);

    // 整体暖色主题
    setStyleSheet(QStringLiteral("QMainWindow { background: #fffdf9; }"));

    // 顶部工具栏
    QToolBar *toolbar = addToolBar(QStringLiteral("主工具栏"));
    toolbar->setMovable(false);
    toolbar->setStyleSheet(QStringLiteral(
        "QToolBar { background: #fff7ef; padding: 10px;"
        "            border-bottom: 1px solid #ffe3c2; }"));

    // 「记一笔」大按钮
    auto *addButton = new QPushButton(QStringLiteral("＋ 记一笔"), this);
    addButton->setStyleSheet(QString::fromUtf8(kPrimaryButtonStyle));
    addButton->setCursor(Qt::PointingHandCursor);
    toolbar->addWidget(addButton);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddExpense);

    // 「统计」按钮（橙色描边样式）
    auto *statsButton = new QPushButton(QStringLiteral("📊 统计"), this);
    statsButton->setStyleSheet(QStringLiteral(
        "QPushButton { background: #ffffff; color: #ff7a1a; font-size: 14pt;"
        "               font-weight: bold; padding: 9px 28px;"
        "               border: 2px solid #ff7a1a; border-radius: 8px; }"
        "QPushButton:hover { background: #fff3e4; }"
        "QPushButton:pressed { background: #ffe3c2; }"));
    statsButton->setCursor(Qt::PointingHandCursor);
    toolbar->addWidget(statsButton);
    connect(statsButton, &QPushButton::clicked, this, &MainWindow::onShowStats);

    // 弹性空隙，把删除按钮推到最右侧
    auto *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolbar->addWidget(spacer);

    // 「删除所选」按钮（选中账单后才能用）
    m_deleteButton = new QPushButton(QStringLiteral("🗑️ 删除所选"), this);
    m_deleteButton->setEnabled(false);
    m_deleteButton->setStyleSheet(QStringLiteral(
        "QPushButton { background: #ffffff; color: #c9c2b8; font-size: 14pt;"
        "               padding: 9px 28px; border: 1px solid #e2d8c9; border-radius: 8px; }"
        "QPushButton:enabled { color: #c0392b; border-color: #e6b3ae; background: #fffaf8; }"
        "QPushButton:enabled:hover { background: #fdece9; }"));
    toolbar->addWidget(m_deleteButton);
    connect(m_deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteSelected);

    // 按键盘 Delete 键也可以删除所选
    auto *deleteShortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(deleteShortcut, &QShortcut::activated, this, &MainWindow::onDeleteSelected);

    // 中部账单列表（4 列：日期 / 分类 / 备注 / 金额）
    m_table = new QTableWidget(this);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels(
        {QStringLiteral("日期"), QStringLiteral("分类"), QStringLiteral("备注"), QStringLiteral("金额")});
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);  // 列表只读
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows); // 整行选中
    m_table->verticalHeader()->setVisible(false);                 // 隐藏行号
    m_table->horizontalHeader()->setStretchLastSection(true);     // 最后一列自动拉伸
    m_table->setAlternatingRowColors(true);                       // 隔行换色
    m_table->verticalHeader()->setDefaultSectionSize(46);         // 行高加大，看着更舒服
    m_table->setStyleSheet(QStringLiteral(
        "QTableWidget {"
        "  font-size: 11pt;"
        "  background: #ffffff;"
        "  alternate-background-color: #fdf3e6;" // 隔行浅橙
        "  border: none;"
        "}"
        "QHeaderView::section {"
        "  background: #fff3e4;"
        "  font-weight: bold;"
        "  font-size: 11pt;"
        "  padding: 8px;"
        "  border: none;"
        "  border-bottom: 2px solid #ffd9b0;"
        "}"));
    m_table->setColumnWidth(0, 120); // 日期
    m_table->setColumnWidth(1, 190); // 分类
    m_table->setColumnWidth(2, 350); // 备注
    setCentralWidget(m_table);

    // 选中行变化时更新「删除所选」按钮状态
    connect(m_table, &QTableWidget::itemSelectionChanged, this, [this]() {
        m_deleteButton->setEnabled(m_table->currentRow() >= 0);
    });

    // 没有账单时的提示文字（覆盖在列表上方）
    m_emptyLabel = new QLabel(
        QStringLiteral("还没有账单记录 🐎\n点击左上角「＋ 记一笔」开始记账吧！"), m_table);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet(QStringLiteral("color: #c9b8a3; font-size: 15pt;"));

    // 底部汇总栏（显示在状态栏右侧，橙色高亮）
    m_totalLabel = new QLabel(this);
    m_totalLabel->setStyleSheet(QStringLiteral(
        "color: #e56a10; font-size: 12pt; font-weight: bold; padding: 4px 12px;"));
    statusBar()->setStyleSheet(QStringLiteral(
        "QStatusBar { background: #fff7ef; border-top: 1px solid #ffe3c2; }"));
    statusBar()->addPermanentWidget(m_totalLabel);

    // 首次加载数据
    refresh();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    // 空状态提示始终盖满整个列表区域
    if (m_table && m_emptyLabel)
        m_emptyLabel->setGeometry(m_table->rect());
}

void MainWindow::onAddExpense()
{
    // 打开「记一笔」弹窗
    AddExpenseDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // 用户点了保存：写入数据库并刷新列表
        if (m_db->addExpense(dialog.expense()))
            refresh();
    }
}

void MainWindow::onShowStats()
{
    // 打开支出统计页面
    StatisticsDialog dialog(m_db, this);
    dialog.exec();
}

void MainWindow::onDeleteSelected()
{
    const int row = m_table->currentRow();
    if (row < 0)
        return;

    // 第一列的 UserRole 里存了这笔账的编号
    const qint64 id = m_table->item(row, 0)->data(Qt::UserRole).toLongLong();
    const QString date = m_table->item(row, 0)->text();
    const QString cat = m_table->item(row, 1)->text();
    const QString amount = m_table->item(row, 3)->text();

    // 删除前先确认，防止误删
    QMessageBox box(QMessageBox::Question, QStringLiteral("删除账单"),
                    QStringLiteral("确定要删除这笔账吗？\n\n%1\n%2\n%3").arg(date, cat, amount),
                    QMessageBox::NoButton, this);
    QPushButton *deleteButton = box.addButton(QStringLiteral("删除"), QMessageBox::DestructiveRole);
    box.addButton(QStringLiteral("取消"), QMessageBox::RejectRole);
    box.exec();

    if (box.clickedButton() == deleteButton) {
        if (m_db->deleteExpense(id))
            refresh();
    }
}

void MainWindow::refresh()
{
    const QList<Expense> list = m_db->allExpenses();

    // 填充列表
    m_table->setRowCount(list.size());
    for (int i = 0; i < list.size(); ++i) {
        const Expense &e = list[i];

        // 分类列带图标：如 "🍜 餐饮饮食 / 午餐"
        const QString catText = Categories::emojiForTop(e.category)
            + QStringLiteral(" ") + e.category
            + QStringLiteral(" / ") + e.subcategory;

        auto *dateItem = new QTableWidgetItem(e.date);
        dateItem->setData(Qt::UserRole, e.id); // 在行里记住这笔账的编号，删除时用
        auto *catItem = new QTableWidgetItem(catText);
        auto *noteItem = new QTableWidgetItem(e.note);
        auto *amountItem = new QTableWidgetItem(formatAmount(e.amountCents));

        // 金额加粗并显示为橙色
        amountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        amountItem->setForeground(QColor(QStringLiteral("#e56a10")));
        QFont amountFont = amountItem->font();
        amountFont.setBold(true);
        amountItem->setFont(amountFont);

        m_table->setItem(i, 0, dateItem);
        m_table->setItem(i, 1, catItem);
        m_table->setItem(i, 2, noteItem);
        m_table->setItem(i, 3, amountItem);
    }

    // 空状态提示：没有账单时显示
    if (m_emptyLabel) {
        m_emptyLabel->setGeometry(m_table->rect());
        m_emptyLabel->setVisible(list.isEmpty());
    }

    // 底部总支出
    m_totalLabel->setText(QStringLiteral("共 %1 笔，总支出：%2")
                              .arg(list.size())
                              .arg(formatAmount(m_db->totalCents())));
}
