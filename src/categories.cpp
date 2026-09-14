// ==========================================
// 黑马记账 - 收支两级分类数据
// ==========================================
#include "categories.h"

namespace {

// 支出分类表：{名称, 图标, {二级小类...}}
const QVector<TopCategory> kExpenseTable = {
    {QStringLiteral("餐饮饮食"), QStringLiteral("🍜"),
     {QStringLiteral("早餐"), QStringLiteral("午餐"), QStringLiteral("晚餐"),
      QStringLiteral("夜宵"), QStringLiteral("外卖"), QStringLiteral("零食饮料"),
      QStringLiteral("水果"), QStringLiteral("聚餐请客")}},
    {QStringLiteral("交通出行"), QStringLiteral("🚌"),
     {QStringLiteral("公交地铁"), QStringLiteral("打车网约车"), QStringLiteral("加油充电"),
      QStringLiteral("停车费"), QStringLiteral("火车机票"), QStringLiteral("汽车保养维修")}},
    {QStringLiteral("购物消费"), QStringLiteral("🛍️"),
     {QStringLiteral("日用品"), QStringLiteral("服饰鞋包"), QStringLiteral("电子产品"),
      QStringLiteral("美妆护肤"), QStringLiteral("家居厨具")}},
    {QStringLiteral("居住日常"), QStringLiteral("🏠"),
     {QStringLiteral("房租房贷"), QStringLiteral("水费"), QStringLiteral("电费"),
      QStringLiteral("燃气费"), QStringLiteral("物业费"), QStringLiteral("维修维护")}},
    {QStringLiteral("娱乐休闲"), QStringLiteral("🎮"),
     {QStringLiteral("电影演出"), QStringLiteral("游戏"), QStringLiteral("旅行出游"),
      QStringLiteral("运动健身"), QStringLiteral("宠物")}},
    {QStringLiteral("医疗健康"), QStringLiteral("💊"),
     {QStringLiteral("看病买药"), QStringLiteral("体检保健"), QStringLiteral("住院医疗")}},
    {QStringLiteral("学习教育"), QStringLiteral("📚"),
     {QStringLiteral("书籍文具"), QStringLiteral("课程培训"), QStringLiteral("学费")}},
    {QStringLiteral("人情往来"), QStringLiteral("🧧"),
     {QStringLiteral("红包礼金"), QStringLiteral("送礼"), QStringLiteral("孝敬长辈")}},
    {QStringLiteral("通讯网络"), QStringLiteral("📱"),
     {QStringLiteral("手机话费"), QStringLiteral("宽带网费"), QStringLiteral("软件会员")}},
    {QStringLiteral("其他杂项"), QStringLiteral("📦"),
     {QStringLiteral("其他")}},
};

// 收入分类表
const QVector<TopCategory> kIncomeTable = {
    {QStringLiteral("工资薪水"), QStringLiteral("💼"),
     {QStringLiteral("基本工资"), QStringLiteral("奖金提成"), QStringLiteral("补贴报销")}},
    {QStringLiteral("理财收益"), QStringLiteral("📈"),
     {QStringLiteral("基金股票"), QStringLiteral("利息"), QStringLiteral("房租收入")}},
    {QStringLiteral("红包转账"), QStringLiteral("🧧"),
     {QStringLiteral("收红包"), QStringLiteral("转账收入"), QStringLiteral("礼金")}},
    {QStringLiteral("兼职副业"), QStringLiteral("🛠️"),
     {QStringLiteral("兼职"), QStringLiteral("自由职业"), QStringLiteral("二手转卖")}},
    {QStringLiteral("其他收入"), QStringLiteral("🎁"),
     {QStringLiteral("其他")}},
};

const QVector<TopCategory> &tableFor(int type)
{
    return type == 2 ? kIncomeTable : kExpenseTable;
}

} // namespace

QVector<TopCategory> Categories::topCategories(int type)
{
    return tableFor(type);
}

QStringList Categories::topLevels(int type)
{
    QStringList result;
    for (const auto &entry : tableFor(type))
        result << entry.name;
    return result;
}

QStringList Categories::subcategories(const QString &top, int type)
{
    for (const auto &entry : tableFor(type)) {
        if (entry.name == top)
            return entry.subs;
    }
    return {};
}

QString Categories::emojiForTop(const QString &top, int type)
{
    for (const auto &entry : tableFor(type)) {
        if (entry.name == top)
            return entry.emoji;
    }
    return {};
}
