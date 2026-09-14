@echo off
rem ==========================================
rem 黑马记账 - 一键运行脚本
rem 编译成功后双击本文件即可启动程序
rem ==========================================
setlocal

rem 把 Qt 运行库加入搜索路径（程序启动时需要）
set "PATH=D:\Qt\6.8.3\msvc2022_64\bin;%PATH%"

rem 启动程序
start "" "%~dp0..\build\heima_accounting.exe"
endlocal
