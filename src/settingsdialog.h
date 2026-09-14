// ==========================================
// 黑马记账 - 设置页面
// ==========================================
#pragma once

#include <QDialog>

class QComboBox;

#include "database.h"

// 设置弹窗：主题色 / 日期格式 / 语言 / 回收站入口
class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(Database *db, QWidget *parent = nullptr);

private:
    void load(); // 读取当前设置填入界面
    void save(); // 保存设置

    Database *m_db;
    QComboBox *m_theme;      // 主题色
    QComboBox *m_dateFormat; // 日期格式
    QComboBox *m_language;   // 语言
};
