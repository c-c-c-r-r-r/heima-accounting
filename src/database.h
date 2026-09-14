// ==========================================
// 黑马记账 - 数据库访问（SQLite）
// ==========================================
#pragma once

#include <QList>
#include <QSqlDatabase>
#include <QString>

// 一条收支记录的数据（支出或收入，用 type 区分）
struct Expense
{
    qint64 id = 0;
    qint64 amountCents = 0; // 金额，单位：分（整数存储，避免小数误差）
    int type = 1;           // 1 = 支出，2 = 收入
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
// 说明：删除为「软删除」——只做标记，数据进回收站，可从设置页恢复
class Database
{
public:
    // 打开（或创建）数据库文件，成功返回 true
    bool open(const QString &filePath);

    // 最近一次出错的原因
    QString lastError() const { return m_lastError; }

    bool addExpense(const Expense &e);              // 新增一条记录
    bool updateExpense(qint64 id, const Expense &e); // 修改一条记录
    bool deleteExpense(qint64 id);                  // 删除（进回收站）
    bool restoreExpense(qint64 id);                 // 从回收站恢复
    bool purgeExpense(qint64 id);                   // 彻底删除（不可恢复）
    bool purgeAllDeleted();                         // 清空回收站

    QList<Expense> allExpenses();     // 全部记录（按日期倒序，新的在前）
    QList<Expense> deletedExpenses(); // 回收站里的记录

    // —— 汇总统计（type: 0 = 收支合计，1 = 支出，2 = 收入；date 均为 yyyy-MM-dd，含首尾两天）——
    qint64 totalCents(int type = 0);                                        // 总金额（分）
    qint64 totalCentsBetween(const QString &from, const QString &to, int type = 0); // 时间段总金额
    int countBetween(const QString &from, const QString &to, int type = 0); // 时间段笔数
    QList<CategoryTotal> categoryTotalsBetween(const QString &from,
                                               const QString &to,
                                               int type = 1); // 时间段各一级大类汇总（金额降序）

private:
    QSqlDatabase m_db;
    QString m_lastError;
};
