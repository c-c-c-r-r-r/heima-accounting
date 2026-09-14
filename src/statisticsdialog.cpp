// ==========================================
// 黑马记账 - 支出统计页面
// ==========================================
#include "statisticsdialog.h"
#include "categories.h"

#include <QColor>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
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

StatisticsDialog::StatisticsDialog(Database *db, QWidget *parent)
    : QDialog(parent)
    , m_db(db)
{
    setWindowTitle(QStringLiteral("📊 支出统计"));
    setMinimumWidth(540);
    setStyleSheet(QStringLiteral(
        "QDialog { background: #fffdf9; }"
        "QLabel { font-size: 12pt; }"
        "QComboBox { font-size: 13pt; padding: 9px 12px; min-height: 28px; }"
        "QComboBox QAbstractItemView { font-size: 13pt; }"
        "QDateEdit { font-size: 13pt; padding: 9px 12px; min-height: 28px; }"));

    // 时间范围预设
    m_preset = new QComboBox(this);
    m_preset->addItems({QStringLiteral("今天"),
                        QStringLiteral("本周"),
                        QStringLiteral("本月"),
                        QStringLiteral("今年"),
                        QStringLiteral("自定义")});

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
    rangeLayout->addWidget(new QLabel(QStringLiteral("从"), m_customRange));
    rangeLayout->addWidget(m_fromDate);
    rangeLayout->addWidget(new QLabel(QStringLiteral("到"), m_customRange));
    rangeLayout->addWidget(m_toDate);
    rangeLayout->addStretch();
    m_customRange->setVisible(false);

    // 总支出（大号橙色）
    m_totalLabel = new QLabel(this);
    m_totalLabel->setAlignment(Qt::AlignCenter);
    m_totalLabel->setStyleSheet(QStringLiteral(
        "color: #e56a10; font-size: 26pt; font-weight: bold; padding: 12px;"));

    // 笔数
    m_countLabel = new QLabel(this);
    m_countLabel->setAlignment(Qt::AlignCenter);
    m_countLabel->setStyleSheet(QStringLiteral("color: #8a7a66; font-size: 12pt;"));

    // 分类明细表
    m_categoryTable = new QTableWidget(this);
    m_categoryTable->setColumnCount(3);
    m_categoryTable->setHorizontalHeaderLabels(
        {QStringLiteral("分类"), QStringLiteral("金额"), QStringLiteral("占比")});
    m_categoryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_categoryTable->setSelectionMode(QAbstractItemView::NoSelection);
    m_categoryTable->verticalHeader()->setVisible(false);
    m_categoryTable->setAlternatingRowColors(true);
    m_categoryTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_categoryTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_categoryTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_categoryTable->setStyleSheet(QStringLiteral(
        "QTableWidget { font-size: 11pt; background: #ffffff;"
        "                alternate-background-color: #fdf3e6; border: none; }"
        "QHeaderView::section { background: #fff3e4; font-weight: bold;"
        "                       font-size: 11pt; padding: 8px; border: none;"
        "                       border-bottom: 2px solid #ffd9b0; }"));

    // 布局：预设 → 自定义范围 → 总支出 → 笔数 → 分类明细
    auto *presetRow = new QHBoxLayout;
    presetRow->addWidget(new QLabel(QStringLiteral("时间范围"), this));
    presetRow->addWidget(m_preset);
    presetRow->addStretch();

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 24, 28, 20);
    layout->addLayout(presetRow);
    layout->addWidget(m_customRange);
    layout->addSpacing(8);
    layout->addWidget(m_totalLabel);
    layout->addWidget(m_countLabel);
    layout->addSpacing(12);
    layout->addWidget(new QLabel(QStringLiteral("分类明细"), this));
    layout->addWidget(m_categoryTable);
    resize(560, 560);

    // 信号连接
    connect(m_preset, &QComboBox::currentIndexChanged,
            this, &StatisticsDialog::onPresetChanged);
    connect(m_fromDate, &QDateEdit::dateChanged, this, &StatisticsDialog::recalc);
    connect(m_toDate, &QDateEdit::dateChanged, this, &StatisticsDialog::recalc);

    // 首次计算
    onPresetChanged();
}

void StatisticsDialog::onPresetChanged()
{
    const bool custom = (m_preset->currentText() == QStringLiteral("自定义"));
    m_customRange->setVisible(custom);
    recalc();
}

QString StatisticsDialog::rangeFrom() const
{
    const QDate today = QDate::currentDate();
    const QString preset = m_preset->currentText();

    if (preset == QStringLiteral("今天"))
        return today.toString(QStringLiteral("yyyy-MM-dd"));
    if (preset == QStringLiteral("本周"))
        return today.addDays(1 - today.dayOfWeek()).toString(QStringLiteral("yyyy-MM-dd")); // 周一
    if (preset == QStringLiteral("本月"))
        return QDate(today.year(), today.month(), 1).toString(QStringLiteral("yyyy-MM-dd"));
    if (preset == QStringLiteral("今年"))
        return QDate(today.year(), 1, 1).toString(QStringLiteral("yyyy-MM-dd"));
    // 自定义
    return m_fromDate->date().toString(QStringLiteral("yyyy-MM-dd"));
}

QString StatisticsDialog::rangeTo() const
{
    const QString preset = m_preset->currentText();
    if (preset == QStringLiteral("自定义"))
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

    const qint64 total = m_db->totalCentsBetween(from, to);
    const int count = m_db->countBetween(from, to);

    m_totalLabel->setText(formatAmount(total));
    m_countLabel->setText(QStringLiteral("共 %1 笔").arg(count));

    // 分类明细
    const QList<CategoryTotal> cats = m_db->categoryTotalsBetween(from, to);
    m_categoryTable->setRowCount(cats.size());
    for (int i = 0; i < cats.size(); ++i) {
        const CategoryTotal &t = cats[i];

        auto *catItem = new QTableWidgetItem(
            Categories::emojiForTop(t.category) + QStringLiteral(" ") + t.category);
        auto *amountItem = new QTableWidgetItem(formatAmount(t.cents));
        amountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        amountItem->setForeground(QColor(QStringLiteral("#e56a10")));
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
