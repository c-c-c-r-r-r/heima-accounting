// ==========================================
// 黑马记账 - 回收站（恢复误删的账单）
// ==========================================
#pragma once

#include <QDialog>

class QLabel;
class QTableWidget;

#include "database.h"

// 回收站弹窗：列出已删除账单，可恢复 / 彻底删除 / 清空
class RecycleBinDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RecycleBinDialog(Database *db, QWidget *parent = nullptr);

private slots:
    void restoreSelected(); // 恢复所选账单
    void purgeSelected();   // 彻底删除所选（不可恢复）
    void purgeAll();        // 清空回收站
    void refresh();         // 重新加载回收站列表

private:
    Database *m_db;
    QTableWidget *m_table;
    QLabel *m_emptyLabel;
    QList<Expense> m_list; // 当前显示的回收站账单
};
