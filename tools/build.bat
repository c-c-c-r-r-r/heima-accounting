@echo off
rem ==========================================
rem 黑马记账 - 一键编译脚本
rem 双击运行即可完成编译（需要先装好 Qt）
rem ==========================================
setlocal

rem 1. 启用 Visual Studio 的 C++ 编译环境
rem    （先让 vcvars 找到定位工具 vswhere.exe，否则编译器路径会设置不全）
set "PATH=D:\;%PATH%"
call "D:\vs2\VC\Auxiliary\Build\vcvars64.bat" >nul

rem 2. 把 VS 自带的 cmake 和 ninja 加入搜索路径
set "PATH=D:\vs2\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;D:\vs2\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%"

set "ROOT=%~dp0.."

rem 3. 生成编译配置（第一次运行需要，之后可复用）
cmake -S "%ROOT%" -B "%ROOT%\build" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=D:/Qt/6.8.3/msvc2022_64
if errorlevel 1 (
    echo.
    echo 编译配置失败，请把上面的报错信息发给 Claude。
    pause
    exit /b 1
)

rem 4. 开始编译
cmake --build "%ROOT%\build"
if errorlevel 1 (
    echo.
    echo 编译失败，请把上面的报错信息发给 Claude。
    pause
    exit /b 1
)

echo.
echo 编译成功！可以双击 tools\run.bat 运行程序。
pause
endlocal
