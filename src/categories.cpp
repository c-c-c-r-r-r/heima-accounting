// ==========================================
// 黑马记账 - 花销两级分类数据
// ==========================================
#include "categories.h"

#include <QPair>
#include <QVector>

namespace {

// 分类表：{一级大类, {二级小类...}}
const QVector<QPair<QString, QStringList>> kCategoryTable = {
    {QStringLiteral("餐饮饮食"),
     {QStringLiteral("早餐"), QStringLiteral("午餐"), QStringLiteral("晚餐"),
      QStringLiteral("夜宵"), QStringLiteral("外卖"), QStringLiteral("零食饮料"),
      QStringLiteral("水果"), QStringLiteral("聚餐请客")}},
    {QStringLiteral("交通出行"),
     {QStringLiteral("公交地铁"), QStringLiteral("打车网约车"), QStringLiteral("加油充电"),
      QStringLiteral("停车费"), QStringLiteral("火车机票"), QStringLiteral("汽车保养维修")}},
    {QStringLiteral("购物消费"),
     {QStringLiteral("日用品"), QStringLiteral("服饰鞋包"), QStringLiteral("电子产品"),
      QStringLiteral("美妆护肤"), QStringLiteral("家居厨具")}},
    {QStringLiteral("居住日常"),
     {QStringLiteral("房租房贷"), QStringLiteral("水费"), QStringLiteral("电费"),
      QStringLiteral("燃气费"), QStringLiteral("物业费"), QStringLiteral("维修维护")}},
    {QStringLiteral("娱乐休闲"),
     {QStringLiteral("电影演出"), QStringLiteral("游戏"), QStringLiteral("旅行出游"),
      QStringLiteral("运动健身"), QStringLiteral("宠物")}},
    {QStringLiteral("医疗健康"),
     {QStringLiteral("看病买药"), QStringLiteral("体检保健"), QStringLiteral("住院医疗")}},
    {QStringLiteral("学习教育"),
     {QStringLiteral("书籍文具"), QStringLiteral("课程培训"), QStringLiteral("学费")}},
    {QStringLiteral("人情往来"),
     {QStringLiteral("红包礼金"), QStringLiteral("送礼"), QStringLiteral("孝敬长辈")}},
    {QStringLiteral("通讯网络"),
     {QStringLiteral("手机话费"), QStringLiteral("宽带网费"), QStringLiteral("软件会员")}},
    {QStringLiteral("其他杂项"),
     {QStringLiteral("其他")}},
};

} // namespace

QStringList Categories::topLevels()
{
    QStringList result;
    result.reserve(kCategoryTable.size());
    for (const auto &entry : kCategoryTable)
        result << entry.first;
    return result;
}

QStringList Categories::subcategories(const QString &top)
{
    for (const auto &entry : kCategoryTable) {
        if (entry.first == top)
            return entry.second;
    }
    return {};
}
