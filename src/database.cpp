// ==========================================
// 黑马记账 - 数据库访问（SQLite）
// ==========================================
#include "database.h"

#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

bool Database::open(const QString &filePath)
{
    // 先确保数据库所在文件夹存在
    QDir().mkpath(QFileInfo(filePath).absolutePath());

    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    m_db.setDatabaseName(filePath);
    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        return false;
    }

    // 建表（表不存在时才创建）
    QSqlQuery query(m_db);
    const bool ok = query.exec(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS expenses ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT," // 编号
        " amount_cents INTEGER NOT NULL,"       // 金额，单位：分
        " category TEXT NOT NULL,"              // 一级大类
        " subcategory TEXT NOT NULL,"           // 二级小类
        " date TEXT NOT NULL,"                  // 记账日期 yyyy-MM-dd
        " note TEXT NOT NULL DEFAULT ''"        // 备注
        ")"));
    if (!ok) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

bool Database::addExpense(const Expense &e)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "INSERT INTO expenses (amount_cents, category, subcategory, date, note) "
        "VALUES (?, ?, ?, ?, ?)"));
    query.addBindValue(e.amountCents);
    query.addBindValue(e.category);
    query.addBindValue(e.subcategory);
    query.addBindValue(e.date);
    query.addBindValue(e.note);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

bool Database::deleteExpense(qint64 id)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("DELETE FROM expenses WHERE id = ?"));
    query.addBindValue(id);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

QList<Expense> Database::allExpenses()
{
    QList<Expense> result;
    QSqlQuery query(m_db);
    // 按日期倒序（新的在前），同一天按编号倒序
    if (!query.exec(QStringLiteral(
            "SELECT id, amount_cents, category, subcategory, date, note "
            "FROM expenses ORDER BY date DESC, id DESC"))) {
        m_lastError = query.lastError().text();
        return result;
    }
    while (query.next()) {
        Expense e;
        e.id = query.value(0).toLongLong();
        e.amountCents = query.value(1).toLongLong();
        e.category = query.value(2).toString();
        e.subcategory = query.value(3).toString();
        e.date = query.value(4).toString();
        e.note = query.value(5).toString();
        result.append(e);
    }
    return result;
}

qint64 Database::totalCents()
{
    QSqlQuery query(m_db);
    if (query.exec(QStringLiteral("SELECT COALESCE(SUM(amount_cents), 0) FROM expenses"))
        && query.next()) {
        return query.value(0).toLongLong();
    }
    return 0;
}

qint64 Database::totalCentsBetween(const QString &from, const QString &to)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "SELECT COALESCE(SUM(amount_cents), 0) FROM expenses WHERE date BETWEEN ? AND ?"));
    query.addBindValue(from);
    query.addBindValue(to);
    if (query.exec() && query.next())
        return query.value(0).toLongLong();
    m_lastError = query.lastError().text();
    return 0;
}

int Database::countBetween(const QString &from, const QString &to)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "SELECT COUNT(*) FROM expenses WHERE date BETWEEN ? AND ?"));
    query.addBindValue(from);
    query.addBindValue(to);
    if (query.exec() && query.next())
        return query.value(0).toInt();
    m_lastError = query.lastError().text();
    return 0;
}

QList<CategoryTotal> Database::categoryTotalsBetween(const QString &from, const QString &to)
{
    QList<CategoryTotal> result;
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "SELECT category, SUM(amount_cents) AS s FROM expenses "
        "WHERE date BETWEEN ? AND ? GROUP BY category ORDER BY s DESC"));
    query.addBindValue(from);
    query.addBindValue(to);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return result;
    }
    while (query.next()) {
        CategoryTotal t;
        t.category = query.value(0).toString();
        t.cents = query.value(1).toLongLong();
        result.append(t);
    }
    return result;
}
