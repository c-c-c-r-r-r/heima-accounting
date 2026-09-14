// ==========================================
// 黑马记账 - 花销两级分类数据
// ==========================================
#pragma once

#include <QString>
#include <QStringList>

// 分类体系：一级大类 → 二级小类（与 CLAUDE.md 产品文档一致）
class Categories
{
public:
    // 获取所有一级大类
    static QStringList topLevels();

    // 获取某个一级大类下的二级小类
    static QStringList subcategories(const QString &top);
};
