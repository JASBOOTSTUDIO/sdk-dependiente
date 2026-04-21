@echo off
REM ============================================================================
REM verify.bat - Script de verificación para Jasboot Package Manager (JPM)
REM Verifica que la compilación fue exitosa y los módulos funcionan
REM ============================================================================

setlocal enabledelayedexpansion

echo.
echo ========================================
echo JPM - Verificacion de Compilacion
echo ========================================
echo.

set "ERRORS=0"
set "WARNINGS=0"
set "CHECKS=0"

REM ============================================================================
REM Verificar que el ejecutable existe
REM ============================================================================

echo [1] Verificando ejecutable...
set /a CHECKS+=1

if exist "bin\jpm.exe" (
    echo [OK] bin\jpm.exe existe
) else (
    echo [ERROR] bin\jpm.exe no encontrado
    echo        Ejecuta 'build.bat' primero
    set /a ERRORS+=1
    goto :resumen
)

REM ============================================================================
REM Verificar archivos fuente
REM ============================================================================

echo.
echo [2] Verificando archivos fuente...
set /a CHECKS+=1

set "ARCHIVOS_REQUERIDOS=src\jpm_main.c src\jpm_core.c src\jpm_json.c src\jpm_metadata.c src\jpm_commands.c"
set "ARCHIVOS_FALTANTES=0"

for %%f in (%ARCHIVOS_REQUERIDOS%) do (
    if exist "%%f" (
        echo [OK] %%f
    ) else (
        echo [ERROR] %%f no encontrado
        set /a ARCHIVOS_FALTANTES+=1
        set /a ERRORS+=1
    )
)

if %ARCHIVOS_FALTANTES% equ 0 (
    echo [OK] Todos los archivos fuente presentes
)

REM ============================================================================
REM Verificar header
REM ============================================================================

echo.
echo [3] Verificando headers...
set /a CHECKS+=1

if exist "include\jpm.h" (
    echo [OK] include\jpm.h existe
) else (
    echo [ERROR] include\jpm.h no encontrado
    set /a ERRORS+=1
)

REM ============================================================================
REM Verificar tamaño del ejecutable
REM ============================================================================

echo.
echo [4] Verificando tamaño del ejecutable...
set /a CHECKS+=1

for %%A in ("bin\jpm.exe") do set "TAMANO=%%~zA"

if %TAMANO% gtr 10000 (
    echo [OK] Tamaño: %TAMANO% bytes
) else (
    echo [WARN] Tamaño sospechosamente pequeño: %TAMANO% bytes
    set /a WARNINGS+=1
)

REM ============================================================================
REM Verificar que el ejecutable se puede ejecutar
REM ============================================================================

echo.
echo [5] Verificando ejecucion del ejecutable...
set /a CHECKS+=1

bin\jpm.exe >nul 2>&1
if !errorlevel! lss 100 (
    echo [OK] El ejecutable se puede ejecutar
) else (
    echo [WARN] El ejecutable devolvio codigo de error: !errorlevel!
    echo        Esto puede ser normal si no se pasaron argumentos
    set /a WARNINGS+=1
)

REM ============================================================================
REM Verificar archivos de documentación
REM ============================================================================

echo.
echo [6] Verificando documentacion...
set /a CHECKS+=1

set "DOCS_FALTANTES=0"

if exist "MODULOS_IMPLEMENTADOS.md" (
    echo [OK] MODULOS_IMPLEMENTADOS.md
) else (
    echo [WARN] MODULOS_IMPLEMENTADOS.md no encontrado
    set /a DOCS_FALTANTES+=1
)

if exist "README_BUILD.md" (
    echo [OK] README_BUILD.md
) else (
    echo [WARN] README_BUILD.md no encontrado
    set /a DOCS_FALTANTES+=1
)

if exist "RESUMEN_IMPLEMENTACION.md" (
    echo [OK] RESUMEN_IMPLEMENTACION.md
) else (
    echo [WARN] RESUMEN_IMPLEMENTACION.md no encontrado
    set /a DOCS_FALTANTES+=1
)

if %DOCS_FALTANTES% gtr 0 (
    set /a WARNINGS+=1
)

REM ============================================================================
REM Verificar archivos de build
REM ============================================================================

echo.
echo [7] Verificando sistema de build...
set /a CHECKS+=1

if exist "Makefile" (
    echo [OK] Makefile
) else (
    echo [INFO] Makefile no encontrado (opcional en Windows)
)

if exist "build.bat" (
    echo [OK] build.bat
) else (
    echo [WARN] build.bat no encontrado
    set /a WARNINGS+=1
)

REM ============================================================================
REM Verificar test suite
REM ============================================================================

echo.
echo [8] Verificando suite de tests...
set /a CHECKS+=1

if exist "test_modulos.c" (
    echo [OK] test_modulos.c existe

    REM Verificar si test_modulos.exe existe
    if exist "test_modulos.exe" (
        echo [OK] test_modulos.exe compilado

        REM Ejecutar tests
        echo [INFO] Ejecutando tests...
        test_modulos.exe >nul 2>&1
        if !errorlevel! equ 0 (
            echo [OK] Tests pasaron correctamente
        ) else (
            echo [WARN] Algunos tests fallaron (codigo: !errorlevel!)
            set /a WARNINGS+=1
        )
    ) else (
        echo [INFO] test_modulos.exe no compilado
        echo        Puedes compilarlo con: gcc test_modulos.c src\*.c -Iinclude -o test_modulos.exe
    )
) else (
    echo [WARN] test_modulos.c no encontrado
    set /a WARNINGS+=1
)

REM ============================================================================
REM Verificar ejemplo
REM ============================================================================

echo.
echo [9] Verificando archivos de ejemplo...
set /a CHECKS+=1

if exist "ejemplo-jasboot.json" (
    echo [OK] ejemplo-jasboot.json
) else (
    echo [INFO] ejemplo-jasboot.json no encontrado (opcional)
)

REM ============================================================================
REM Resumen
REM ============================================================================

:resumen
echo.
echo ========================================
echo RESUMEN DE VERIFICACION
echo ========================================
echo.
echo Verificaciones realizadas: %CHECKS%
echo Errores:                   %ERRORS%
echo Advertencias:              %WARNINGS%
echo.

if %ERRORS% equ 0 (
    if %WARNINGS% equ 0 (
        echo [EXITO] Compilacion verificada correctamente
        echo.
        echo Puedes usar JPM con:
        echo   bin\jpm.exe init mi-paquete
        echo   bin\jpm.exe list
        echo   bin\jpm.exe clean
        echo.
        exit /b 0
    ) else (
        echo [ADVERTENCIA] Compilacion exitosa con advertencias
        echo Por favor revisa las advertencias arriba
        echo.
        exit /b 0
    )
) else (
    echo [ERROR] Compilacion incompleta o con errores
    echo Por favor corrige los errores indicados
    echo.
    exit /b 1
)
