@echo off
setlocal enabledelayedexpansion

echo ===================================================
echo   TEST RUNNER - JASBOOT TO C (AOT)
echo ===================================================

set TEST_DIR=tests\lenguaje
set RUNTIME_DIR=runtime
set BUILD_DIR=build_tests

if not exist %BUILD_DIR% mkdir %BUILD_DIR%

echo.
echo 1. Compilando Transpilador...
call build_transpiler.bat

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] No se pudo construir el transpilador.
    exit /b 1
)

echo.
echo 2. Ejecutando Bateria de Tests...
echo.

for %%f in (%TEST_DIR%\*.jasb) do (
    set TEST_NAME=%%~nf
    echo [TEST] !TEST_NAME!
    
    REM En el prototipo, jbc-to-c genera hola_mundo.c siempre.
    REM Para los tests, simulamos el flujo completo:
    
    .\jbc-to-c.exe %%f > nul
    
    if exist hola_mundo.c (
        gcc hola_mundo.c %RUNTIME_DIR%\jasboot_rt.c -I%RUNTIME_DIR% -o %BUILD_DIR%\!TEST_NAME!.exe
        
        if !ERRORLEVEL! EQU 0 (
            echo   - Compilacion Nativa: OK
            echo   - Resultado:
            %BUILD_DIR%\!TEST_NAME!.exe
        ) else (
            echo   - [ERROR] Fallo la compilacion de C.
        )
    ) else (
        echo   - [ERROR] El transpilador no genero el archivo .c
    )
    echo ---------------------------------------------------
)

echo.
echo Pruebas finalizadas.
