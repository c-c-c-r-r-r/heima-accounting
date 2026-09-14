// ==========================================
// 黑马记账 - 主题色管理（设置页可切换）
// ==========================================
#pragma once

#include <QColor>
#include <QString>
#include <QStringList>

// 全局主题：所有窗口的配色都从这里取，切换主题后立即生效
class Theme
{
public:
    // 可选主题名称列表
    static QStringList names();

    // 当前主题名（默认「活力橙」，保存在系统设置中）
    static QString currentName();
    static void setCurrent(const QString &name); // 切换主题并保存

    // —— 当前主题的颜色 ——
    static QColor primary();        // 主色（大按钮、高亮）
    static QColor primaryHover();   // 主色悬停
    static QColor primaryPressed(); // 主色按下
    static QColor lightBg();        // 浅色背景（工具栏、表头、隔行）
    static QColor border();         // 边框
    static QColor amountText();     // 金额强调色

private:
    static int currentIndex(); // 当前主题在表中的下标
};
