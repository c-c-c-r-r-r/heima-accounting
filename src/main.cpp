// ==========================================
// 黑马记账 - 程序入口
// ==========================================
#include "database.h"
#include "mainwindow.h"

#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 设置应用名称（决定数据文件的存放位置）
    QApplication::setApplicationName(QStringLiteral("黑马记账"));
    QApplication::setOrganizationName(QStringLiteral("黑马"));

    // 语言设置：选 English 时加载英文翻译，其余情况使用源码里的中文
    const QString lang = QSettings().value(QStringLiteral("ui/language"),
                                           QStringLiteral("zh_CN")).toString();
    static QTranslator translator; // static 保证比 app 活得久
    if (lang == QStringLiteral("en_US")) {
        // 翻译文件放在程序同级的 translations 文件夹里
        const QString qmPath = QCoreApplication::applicationDirPath()
            + QStringLiteral("/translations/heima_en_US.qm");
        if (translator.load(qmPath))
            QApplication::installTranslator(&translator);
    }

    // 打开数据库：存放在系统"应用数据"目录，关闭软件后数据不丢失
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString dbPath = dataDir + QStringLiteral("/heima_accounting.db");
    Database db;
    if (!db.open(dbPath)) {
        QMessageBox::critical(nullptr, QStringLiteral("黑马记账"),
                              QObject::tr("数据文件打开失败：\n%1").arg(db.lastError()));
        return 1;
    }

    // 创建并显示主窗口
    MainWindow window(&db);
    window.show();

    return app.exec();
}
