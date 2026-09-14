// ==========================================
// 黑马记账 - 主题色管理（设置页可切换）
// ==========================================
#include "theme.h"

#include <QSettings>

namespace {

struct ThemeEntry
{
    QString name;
    QColor primary;
    QColor hover;
    QColor pressed;
    QColor lightBg;
    QColor border;
};

// 主题表：{名称, 主色, 悬停, 按下, 浅背景, 边框}
const QList<ThemeEntry> kThemes = {
    {QStringLiteral("活力橙"), QColor(QStringLiteral("#ff7a1a")), QColor(QStringLiteral("#ff8f3d")),
     QColor(QStringLiteral("#e56a10")), QColor(QStringLiteral("#fff3e4")), QColor(QStringLiteral("#ffe3c2"))},
    {QStringLiteral("清新蓝"), QColor(QStringLiteral("#2f7df6")), QColor(QStringLiteral("#5b9bf8")),
     QColor(QStringLiteral("#2567cf")), QColor(QStringLiteral("#eaf2ff")), QColor(QStringLiteral("#cfe0ff"))},
    {QStringLiteral("自然绿"), QColor(QStringLiteral("#2e9e5b")), QColor(QStringLiteral("#4cb57a")),
     QColor(QStringLiteral("#258049")), QColor(QStringLiteral("#e8f7ee")), QColor(QStringLiteral("#c9ecd7"))},
    {QStringLiteral("优雅紫"), QColor(QStringLiteral("#8b5cf6")), QColor(QStringLiteral("#a37bf8")),
     QColor(QStringLiteral("#7447d6")), QColor(QStringLiteral("#f2ecfe")), QColor(QStringLiteral("#e0d4fd"))},
    {QStringLiteral("樱花粉"), QColor(QStringLiteral("#ec5f8a")), QColor(QStringLiteral("#f07ba0")),
     QColor(QStringLiteral("#d14a74")), QColor(QStringLiteral("#fdeaf1")), QColor(QStringLiteral("#fbd3e0"))},
};

} // namespace

QStringList Theme::names()
{
    QStringList result;
    result.reserve(kThemes.size());
    for (const auto &t : kThemes)
        result << t.name;
    return result;
}

int Theme::currentIndex()
{
    const QString name = currentName();
    for (int i = 0; i < kThemes.size(); ++i) {
        if (kThemes[i].name == name)
            return i;
    }
    return 0; // 默认活力橙
}

QString Theme::currentName()
{
    return QSettings().value(QStringLiteral("ui/theme"), QStringLiteral("活力橙")).toString();
}

void Theme::setCurrent(const QString &name)
{
    QSettings().setValue(QStringLiteral("ui/theme"), name);
}

QColor Theme::primary()        { return kThemes[currentIndex()].primary; }
QColor Theme::primaryHover()   { return kThemes[currentIndex()].hover; }
QColor Theme::primaryPressed() { return kThemes[currentIndex()].pressed; }
QColor Theme::lightBg()        { return kThemes[currentIndex()].lightBg; }
QColor Theme::border()         { return kThemes[currentIndex()].border; }
QColor Theme::amountText()     { return kThemes[currentIndex()].pressed; }
