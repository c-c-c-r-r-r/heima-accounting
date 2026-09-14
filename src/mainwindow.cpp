// ==========================================
// 黑马记账 - 主窗口
// ==========================================
#include "mainwindow.h"
#include "addexpensedialog.h"
#include "categories.h"
#include "settingsdialog.h"
#include "statisticsdialog.h"
#include "theme.h"

#include <QColor>
#include <QComboBox>
#include <QDate>
#include <QHeaderView>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QResizeEvent>
#include <QSettings>
#include <QShortcut>
#include <QSizePolicy>
#include <QStatusBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QToolBar>

namespace {

// 金额格式化：分 → "¥ 12.34"（金额格式不参与翻译）
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
    setWindowTitle(tr("黑马记账"));
    resize(1000, 660);

    // —— 顶部工具栏（第一行）——
    QToolBar *toolbar = addToolBar(tr("主工具栏"));
    toolbar->setMovable(false);

    m_addButton = new QPushButton(tr("＋ 记一笔"), this);
    m_addButton->setCursor(Qt::PointingHandCursor);
    toolbar->addWidget(m_addButton);
    connect(m_addButton, &QPushButton::clicked, this, &MainWindow::onAddExpense);

    m_statsButton = new QPushButton(tr("📊 统计"), this);
    m_statsButton->setCursor(Qt::PointingHandCursor);
    toolbar->addWidget(m_statsButton);
    connect(m_statsButton, &QPushButton::clicked, this, &MainWindow::onShowStats);

    m_settingsButton = new QPushButton(tr("⚙️ 设置"), this);
    m_settingsButton->setCursor(Qt::PointingHandCursor);
    toolbar->addWidget(m_settingsButton);
    connect(m_settingsButton, &QPushButton::clicked, this, &MainWindow::onShowSettings);

    // 弹性空隙，把修改/删除推到最右侧
    auto *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolbar->addWidget(spacer);

    m_editButton = new QPushButton(tr("✏️ 修改"), this);
    m_editButton->setCursor(Qt::PointingHandCursor);
    toolbar->addWidget(m_editButton);
    connect(m_editButton, &QPushButton::clicked, this, &MainWindow::onEditSelected);

    m_deleteButton = new QPushButton(tr("🗑️ 删除"), this);
    m_deleteButton->setCursor(Qt::PointingHandCursor);
    toolbar->addWidget(m_deleteButton);
    connect(m_deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteButton);

    m_cancelDeleteButton = new QPushButton(tr("取消删除"), this);
    m_cancelDeleteButton->setVisible(false);
    toolbar->addWidget(m_cancelDeleteButton);
    connect(m_cancelDeleteButton, &QPushButton::clicked, this, &MainWindow::onCancelDelete);

    // 按键盘 Delete 键：进入勾选模式 / 确认删除
    auto *deleteShortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(deleteShortcut, &QShortcut::activated, this, &MainWindow::onDeleteButton);

    // —— 搜索栏（第二行）——
    addToolBarBreak();
    QToolBar *searchBar = addToolBar(tr("搜索栏"));
    searchBar->setMovable(false);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("🔍 搜索金额 / 日期 / 分类 / 备注"));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setMinimumWidth(320);
    searchBar->addWidget(m_searchEdit);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::refresh);

    m_categoryFilter = new QComboBox(this);
    m_categoryFilter->addItem(tr("全部分类"), QString());
    for (const auto &entry : Categories::topCategories())
        m_categoryFilter->addItem(entry.emoji + QStringLiteral(" ") + entry.name, entry.name);
    searchBar->addWidget(m_categoryFilter);
    connect(m_categoryFilter, &QComboBox::currentIndexChanged, this, &MainWindow::refresh);

    m_clearButton = new QPushButton(tr("清空"), this);
    searchBar->addWidget(m_clearButton);
    connect(m_clearButton, &QPushButton::clicked, this, [this]() {
        m_searchEdit->clear();
        m_categoryFilter->setCurrentIndex(0);
    });

    // —— 中部账单列表 ——
    m_table = new QTableWidget(this);
    m_table->setColumnCount(5); // 第 0 列勾选（平时隐藏），1~4 日期/分类/备注/金额
    m_table->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("选择")));
    m_table->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("日期")));
    m_table->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("分类")));
    m_table->setHorizontalHeaderItem(3, new QTableWidgetItem(tr("备注")));
    m_table->setHorizontalHeaderItem(4, new QTableWidgetItem(tr("金额")));
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setDefaultSectionSize(46);
    m_table->setColumnHidden(0, true); // 勾选列平时隐藏
    m_table->setColumnWidth(0, 70);
    m_table->setColumnWidth(1, 120); // 日期
    m_table->setColumnWidth(2, 190); // 分类
    m_table->setColumnWidth(3, 350); // 备注
    setCentralWidget(m_table);

    connect(m_table, &QTableWidget::itemChanged, this, &MainWindow::onItemChanged);
    connect(m_table, &QTableWidget::cellDoubleClicked, this,
            [this](int row, int) {
                if (!m_deleteMode)
                    editRow(row);
            });
    // 选中行变化时更新「删除」按钮的可用状态
    connect(m_table, &QTableWidget::itemSelectionChanged, this, [this]() {
        if (!m_deleteMode)
            m_deleteButton->setEnabled(m_table->currentRow() >= 0);
    });

    // 没有账单时的提示文字（覆盖在列表上方）
    m_emptyLabel = new QLabel(tr("还没有账单记录 🐎\n点击左上角「＋ 记一笔」开始记账吧！"), m_table);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet(QStringLiteral("color: #c9b8a3; font-size: 15pt;"));

    // —— 底部汇总栏 ——
    m_totalLabel = new QLabel(this);
    statusBar()->addPermanentWidget(m_totalLabel);

    // 应用主题 + 首次加载数据
    applyTheme();
    refresh();
}

