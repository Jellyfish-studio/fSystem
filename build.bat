@echo off
chcp 65001 >nul
echo ========================================
echo f系统1.0 编译脚本
echo ========================================
echo 果冻鱼工作室 f系统1.0 官方编译脚本
echo 水母严选，必是佳品
echo 准备完成！
echo ========================================
echo.

set "GXX=C:\Program Files\RedPanda-Cpp\mingw64\bin\g++.exe"
set FLAGS=-std=c++17 -O2 -finput-charset=UTF-8 -fexec-charset=gbk
set INC=-I"nunn-1.56\nunn"

echo [1/7] 编译 main.cpp...
"%GXX%" -c main.cpp -o main.o %INC% %FLAGS%
if errorlevel 1 goto error

echo [2/7] 编译 nu_mlpnn.cc...
"%GXX%" -c nunn-1.56\nunn\nu_mlpnn.cc -o nu_mlpnn.o %INC% %FLAGS%
if errorlevel 1 goto error

echo [3/7] 编译 nu_rmlpnn.cc...
"%GXX%" -c nunn-1.56\nunn\nu_rmlpnn.cc -o nu_rmlpnn.o %INC% %FLAGS%
if errorlevel 1 goto error

echo [4/7] 编译 nu_perceptron.cc...
"%GXX%" -c nunn-1.56\nunn\nu_perceptron.cc -o nu_perceptron.o %INC% %FLAGS%
if errorlevel 1 goto error

echo [5/7] 编译 nu_hopfieldnn.cc...
"%GXX%" -c nunn-1.56\nunn\nu_hopfieldnn.cc -o nu_hopfieldnn.o %INC% %FLAGS%
if errorlevel 1 goto error

echo [6/7] 编译 nu_costfuncs.cc...
"%GXX%" -c nunn-1.56\nunn\nu_costfuncs.cc -o nu_costfuncs.o %INC% %FLAGS%
if errorlevel 1 goto error

echo [7/7] 链接生成 f系统1.0.exe...
"%GXX%" main.o nu_mlpnn.o nu_rmlpnn.o nu_perceptron.o nu_hopfieldnn.o nu_costfuncs.o f1.0_private.res -o "f系统1.0.exe" -mwindows -lwinmm -limm32 -lole32 -lwlanapi -liphlpapi -static -static-libgcc -static-libstdc++
if errorlevel 1 goto error

echo.
echo ========================================
echo 编译成功！生成 f系统1.0.exe
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