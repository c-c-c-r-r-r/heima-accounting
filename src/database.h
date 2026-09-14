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

// 负责账单数据的保存和查询
class Database
{
public:
    // 打开（或创建）数据库文件，成功返回 true
    bool open(const QString &filePath);

    // 最近一次出错的原因
    QString lastError() const { return m_lastError; }

    bool addExpense(const Expense &e); // 新增一笔账
    QList<Expense> allExpenses();      // 全部账单（按日期倒序，新的在前）
    qint64 totalCents();               // 总支出（单位：分）

private:
    QSqlDatabase m_db;
    QString m_lastError;
};