void MainWindow::applyTheme()
{
    const QString p = Theme::primary().name();
    const QString h = Theme::primaryHover().name();
    const QString pr = Theme::primaryPressed().name();
    const QString lb = Theme::lightBg().name();
    const QString bd = Theme::border().name();
    const QString amt = Theme::amountText().name();

    setStyleSheet(QStringLiteral("QMainWindow { background: #fffdf9; }"));

    // 工具栏与搜索栏背景
    for (QToolBar *tb : findChildren<QToolBar *>()) {
        tb->setStyleSheet(QStringLiteral(
            "QToolBar { background: %1; padding: 10px; border-bottom: 1px solid %2; }")
            .arg(lb, bd));
    }

    // 主按钮（记一笔）：主色填充
    m_addButton->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: %1; color: white; font-size: 17pt;"
        "               font-weight: bold; padding: 10px 36px; border: none;"
        "               border-radius: 8px; }"
        "QPushButton:hover { background-color: %2; }"
        "QPushButton:pressed { background-color: %3; }")
        .arg(p, h, pr));

    // 描边按钮（统计 / 设置）：主色描边
    const QString outlineStyle = QStringLiteral(
        "QPushButton { background: #ffffff; color: %1; font-size: 14pt;"
        "               font-weight: bold; padding: 9px 28px;"
        "               border: 2px solid %1; border-radius: 8px; }"
        "QPushButton:hover { background: %2; }"
        "QPushButton:pressed { background: #ffffff; }")
        .arg(p, lb);
    m_statsButton->setStyleSheet(outlineStyle);
    m_settingsButton->setStyleSheet(outlineStyle);

    // 修改按钮：浅色描边
    m_editButton->setStyleSheet(QStringLiteral(
        "QPushButton { background: #ffffff; color: #55504a; font-size: 14pt;"
        "               padding: 9px 28px; border: 1px solid %1; border-radius: 8px; }"
        "QPushButton:hover { background: %2; }")
        .arg(bd, lb));

    // 删除按钮：平时灰色，可删除时偏红
    const bool deletable = (m_table->currentRow() >= 0);
    m_deleteButton->setEnabled(deletable);
    m_deleteButton->setStyleSheet(QStringLiteral(
        "QPushButton { background: #ffffff; color: #c9c2b8; font-size: 14pt;"
        "               padding: 9px 28px; border: 1px solid #e2d8c9; border-radius: 8px; }"
        "QPushButton:enabled { color: #c0392b; border-color: #e6b3ae; background: #fffaf8; }"
        "QPushButton:enabled:hover { background: #fdece9; }"));

    // 取消删除按钮：浅灰
    m_cancelDeleteButton->setStyleSheet(QStringLiteral(
        "QPushButton { background: #f7f4ee; color: #55504a; font-size: 14pt;"
        "               padding: 9px 28px; border: 1px solid %1; border-radius: 8px; }")
        .arg(bd));

    // 搜索栏控件
    m_searchEdit->setStyleSheet(QStringLiteral(
        "QLineEdit { font-size: 12pt; padding: 8px 12px; background: #ffffff;"
        "             border: 1px solid %1; border-radius: 8px; }")
        .arg(bd));
    m_categoryFilter->setStyleSheet(QStringLiteral(
        "QComboBox { font-size: 12pt; padding: 8px 12px; min-height: 24px; }"
        "QComboBox QAbstractItemView { font-size: 12pt; }"));
    m_clearButton->setStyleSheet(QStringLiteral(
        "QPushButton { background: #ffffff; color: #55504a; font-size: 12pt;"
        "               padding: 8px 20px; border: 1px solid %1; border-radius: 8px; }")
        .arg(bd));

    // 账单列表
    m_table->setStyleSheet(QStringLiteral(
        "QTableWidget {"
        "  font-size: 11pt;"
        "  background: #ffffff;"
        "  alternate-background-color: %1;" // 隔行浅色
        "  border: none;"
        "}"
        "QHeaderView::section {"
        "  background: %1;"
        "  font-weight: bold;"
        "  font-size: 11pt;"
        "  padding: 8px;"
        "  border: none;"
        "  border-bottom: 2px solid %2;"
        "}")
        .arg(lb, bd));

    // 底部汇总
    m_totalLabel->setStyleSheet(QStringLiteral(
        "color: %1; font-size: 12pt; font-weight: bold; padding: 4px 12px;").arg(amt));
    statusBar()->setStyleSheet(QStringLiteral(
        "QStatusBar { background: %1; border-top: 1px solid %2; }").arg(lb, bd));
}

