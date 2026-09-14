// ==========================================
// 黑马记账 - 收支两级分类数据
// ==========================================
#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

// 一级大类信息：名称 + 图标表情 + 二级小类列表
struct TopCategory
{
    QString name;
    QString emoji;
    QStringList subs;
};

// 分类体系：一级大类 → 二级小类
// type: 1 = 支出（默认），2 = 收入
class Categories
{
public:
    // 完整分类表
    static QVector<TopCategory> topCategories(int type = 1);

    // 所有一级大类名称（不含图标）
    static QStringList topLevels(int type = 1);

    // 某个一级大类下的二级小类
    static QStringList subcategories(const QString &top, int type = 1);

    // 某个一级大类对应的图标表情（找不到时返回空串）
    static QString emojiForTop(const QString &top, int type = 1);
};
