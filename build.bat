@echo off
set CXX=g++
set CXX_FLAGS=-std=c++23 -Ofast -static-libstdc++ -static-libgcc -mwindows
set BIN=bin
set SRC=src
set INCLUDE=include\
set LIB=
set EXECUTABLE=usb_copier.exe

if not exist %BIN% mkdir %BIN%

echo Compiling the C++ program...
%CXX% %CXX_FLAGS% -I%INCLUDE% %LIB% %SRC%\*.cpp -o %BIN%\%EXECUTABLE% %LIB%
if %errorlevel% neq 0 (
    echo Compilation failed.
    exit /b %errorlevel%
)
echo Done.