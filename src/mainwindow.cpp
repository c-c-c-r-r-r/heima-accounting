// ==========================================
// 黑马记账 - 主窗口
// ==========================================
#include "mainwindow.h"
#include "addexpensedialog.h"

#include <QAction>
#include <QHeaderView>
#include <QLabel>
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
    resize(900, 600);

    // 顶部工具栏：「记一笔」按钮
    QToolBar *toolbar = addToolBar(QStringLiteral("主工具栏"));
    toolbar->setMovable(false);
    QAction *addAction = toolbar->addAction(QStringLiteral("＋ 记一笔"));
    connect(addAction, &QAction::triggered, this, &MainWindow::onAddExpense);

    // 中部账单列表（4 列：日期 / 分类 / 备注 / 金额）
    m_table = new QTableWidget(this);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels(
        {QStringLiteral("日期"), QStringLiteral("分类"), QStringLiteral("备注"), QStringLiteral("金额")});
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);  // 列表只读
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows); // 整行选中
    m_table->verticalHeader()->setVisible(false);                 // 隐藏行号
    m_table->horizontalHeader()->setStretchLastSection(true);     // 最后一列自动拉伸
    m_table->setColumnWidth(0, 120); // 日期
    m_table->setColumnWidth(1, 170); // 分类
    m_table->setColumnWidth(2, 330); // 备注
    setCentralWidget(m_table);

    // 底部汇总栏（显示在状态栏右侧）
    m_totalLabel = new QLabel(this);
    statusBar()->addPermanentWidget(m_totalLabel);

    // 首次加载数据
    refresh();
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

        auto *dateItem = new QTableWidgetItem(e.date);
        auto *catItem = new QTableWidgetItem(e.category + QStringLiteral(" / ") + e.subcategory);
        auto *noteItem = new QTableWidgetItem(e.note);
        auto *amountItem = new QTableWidgetItem(formatAmount(e.amountCents));
        amountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        m_table->setItem(i, 0, dateItem);
        m_table->setItem(i, 1, catItem);
        m_table->setItem(i, 2, noteItem);
        m_table->setItem(i, 3, amountItem);
    }

    // 底部总支出
    m_totalLabel->setText(QStringLiteral("共 %1 笔，总支出：%2")
                              .arg(list.size())
                              .arg(formatAmount(m_db->totalCents())));
}
