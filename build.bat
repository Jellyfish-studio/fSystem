@echo off
chcp 65001 >nul
title 编译 f系统 - 水母工作室

cls
echo ========================================
echo f系统1.0 编译脚本
echo ========================================
echo 果冻鱼工作室 f系统1.0 官方编译脚本
echo 水母严选，必是佳品
echo 任何从非GitHub fSystem谷仓中下载的编译脚本都是盗版
echo 准备完成！
echo ========================================
echo.

REM ---- 设置中文环境（尝试让 g++ 输出中文报错） ----
set LANG=zh_CN.UTF-8
set LC_ALL=zh_CN.UTF-8

REM ---- 编译参数 ----
set FLAGS=-std=c++17 -O2 -finput-charset=gbk -fexec-charset=gbk -finput-charset=UTF-8
set INC=-I"nunn-1.56\nunn"

echo [1/8] 编译 main.cpp...
g++ -c main.cpp -o main.o %INC% %FLAGS%
if errorlevel 1 goto error

echo [2/8] 编译 encrypt.cpp...
g++ -c encrypt.cpp -o encrypt.o %INC% %FLAGS%
if errorlevel 1 goto error

echo [3/8] 编译 nu_mlpnn.cc...
g++ -c nunn-1.56\nunn\nu_mlpnn.cc -o nu_mlpnn.o %INC% %FLAGS%
if errorlevel 1 goto error

echo [4/8] 编译 nu_rmlpnn.cc...
g++ -c nunn-1.56\nunn\nu_rmlpnn.cc -o nu_rmlpnn.o %INC% %FLAGS%
if errorlevel 1 goto error

echo [5/8] 编译 nu_perceptron.cc...
g++ -c nunn-1.56\nunn\nu_perceptron.cc -o nu_perceptron.o %INC% %FLAGS%
if errorlevel 1 goto error

echo [6/8] 编译 nu_hopfieldnn.cc...
g++ -c nunn-1.56\nunn\nu_hopfieldnn.cc -o nu_hopfieldnn.o %INC% %FLAGS%
if errorlevel 1 goto error

echo [7/8] 编译 nu_costfuncs.cc...
g++ -c nunn-1.56\nunn\nu_costfuncs.cc -o nu_costfuncs.o %INC% %FLAGS%
if errorlevel 1 goto error

echo [8/8] 链接生成 fOS.exe...
g++ main.o encrypt.o nu_mlpnn.o nu_rmlpnn.o nu_perceptron.o nu_hopfieldnn.o nu_costfuncs.o f1.0_private.res -o "fOS.exe" -mwindows -lwinmm -limm32 -lole32 -lwlanapi -liphlpapi -static -static-libgcc -static-libstdc++
if errorlevel 1 goto error

echo.
echo ========================================
echo 编译成功！生成 fOS.exe
echo ========================================
pause
exit

:error
echo.
echo ========================================
echo 编译失败！
echo ========================================
pause
exit