QString MainWindow::dateFormat() const
{
    return QSettings().value(QStringLiteral("ui/dateFormat"), QStringLiteral("yyyy-MM-dd")).toString();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (m_table && m_emptyLabel)
        m_emptyLabel->setGeometry(m_table->rect());
}

void MainWindow::onAddExpense()
{
    AddExpenseDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        if (m_db->addExpense(dialog.expense()))
            refresh();
    }
}

void MainWindow::onShowStats()
{
    StatisticsDialog dialog(m_db, this);
    dialog.exec();
}

void MainWindow::onShowSettings()
{
    SettingsDialog dialog(m_db, this);
    dialog.exec();
    applyTheme(); // 主题色可能已改
    refresh();    // 日期格式可能已改
}

void MainWindow::onEditSelected()
{
    if (m_deleteMode)
        return;
    const int row = m_table->currentRow();
    if (row >= 0 && row < m_currentList.size())
        editRow(row);
}

void MainWindow::editRow(int row)
{
    const Expense &e = m_currentList[row];

    // 打开弹窗并预填这笔账
    AddExpenseDialog dialog(this);
    dialog.setExpense(e);
    if (dialog.exec() == QDialog::Accepted) {
        if (m_db->updateExpense(e.id, dialog.expense()))
            refresh();
    }
}

void MainWindow::onDeleteButton()
{
    if (m_deleteMode)
        confirmDelete();
    else
        enterDeleteMode();
}

void MainWindow::enterDeleteMode()
{
    if (m_table->rowCount() == 0)
        return; // 没有账单，无需删除

    m_deleteMode = true;
    m_table->setColumnHidden(0, false); // 显示勾选列

    m_deleteButton->setText(tr("确认删除(0)"));
    m_deleteButton->setEnabled(false); // 勾选后才能确认
    m_cancelDeleteButton->setVisible(true);

    // 删除模式下禁用其他操作，避免误点
    m_addButton->setEnabled(false);
    m_statsButton->setEnabled(false);
    m_settingsButton->setEnabled(false);
    m_editButton->setEnabled(false);
}

void MainWindow::exitDeleteMode()
{
    m_deleteMode = false;
    m_table->setColumnHidden(0, true); // 隐藏勾选列
    m_deleteButton->setText(tr("🗑️ 删除"));
    m_cancelDeleteButton->setVisible(false);

    m_addButton->setEnabled(true);
    m_statsButton->setEnabled(true);
    m_settingsButton->setEnabled(true);
    m_editButton->setEnabled(true);

    applyTheme(); // 恢复删除按钮的可用状态样式
}

void MainWindow::confirmDelete()
{
    // 收集勾选的账单编号
    QList<qint64> ids;
    for (int r = 0; r < m_table->rowCount(); ++r) {
        QTableWidgetItem *item = m_table->item(r, 0);
        if (item && item->checkState() == Qt::Checked)
            ids.append(m_currentList[r].id);
    }
    if (ids.isEmpty())
        return;

    // 确认框：防止误删，并提示可到回收站恢复
    QMessageBox box(QMessageBox::Question, tr("删除账单"),
                    tr("确定要删除选中的 %1 笔账吗？\n\n删除后可在「设置 → 回收站」中恢复。").arg(ids.size()),
                    QMessageBox::NoButton, this);
    QPushButton *deleteButton = box.addButton(tr("删除"), QMessageBox::DestructiveRole);
    box.addButton(tr("取消"), QMessageBox::RejectRole);
    box.exec();
    if (box.clickedButton() != deleteButton)
        return;

    for (qint64 id : ids)
        m_db->deleteExpense(id);

    exitDeleteMode();
    refresh();
}

