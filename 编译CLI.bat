@echo off
chcp 65001 >nul
title 编译 jellyfish-cli - 水母工作室

cls
echo.
echo  ^+---------------------------------------^+
echo  ^|    水母工作室 / Jellyfish Studio       ^|
echo  ^+---------------------------------------^+
echo.
echo  编译目标: jellyfish-cli.exe
echo.

if not exist "jellyfish-cli.cpp" (
    echo  [错误] 未找到 jellyfish-cli.cpp
    pause
    exit /b 1
)

where g++ >nul 2>nul
if %errorlevel% neq 0 (
    echo  [错误] 未找到 g++
    pause
    exit /b 1
)

echo  [OK] 源文件检查通过
echo  [OK] 编译器环境正常
echo.
echo  [编译] 正在编译 jellyfish-cli.cpp ...
echo.

g++ -std=c++17 -O2 -s -static -static-libgcc -static-libstdc++ jellyfish-cli.cpp -o jellyfish-cli.exe -lole32 -lwinmm

if %errorlevel% neq 0 (
    echo.
    echo  [错误] 编译失败
    pause
    exit /b 1
)

if not exist "jellyfish-cli.exe" (
    echo.
    echo  [错误] 未生成 exe 文件
    pause
    exit /b 1
)

echo.
echo  ^+---------------------------------------^+
echo  ^|  [成功] jellyfish-cli.exe 已生成       ^|
echo  ^+---------------------------------------^+
echo.
for %%A in ("jellyfish-cli.exe") do echo  文件大小: %%~zA 字节
echo.
echo  运行: jellyfish-cli.exe
echo.
pause