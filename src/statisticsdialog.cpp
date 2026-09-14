// ==========================================
// 黑马记账 - 收支统计页面
// ==========================================
#include "statisticsdialog.h"
#include "categories.h"
#include "theme.h"

#include <QColor>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMargins>
#include <QPainter>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>

namespace {

// 收入固定用绿色（与支出颜色区分）
const QColor kIncomeColor = QColor(QStringLiteral("#16a34a"));
const QColor kNegativeColor = QColor(QStringLiteral("#c0392b"));

// 金额格式化：分 → "¥ 12.34"
QString formatAmount(qint64 cents)
{
    return QStringLiteral("¥ %1.%2")
        .arg(cents / 100)
        .arg(cents % 100, 2, 10, QLatin1Char('0'));
}

// 带正负号的金额
QString formatAmountSigned(qint64 cents)
{
    if (cents < 0)
        return QStringLiteral("-") + formatAmount(-cents);
    return formatAmount(cents);
}

// 饼图配色（固定一组柔和的颜色，循环使用）
const QList<QColor> kSliceColors = {
    QColor(QStringLiteral("#ff9f43")), QColor(QStringLiteral("#54a0ff")),
    QColor(QStringLiteral("#5f27cd")), QColor(QStringLiteral("#1dd1a1")),
    QColor(QStringLiteral("#ff6b6b")), QColor(QStringLiteral("#feca57")),
    QColor(QStringLiteral("#48dbfb")), QColor(QStringLiteral("#ff9ff3")),
    QColor(QStringLiteral("#10ac84")), QColor(QStringLiteral("#a29bfe")),
};

} // namespace