void MainWindow::onCancelDelete()
{
    exitDeleteMode();
    refresh(); // 顺便把勾选状态清掉
}

void MainWindow::onItemChanged(QTableWidgetItem *item)
{
    if (m_updating || !m_deleteMode || item->column() != 0)
        return;

    // 统计勾选数量，更新确认按钮
    int checked = 0;
    for (int r = 0; r < m_table->rowCount(); ++r) {
        QTableWidgetItem *checkItem = m_table->item(r, 0);
        if (checkItem && checkItem->checkState() == Qt::Checked)
            ++checked;
    }
    m_deleteButton->setText(tr("确认删除(%1)").arg(checked));
    m_deleteButton->setEnabled(checked > 0);
}

void MainWindow::refresh()
{
    // —— 先按搜索条件筛选（在内存中过滤，个人账单量级足够快）——
    const QString kw = m_searchEdit->text().trimmed();
    const QString filterCat = m_categoryFilter->currentData().toString();

    QList<Expense> list;
    for (const Expense &e : m_db->allExpenses()) {
        // 分类筛选
        if (!filterCat.isEmpty() && e.category != filterCat)
            continue;
        // 关键词搜索：金额 / 日期 / 分类 / 备注
        if (!kw.isEmpty()) {
            const QString amountText = QString::number(e.amountCents / 100.0, 'f', 2);
            const bool match = e.date.contains(kw, Qt::CaseInsensitive)
                || e.category.contains(kw, Qt::CaseInsensitive)
                || e.subcategory.contains(kw, Qt::CaseInsensitive)
                || e.note.contains(kw, Qt::CaseInsensitive)
                || amountText.contains(kw, Qt::CaseInsensitive)
                || formatAmount(e.amountCents).contains(kw, Qt::CaseInsensitive);
            if (!match)
                continue;
        }
        list.append(e);
    }
    m_currentList = list;

    // —— 填充列表 ——
    m_updating = true;
    m_table->setRowCount(list.size());
    for (int i = 0; i < list.size(); ++i) {
        const Expense &e = list[i];

        // 勾选列（删除模式用）
        auto *checkItem = new QTableWidgetItem();
        checkItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        checkItem->setCheckState(Qt::Unchecked);

        // 日期列（按设置里的格式显示；UserRole 存编号，删除/修改用）
        const QString dateText = QDate::fromString(e.date, QStringLiteral("yyyy-MM-dd"))
                                     .toString(dateFormat());
        auto *dateItem = new QTableWidgetItem(dateText);
        dateItem->setData(Qt::UserRole, e.id);

        // 分类列带图标：如 "🍜 餐饮饮食 / 午餐"
        const QString catText = Categories::emojiForTop(e.category)
            + QStringLiteral(" ") + e.category
            + QStringLiteral(" / ") + e.subcategory;

        auto *catItem = new QTableWidgetItem(catText);
        auto *noteItem = new QTableWidgetItem(e.note);
        auto *amountItem = new QTableWidgetItem(formatAmount(e.amountCents));

        // 金额加粗并显示为主题色
        amountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        amountItem->setForeground(Theme::amountText());
        QFont amountFont = amountItem->font();
        amountFont.setBold(true);
        amountItem->setFont(amountFont);

        m_table->setItem(i, 0, checkItem);
        m_table->setItem(i, 1, dateItem);
        m_table->setItem(i, 2, catItem);
        m_table->setItem(i, 3, noteItem);
        m_table->setItem(i, 4, amountItem);
    }
    m_updating = false;

    // 空状态提示
    if (m_emptyLabel) {
        m_emptyLabel->setGeometry(m_table->rect());
        m_emptyLabel->setVisible(list.isEmpty());
    }

    // —— 底部汇总（有筛选时显示筛选结果）——
    const bool filtering = !kw.isEmpty() || !filterCat.isEmpty();
    if (filtering) {
        qint64 sum = 0;
        for (const Expense &e : list)
            sum += e.amountCents;
        m_totalLabel->setText(tr("筛选出 %1 笔，合计 %2").arg(list.size()).arg(formatAmount(sum)));
    } else {
        m_totalLabel->setText(tr("共 %1 笔，总支出：%2")
                                  .arg(list.size())
                                  .arg(formatAmount(m_db->totalCents())));
    }
}
