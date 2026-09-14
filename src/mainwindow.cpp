// ==========================================
// 黑马记账 - 主窗口
// ==========================================
#include "mainwindow.h"
#include "addexpensedialog.h"
#include "categories.h"

#include <QColor>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>
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

} // namespace

MainWindow::MainWindow(Database *db, QWidget *parent)
    : QMainWindow(parent)
    , m_db(db)
{
    setWindowTitle(QStringLiteral("黑马记账"));
    resize(960, 640);

    // 整体暖色主题
    setStyleSheet(QStringLiteral("QMainWindow { background: #fffdf9; }"));

    // 顶部工具栏：显眼的「记一笔」大按钮
    QToolBar *toolbar = addToolBar(QStringLiteral("主工具栏"));
    toolbar->setMovable(false);
    toolbar->setStyleSheet(QStringLiteral(
        "QToolBar { background: #fff7ef; padding: 10px;"
        "            border-bottom: 1px solid #ffe3c2; }"));

    auto *addButton = new QPushButton(QStringLiteral("＋ 记一笔"), this);
    addButton->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background-color: #ff7a1a;"   // 醒目橙色
        "  color: white;"                // 白色文字
        "  font-size: 17pt;"             // 大号字体
        "  font-weight: bold;"           // 加粗
        "  padding: 10px 36px;"          // 加大按钮面积
        "  border: none;"
        "  border-radius: 8px;"          // 圆角
        "}"
        "QPushButton:hover { background-color: #ff8f3d; }"     // 鼠标悬停变亮
        "QPushButton:pressed { background-color: #e56a10; }")); // 按下变深
    addButton->setCursor(Qt::PointingHandCursor); // 鼠标移上去变成小手
    toolbar->addWidget(addButton);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddExpense);

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