StatisticsDialog::StatisticsDialog(Database *db, QWidget *parent)
    : QDialog(parent)
    , m_db(db)
{
    setWindowTitle(tr("📊 收支统计"));
    setMinimumWidth(560);
    setStyleSheet(QStringLiteral(
        "QDialog { background: #fffdf9; }"
        "QLabel { font-size: 12pt; }"
        "QComboBox { font-size: 13pt; padding: 9px 12px; min-height: 28px; }"
        "QComboBox QAbstractItemView { font-size: 13pt; }"
        "QDateEdit { font-size: 13pt; padding: 9px 12px; min-height: 28px; }"));

    // 时间范围预设
    m_preset = new QComboBox(this);
    m_preset->addItems({tr("今天"), tr("本周"), tr("本月"), tr("今年"), tr("自定义")});

    // 统计类型：支出 / 收入（控制饼图和明细表）
    m_typeFilter = new QComboBox(this);
    m_typeFilter->addItem(tr("支出"), 1);
    m_typeFilter->addItem(tr("收入"), 2);

    // 自定义日期范围（默认隐藏，选「自定义」才显示）
    m_fromDate = new QDateEdit(QDate::currentDate(), this);
    m_toDate = new QDateEdit(QDate::currentDate(), this);
    for (QDateEdit *edit : {m_fromDate, m_toDate}) {
        edit->setCalendarPopup(true);
        edit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    }
    m_customRange = new QWidget(this);
    auto *rangeLayout = new QHBoxLayout(m_customRange);
    rangeLayout->setContentsMargins(0, 0, 0, 0);
    rangeLayout->addWidget(new QLabel(tr("从"), m_customRange));
    rangeLayout->addWidget(m_fromDate);
    rangeLayout->addWidget(new QLabel(tr("到"), m_customRange));
    rangeLayout->addWidget(m_toDate);
    rangeLayout->addStretch();
    m_customRange->setVisible(false);

    // 汇总区：总收入（绿）/ 总支出（主题色）/ 结余（正绿负红）
    const QString bigLabelStyle = QStringLiteral(
        "font-size: 19pt; font-weight: bold; padding: 8px 4px;");
    m_incomeLabel = new QLabel(this);
    m_incomeLabel->setAlignment(Qt::AlignCenter);
    m_incomeLabel->setStyleSheet(
        QStringLiteral("color: %1; %2").arg(kIncomeColor.name(), bigLabelStyle));
    m_expenseLabel = new QLabel(this);
    m_expenseLabel->setAlignment(Qt::AlignCenter);
    m_expenseLabel->setStyleSheet(
        QStringLiteral("color: %1; %2").arg(Theme::amountText().name(), bigLabelStyle));
    m_balanceLabel = new QLabel(this);
    m_balanceLabel->setAlignment(Qt::AlignCenter);
    m_balanceLabel->setStyleSheet(bigLabelStyle); // 颜色按正负动态设置

    auto *summaryRow = new QHBoxLayout;
    summaryRow->addWidget(m_incomeLabel, 1);
    summaryRow->addWidget(m_expenseLabel, 1);
    summaryRow->addWidget(m_balanceLabel, 1);

    // 笔数
    m_countLabel = new QLabel(this);
    m_countLabel->setAlignment(Qt::AlignCenter);
    m_countLabel->setStyleSheet(QStringLiteral("color: #8a7a66; font-size: 12pt;"));

    // 分类占比饼图
    m_chartView = new QChartView(new QChart, this);
    m_chartView->chart()->legend()->hide();
    m_chartView->chart()->setBackgroundRoundness(0);
    m_chartView->chart()->setMargins(QMargins(0, 0, 0, 0));
    m_chartView->chart()->setBackgroundBrush(Qt::NoBrush);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setStyleSheet(QStringLiteral("background: transparent;"));
    m_chartView->setMinimumHeight(260);

    // 分类明细表
    m_categoryTable = new QTableWidget(this);
    m_categoryTable->setColumnCount(3);
    m_categoryTable->setHorizontalHeaderLabels(
        {tr("分类"), tr("金额"), tr("占比")});
    m_categoryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_categoryTable->setSelectionMode(QAbstractItemView::NoSelection);
    m_categoryTable->verticalHeader()->setVisible(false);
    m_categoryTable->setAlternatingRowColors(true);
    m_categoryTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_categoryTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_categoryTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_categoryTable->setStyleSheet(QStringLiteral(
        "QTableWidget { font-size: 11pt; background: #ffffff;"
        "                alternate-background-color: %1; border: none; }"
        "QHeaderView::section { background: %1; font-weight: bold;"
        "                       font-size: 11pt; padding: 8px; border: none;"
        "                       border-bottom: 2px solid %2; }")
        .arg(Theme::lightBg().name(), Theme::border().name()));

    // 布局：预设+类型 → 自定义范围 → 汇总 → 笔数 → 饼图 → 分类明细
    auto *presetRow = new QHBoxLayout;
    presetRow->addWidget(new QLabel(tr("时间范围"), this));
    presetRow->addWidget(m_preset);
    presetRow->addSpacing(24);
    presetRow->addWidget(new QLabel(tr("统计类型"), this));
    presetRow->addWidget(m_typeFilter);
    presetRow->addStretch();

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 24, 28, 20);
    layout->addLayout(presetRow);
    layout->addWidget(m_customRange);
    layout->addSpacing(8);
    layout->addLayout(summaryRow);
    layout->addWidget(m_countLabel);
    layout->addWidget(m_chartView);
    layout->addSpacing(8);
    layout->addWidget(new QLabel(tr("分类明细"), this));
    layout->addWidget(m_categoryTable);
    resize(620, 780);

    // 信号连接
    connect(m_preset, &QComboBox::currentIndexChanged,
            this, &StatisticsDialog::onPresetChanged);
    connect(m_typeFilter, &QComboBox::currentIndexChanged, this, &StatisticsDialog::recalc);
    connect(m_fromDate, &QDateEdit::dateChanged, this, &StatisticsDialog::recalc);
    connect(m_toDate, &QDateEdit::dateChanged, this, &StatisticsDialog::recalc);

    // 首次计算
    onPresetChanged();
}

void StatisticsDialog::onPresetChanged()
{
    const bool custom = (m_preset->currentText() == tr("自定义"));
    m_customRange->setVisible(custom);
    recalc();
}

