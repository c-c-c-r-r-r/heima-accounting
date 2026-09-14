# ==========================================
# 黑马记账 - 项目构建配置
# ==========================================
cmake_minimum_required(VERSION 3.21)
project(heima_accounting VERSION 0.1.0 LANGUAGES CXX)

# 使用 C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 找到 Qt6 的界面库（Widgets）、数据库库（Sql）、图表库（Charts）
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Charts)
qt_standard_project_setup()

# 生成可执行程序（WIN32 表示 Windows 图形程序，运行时不会弹出黑色命令行窗口）
qt_add_executable(heima_accounting WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/addexpensedialog.cpp
    src/addexpensedialog.h
    src/categories.cpp
    src/categories.h
    src/database.cpp
    src/database.h
    src/statisticsdialog.cpp
    src/statisticsdialog.h
    src/settingsdialog.cpp
    src/settingsdialog.h
    src/recyclebindialog.cpp
    src/recyclebindialog.h
    src/theme.cpp
    src/theme.h
)

# 链接 Qt6 的界面库、数据库库和图表库
target_link_libraries(heima_accounting PRIVATE Qt6::Widgets Qt6::Sql Qt6::Charts)

# 翻译文件说明：translations/heima_en_US.ts 用 Qt 自带 lrelease 工具手动编译为
# translations/heima_en_US.qm，程序运行时从自身 translations 文件夹加载（见 main.cpp）。

# 让 MSVC 编译器把源码里的中文按 UTF-8 处理（否则中文注释和文字会乱码）
if(MSVC)
    target_compile_options(heima_accounting PRIVATE /utf-8)
endif()
