@echo off
REM ============================================================================
REM build.bat - Script de compilación para Jasboot Package Manager (JPM)
REM Copyright (c) 2024 Jasboot Team
REM ============================================================================

setlocal enabledelayedexpansion

REM Configuración
set "PROJECT_NAME=JPM"
set "TARGET=jpm.exe"
set "SRC_DIR=src"
set "INC_DIR=include"
set "OBJ_DIR=build"
set "BIN_DIR=bin"

REM Compilador (ajustar según tu instalación)
set "CC=gcc"

REM Verificar si GCC está disponible
where gcc >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] GCC no encontrado en PATH
    echo Por favor instala MinGW o añade GCC a tu PATH
    echo Descarga: https://www.mingw-w64.org/
    exit /b 1
)

REM Flags del compilador
set "CFLAGS=-std=c11 -Wall -Wextra -I%INC_DIR%"
set "LDFLAGS="
set "LIBS="

REM Determinar modo de compilación
set "BUILD_TYPE=release"
if /i "%1"=="debug" (
    set "CFLAGS=%CFLAGS% -g -O0 -DDEBUG"
    set "BUILD_TYPE=debug"
) else (
    set "CFLAGS=%CFLAGS% -O2 -DNDEBUG"
)

REM ============================================================================
REM Procesamiento de comandos
REM ============================================================================

if /i "%1"=="" goto :build
if /i "%1"=="build" goto :build
if /i "%1"=="debug" goto :build
if /i "%1"=="release" goto :build
if /i "%1"=="clean" goto :clean
if /i "%1"=="distclean" goto :distclean
if /i "%1"=="help" goto :help
if /i "%1"=="info" goto :info
if /i "%1"=="test" goto :test

echo [ERROR] Comando desconocido: %1
goto :help

REM ============================================================================
REM BUILD
REM ============================================================================

:build
echo ========================================
echo %PROJECT_NAME% - Compilando (%BUILD_TYPE%)
echo ========================================
echo.

REM Crear directorios si no existen
if not exist "%OBJ_DIR%" (
    echo [%PROJECT_NAME%] Creando directorio %OBJ_DIR%...
    mkdir "%OBJ_DIR%"
)

if not exist "%BIN_DIR%" (
    echo [%PROJECT_NAME%] Creando directorio %BIN_DIR%...
    mkdir "%BIN_DIR%"
)

REM Lista de archivos fuente
set "SOURCES=jpm_main.c jpm_core.c jpm_json.c jpm_metadata.c jpm_commands.c jpm_pack.c jpm_install.c jpm_lock.c"
set "OBJECTS="

REM Compilar cada archivo fuente
for %%f in (%SOURCES%) do (
    echo [%PROJECT_NAME%] Compilando %SRC_DIR%\%%f...
    %CC% %CFLAGS% -c "%SRC_DIR%\%%f" -o "%OBJ_DIR%\%%~nf.o"
    if !errorlevel! neq 0 (
        echo [ERROR] Fallo al compilar %%f
        exit /b 1
    )
    set "OBJECTS=!OBJECTS! %OBJ_DIR%\%%~nf.o"
)

REM Enlazar
echo.
echo [%PROJECT_NAME%] Enlazando %TARGET%...
%CC% %OBJECTS% %LDFLAGS% %LIBS% -o "%BIN_DIR%\%TARGET%"
if %errorlevel% neq 0 (
    echo [ERROR] Fallo al enlazar
    exit /b 1
)

echo.
echo ========================================
echo Build exitoso: %BIN_DIR%\%TARGET%
echo ========================================
echo.
echo Para ejecutar:
echo   %BIN_DIR%\%TARGET% --help
echo.
goto :end

REM ============================================================================
REM CLEAN
REM ============================================================================

:clean
echo [%PROJECT_NAME%] Limpiando archivos de compilación...

if exist "%OBJ_DIR%" (
    echo [%PROJECT_NAME%] Eliminando %OBJ_DIR%...
    rmdir /S /Q "%OBJ_DIR%"
)

if exist "%BIN_DIR%" (
    echo [%PROJECT_NAME%] Eliminando %BIN_DIR%...
    rmdir /S /Q "%BIN_DIR%"
)

echo [%PROJECT_NAME%] Limpieza completa
goto :end

REM ============================================================================
REM DISTCLEAN
REM ============================================================================

:distclean
call :clean

echo [%PROJECT_NAME%] Eliminando archivos generados adicionales...

if exist "*.exe" del /Q *.exe
if exist "*.o" del /Q *.o
if exist "*.obj" del /Q *.obj

echo [%PROJECT_NAME%] Limpieza completa (distclean)
goto :end

REM ============================================================================
REM INFO
REM ============================================================================

:info
echo ========================================
echo %PROJECT_NAME% - Información de Build
echo ========================================
echo.
echo Compilador:     %CC%
echo Target:         %TARGET%
echo Build Type:     %BUILD_TYPE%
echo Source Dir:     %SRC_DIR%
echo Object Dir:     %OBJ_DIR%
echo Binary Dir:     %BIN_DIR%
echo Flags:          %CFLAGS%
echo Libraries:      %LIBS%
echo.
echo Archivos fuente:
for %%f in (%SOURCES%) do (
    echo   - %SRC_DIR%\%%f
)
echo.
echo ========================================
goto :end

REM ============================================================================
REM TEST
REM ============================================================================

:test
if not exist "%BIN_DIR%\%TARGET%" (
    echo [ERROR] El ejecutable no existe. Ejecuta 'build.bat build' primero.
    exit /b 1
)

echo [%PROJECT_NAME%] Ejecutando tests básicos...
echo.

REM Test 1: Verificar que el ejecutable funciona
echo Test 1: Verificar ejecución...
"%BIN_DIR%\%TARGET%" 2>nul
if !errorlevel! equ 0 (
    echo [OK] Ejecutable funciona
) else (
    echo [INFO] Ejecutable devolvió código de error ^(puede ser normal sin argumentos^)
)

echo.
echo [%PROJECT_NAME%] Tests completados
goto :end

REM ============================================================================
REM HELP
REM ============================================================================

:help
echo ========================================
echo %PROJECT_NAME% - Script de Compilación
echo ========================================
echo.
echo Uso: build.bat [comando]
echo.
echo Comandos disponibles:
echo   build       - Compilar versión release ^(por defecto^)
echo   debug       - Compilar con símbolos de depuración
echo   release     - Compilar versión optimizada
echo   clean       - Eliminar archivos de compilación
echo   distclean   - Eliminar todos los archivos generados
echo   info        - Mostrar información de configuración
echo   test        - Ejecutar tests básicos
echo   help        - Mostrar este mensaje
echo.
echo Ejemplos:
echo   build.bat              - Compilar versión release
echo   build.bat debug        - Compilar versión debug
echo   build.bat clean        - Limpiar archivos compilados
echo   build.bat info         - Ver configuración
echo.
echo Requisitos:
echo   - GCC ^(MinGW^) instalado y en PATH
echo   - Windows 7 o superior
echo.
echo ========================================
goto :end

REM ============================================================================
REM FIN
REM ============================================================================

:end
endlocal
exit /b 0