QString StatisticsDialog::rangeFrom() const
{
    const QDate today = QDate::currentDate();
    const QString preset = m_preset->currentText();

    // 注意：必须与 tr() 后的文字比较，保证中英文界面都正确
    if (preset == tr("今天"))
        return today.toString(QStringLiteral("yyyy-MM-dd"));
    if (preset == tr("本周"))
        return today.addDays(1 - today.dayOfWeek()).toString(QStringLiteral("yyyy-MM-dd")); // 周一
    if (preset == tr("本月"))
        return QDate(today.year(), today.month(), 1).toString(QStringLiteral("yyyy-MM-dd"));
    if (preset == tr("今年"))
        return QDate(today.year(), 1, 1).toString(QStringLiteral("yyyy-MM-dd"));
    // 自定义
    return m_fromDate->date().toString(QStringLiteral("yyyy-MM-dd"));
}

QString StatisticsDialog::rangeTo() const
{
    if (m_preset->currentText() == tr("自定义"))
        return m_toDate->date().toString(QStringLiteral("yyyy-MM-dd"));
    return QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd")); // 预设均统计到今天
}

void StatisticsDialog::recalc()
{
    // 若自定义范围"从"晚于"到"，自动把"到"对齐成"从"
    if (m_fromDate->date() > m_toDate->date())
        m_toDate->setDate(m_fromDate->date());

    const QString from = rangeFrom();
    const QString to = rangeTo();
    const int filterType = m_typeFilter->currentData().toInt();

    // 收支总额与笔数
    const qint64 income = m_db->totalCentsBetween(from, to, 2);
    const qint64 expense = m_db->totalCentsBetween(from, to, 1);
    const qint64 balance = income - expense;
    const int incomeCount = m_db->countBetween(from, to, 2);
    const int expenseCount = m_db->countBetween(from, to, 1);

    m_incomeLabel->setText(tr("总收入 %1").arg(formatAmount(income)));
    m_expenseLabel->setText(tr("总支出 %1").arg(formatAmount(expense)));
    m_balanceLabel->setText(tr("结余 %1").arg(formatAmountSigned(balance)));
    m_balanceLabel->setStyleSheet(QStringLiteral(
        "color: %1; font-size: 19pt; font-weight: bold; padding: 8px 4px;")
        .arg(balance >= 0 ? kIncomeColor.name() : kNegativeColor.name()));
    m_countLabel->setText(tr("收入 %1 笔 · 支出 %2 笔").arg(incomeCount).arg(expenseCount));

    // 当前统计类型的总额（用于饼图和占比计算）
    const qint64 total = (filterType == 2) ? income : expense;
    const QList<CategoryTotal> cats = m_db->categoryTotalsBetween(from, to, filterType);

    // —— 饼图（分类占比）——
    QChart *chart = m_chartView->chart();
    chart->removeAllSeries();
    auto *series = new QPieSeries;
    series->setHoleSize(0.45); // 环形饼图更好看
    for (int i = 0; i < cats.size(); ++i) {
        const CategoryTotal &t = cats[i];
        if (t.cents <= 0)
            continue;
        QPieSlice *slice = series->append(
            Categories::emojiForTop(t.category, filterType)
                + QStringLiteral(" ") + t.category, t.cents);
        const double pct = total > 0 ? t.cents * 100.0 / total : 0.0;
        slice->setLabel(QString::number(pct, 'f', 1) + QStringLiteral("%"));
        slice->setLabelVisible(pct >= 4.0); // 太小的占比不标字，避免重叠
        slice->setLabelColor(QColor(QStringLiteral("#5a4a38")));
        slice->setColor(kSliceColors[i % kSliceColors.size()]);
    }
    chart->addSeries(series);

    // —— 分类明细表 ——
    m_categoryTable->setRowCount(cats.size());
    for (int i = 0; i < cats.size(); ++i) {
        const CategoryTotal &t = cats[i];

        auto *catItem = new QTableWidgetItem(
            Categories::emojiForTop(t.category, filterType)
                + QStringLiteral(" ") + t.category);
        auto *amountItem = new QTableWidgetItem(formatAmount(t.cents));
        amountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        amountItem->setForeground(filterType == 2 ? kIncomeColor : Theme::amountText());
        auto *percentItem = new QTableWidgetItem(
            total > 0
                ? QStringLiteral("%1%").arg(t.cents * 100.0 / total, 0, 'f', 1)
                : QStringLiteral("0.0%"));
        percentItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        m_categoryTable->setItem(i, 0, catItem);
        m_categoryTable->setItem(i, 1, amountItem);
        m_categoryTable->setItem(i, 2, percentItem);
    }
}
