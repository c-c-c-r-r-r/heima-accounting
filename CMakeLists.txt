# ==========================================
# 黑马记账 - 项目构建配置
# ==========================================
cmake_minimum_required(VERSION 3.21)
project(heima_accounting VERSION 0.1.0 LANGUAGES CXX)

# 使用 C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 找到 Qt6 的界面库（Widgets）和数据库库（Sql）
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)
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
)

# 链接 Qt6 的界面库和数据库库
target_link_libraries(heima_accounting PRIVATE Qt6::Widgets Qt6::Sql)

# 让 MSVC 编译器把源码里的中文按 UTF-8 处理（否则中文注释和文字会乱码）
if(MSVC)
    target_compile_options(heima_accounting PRIVATE /utf-8)
endif()
