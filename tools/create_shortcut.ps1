# ==========================================
# 黑马记账 - 在桌面创建快捷方式
# 用法：右键本文件 → 使用 PowerShell 运行
# ==========================================

# 获取真实的桌面路径（支持桌面被重定向到 OneDrive 等情况）
$desktop = [Environment]::GetFolderPath('Desktop')

# 程序路径（发布版文件夹里的主程序）
$exePath = 'D:\黑马记账 app\release\heima_accounting.exe'

if (-not (Test-Path $exePath)) {
    Write-Output "找不到程序：$exePath（请先运行 build.bat 编译，并生成发布版）"
    Read-Host "按回车退出"
    exit 1
}

# 创建快捷方式
$ws = New-Object -ComObject WScript.Shell
$sc = $ws.CreateShortcut((Join-Path $desktop '黑马记账.lnk'))
$sc.TargetPath = $exePath
$sc.WorkingDirectory = Split-Path $exePath
$sc.Description = '黑马记账'
$sc.Save()

Write-Output "快捷方式已创建在桌面：黑马记账.lnk"
