// ==========================================
// 黑马记账 - 设置页面
// ==========================================
#include "settingsdialog.h"
#include "recyclebindialog.h"
#include "theme.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(Database *db, QWidget *parent)
    : QDialog(parent)
    , m_db(db)
{
    setWindowTitle(tr("⚙️ 设置"));
    setMinimumWidth(440);
    setStyleSheet(QStringLiteral(
        "QDialog { background: #fffdf9; }"
        "QLabel { font-size: 12pt; }"
        "QComboBox { font-size: 13pt; padding: 9px 12px; min-height: 28px; }"
        "QComboBox QAbstractItemView { font-size: 13pt; }"));

    // 主题色
    m_theme = new QComboBox(this);
    for (const QString &name : Theme::names())
        m_theme->addItem(name, name);

    // 日期格式（显示示例 → 实际格式值）
    m_dateFormat = new QComboBox(this);
    m_dateFormat->addItem(QStringLiteral("2024-09-14"), QStringLiteral("yyyy-MM-dd"));
    m_dateFormat->addItem(QStringLiteral("2024/09/14"), QStringLiteral("yyyy/MM/dd"));
    m_dateFormat->addItem(QStringLiteral("2024年9月14日"), QStringLiteral("yyyy年M月d日"));

    // 语言
    m_language = new QComboBox(this);
    m_language->addItem(QStringLiteral("简体中文"), QStringLiteral("zh_CN"));
    m_language->addItem(QStringLiteral("English"), QStringLiteral("en_US"));

    // 表单
    auto *form = new QFormLayout;
    form->setVerticalSpacing(16);
    form->addRow(tr("主题色"), m_theme);
    form->addRow(tr("日期格式"), m_dateFormat);
    form->addRow(tr("语言"), m_language);

    // 回收站入口（显示当前条数）
    auto *recycleButton = new QPushButton(this);
    const auto updateRecycleText = [this, recycleButton]() {
        recycleButton->setText(tr("🗑️ 回收站（%1 条）").arg(m_db->deletedExpenses().size()));
    };
    updateRecycleText();
    recycleButton->setCursor(Qt::PointingHandCursor);
    recycleButton->setStyleSheet(QStringLiteral(
        "QPushButton { background: #ffffff; color: #55504a; font-size: 13pt;"
        "               padding: 10px 24px; border: 1px solid %1; border-radius: 8px;"
        "               text-align: left; }"
        "QPushButton:hover { background: %2; }")
        .arg(Theme::border().name(), Theme::lightBg().name()));
    connect(recycleButton, &QPushButton::clicked, this, [this, updateRecycleText]() {
        RecycleBinDialog dialog(m_db, this);
        dialog.exec();
        updateRecycleText(); // 回收站数量可能变化
    });

    // 关闭按钮（关闭时保存设置）
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    buttons->button(QDialogButtonBox::Close)->setText(tr("关闭"));
    connect(buttons, &QDialogButtonBox::rejected, this, [this]() {
        save();
        reject();
    });

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 24, 28, 20);
    layout->addLayout(form);
    layout->addSpacing(12);
    layout->addWidget(recycleButton);
    layout->addSpacing(8);
    layout->addWidget(buttons);

    load();
}

void SettingsDialog::load()
{
    // 主题色
    const int themeIndex = m_theme->findData(Theme::currentName());
    m_theme->setCurrentIndex(themeIndex >= 0 ? themeIndex : 0);

    // 日期格式
    const QString fmt = QSettings().value(QStringLiteral("ui/dateFormat"),
                                          QStringLiteral("yyyy-MM-dd")).toString();
    const int fmtIndex = m_dateFormat->findData(fmt);
    m_dateFormat->setCurrentIndex(fmtIndex >= 0 ? fmtIndex : 0);

    // 语言
    const QString lang = QSettings().value(QStringLiteral("ui/language"),
                                           QStringLiteral("zh_CN")).toString();
    const int langIndex = m_language->findData(lang);
    m_language->setCurrentIndex(langIndex >= 0 ? langIndex : 0);
}

void SettingsDialog::save()
{
    QSettings settings;

    // 语言变化 → 提示重启生效
    const QString oldLang = settings.value(QStringLiteral("ui/language"),
                                           QStringLiteral("zh_CN")).toString();
    const QString newLang = m_language->currentData().toString();
    settings.setValue(QStringLiteral("ui/language"), newLang);

    settings.setValue(QStringLiteral("ui/theme"), m_theme->currentData().toString());
    settings.setValue(QStringLiteral("ui/dateFormat"), m_dateFormat->currentData().toString());

    if (oldLang != newLang) {
        QMessageBox::information(this, tr("提示"),
                                 tr("语言修改将在下次启动后生效。"));
    }
}
