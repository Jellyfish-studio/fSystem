@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion
title f系统验证工具

echo ========================================
echo f系统1.0 验证脚本
echo ========================================
echo 果冻鱼工作室 f系统1.0 官方验证脚本
echo 水母严选，必是佳品
echo 任何从非GitHub fSystem谷仓中下载的验证脚本都是盗版
echo 准备完成！
echo ========================================
echo.
echo ============================================================
echo     警告
echo ============================================================
echo.
echo   请确保本工具从官方渠道下载
echo.
echo   官方渠道
echo     GitHub: https://github.com/JellyfishStudio/fSystem
echo     邮箱: JammyJellyfish@hotmail.com
echo.
echo   警告
echo     非官方版本可能存在安全风险
echo     修改版本可能包含恶意代码或后门
echo     请勿运行来历不明的修改版本
echo.
echo   本工具将验证 main.cpp 是否为官方原始版本
echo.
echo ============================================================
echo.
echo 按任意键开始验证
pause >nul
echo.

:: ========== 配置 ==========
set OFFICIAL_DATE=2026/08/09
set OFFICIAL_SIZE=618967
:: ===========================

if not exist "main.cpp" (
    echo.
    echo [错误] 找不到 main.cpp
    echo.
    pause
    exit /b 1
)

echo [1/4] 检查 main.cpp
echo.

for %%f in (main.cpp) do (
    set "FILE_DATE=%%~tf"
    set "FILE_SIZE=%%~zf"
)

echo     修改时间: !FILE_DATE!
echo     文件大小: !FILE_SIZE! 字节

echo.
echo [2/4] 验证日期
echo.

set "FILE_DATE_ONLY=!FILE_DATE:~0,10!"

if "!FILE_DATE_ONLY!"=="%OFFICIAL_DATE%" (
    echo     [通过] 日期匹配: !FILE_DATE_ONLY!
    set "DATE_OK=1"
) else (
    echo     [失败] 日期不匹配
    echo         当前: !FILE_DATE_ONLY!
    echo         官方: %OFFICIAL_DATE%
    set "DATE_OK=0"
)

echo.
echo [3/4] 验证文件大小
echo.

if "!FILE_SIZE!"=="%OFFICIAL_SIZE%" (
    echo     [通过] 大小匹配: !FILE_SIZE! 字节
    set "SIZE_OK=1"
) else (
    echo     [警告] 大小不匹配
    echo         当前: !FILE_SIZE! 字节
    echo         官方: %OFFICIAL_SIZE% 字节
    set "SIZE_OK=0"
)

echo.
echo [4/4] 验证结果
echo ========================================
echo.

if "%DATE_OK%"=="1" (
    if "%SIZE_OK%"=="1" (
        echo.
        echo     ****************************************
        echo     *                                      *
        echo     *          验证通过                    *
        echo     *                                      *
        echo     ****************************************
        echo.
        echo     这是官方原始版本
        echo     可以放心使用
    ) else (
        echo.
        echo     ****************************************
        echo     *                                      *
        echo     *          验证失败                    *
        echo     *                                      *
        echo     ****************************************
        echo.
        echo     文件大小不匹配，可能已被修改，请从官方渠道重新获取
    )
) else (
    echo.
    echo     ****************************************
    echo     *                                      *
    echo     *          验证失败                    *
    echo     *                                      *
    echo     ****************************************
    echo.
    echo     文件日期不匹配，可能已被修改，请从官方渠道重新获取
)

echo.
echo ========================================
echo   验证报告
echo    文件 main.cpp
echo    大小 !FILE_SIZE! 字节
echo    修改时间 !FILE_DATE!
echo ========================================
echo.

echo 按任意键退出
pause >nul