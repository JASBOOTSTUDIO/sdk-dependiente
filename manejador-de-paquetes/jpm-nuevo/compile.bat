@echo off
REM Script de compilación simple para JPM
REM Copyright (c) 2024 Jasboot Team

echo ========================================
echo Compilando Jasboot Package Manager
echo ========================================

REM Crear directorios
if not exist build mkdir build
if not exist bin mkdir bin

REM Flags de compilación
set CFLAGS=-std=c11 -Wall -Wextra -Wpedantic -Iinclude -O2 -DNDEBUG
set LDFLAGS=

echo.
echo [1/9] Compilando jpm_core.c...
gcc %CFLAGS% -c src/jpm_core.c -o build/jpm_core.o
if %errorlevel% neq 0 goto :error

echo [2/9] Compilando jpm_json.c...
gcc %CFLAGS% -c src/jpm_json.c -o build/jpm_json.o
if %errorlevel% neq 0 goto :error

echo [3/9] Compilando jpm_metadata.c...
gcc %CFLAGS% -c src/jpm_metadata.c -o build/jpm_metadata.o
if %errorlevel% neq 0 goto :error

echo [4/9] Compilando jpm_commands.c...
gcc %CFLAGS% -c src/jpm_commands.c -o build/jpm_commands.o
if %errorlevel% neq 0 goto :error

echo [5/9] Compilando jpm_pack.c...
gcc %CFLAGS% -c src/jpm_pack.c -o build/jpm_pack.o
if %errorlevel% neq 0 goto :error

echo [6/9] Compilando jpm_install.c...
gcc %CFLAGS% -c src/jpm_install.c -o build/jpm_install.o
if %errorlevel% neq 0 goto :error

echo [7/9] Compilando jpm_lock.c...
gcc %CFLAGS% -c src/jpm_lock.c -o build/jpm_lock.o
if %errorlevel% neq 0 goto :error

echo [8/9] Compilando jpm_main.c...
gcc %CFLAGS% -c src/jpm_main.c -o build/jpm_main.o
if %errorlevel% neq 0 goto :error

echo [9/9] Enlazando jpm.exe...
gcc %LDFLAGS% build/jpm_main.o build/jpm_core.o build/jpm_json.o build/jpm_metadata.o build/jpm_commands.o build/jpm_pack.o build/jpm_install.o build/jpm_lock.o -o bin/jpm.exe
if %errorlevel% neq 0 goto :error

echo.
echo ========================================
echo Compilacion exitosa!
echo Ejecutable: bin\jpm.exe
echo ========================================
echo.
echo Prueba: bin\jpm.exe --version
echo.

goto :end

:error
echo.
echo ========================================
echo ERROR: La compilacion fallo
echo ========================================
exit /b 1

:end
