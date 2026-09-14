// ==========================================
// 黑马记账 - 数据库访问（SQLite）
// ==========================================
#pragma once

#include <QList>
#include <QSqlDatabase>
#include <QString>

// 一笔账的数据
struct Expense
{
    qint64 id = 0;
    qint64 amountCents = 0; // 金额，单位：分（整数存储，避免小数误差）
    QString category;       // 一级大类
    QString subcategory;    // 二级小类
    QString date;           // 记账日期 yyyy-MM-dd
    QString note;           // 备注
};

// 某个一级大类的汇总金额
struct CategoryTotal
{
    QString category;       // 一级大类名称
    qint64 cents = 0;       // 汇总金额（分）
};

// 负责账单数据的保存和查询
class Database
{
public:
    // 打开（或创建）数据库文件，成功返回 true
    bool open(const QString &filePath);

    // 最近一次出错的原因
    QString lastError() const { return m_lastError; }

    bool addExpense(const Expense &e);   // 新增一笔账
    bool deleteExpense(qint64 id);       // 按编号删除一笔账
    QList<Expense> allExpenses();        // 全部账单（按日期倒序，新的在前）
    qint64 totalCents();                 // 总支出（单位：分）

    // —— 统计相关（date 均为 yyyy-MM-dd，含首尾两天）——
    qint64 totalCentsBetween(const QString &from, const QString &to);            // 时间段总支出
    int countBetween(const QString &from, const QString &to);                    // 时间段账单笔数
    QList<CategoryTotal> categoryTotalsBetween(const QString &from,
                                               const QString &to); // 时间段各一级大类汇总（金额降序）

private:
    QSqlDatabase m_db;
    QString m_lastError;
};
