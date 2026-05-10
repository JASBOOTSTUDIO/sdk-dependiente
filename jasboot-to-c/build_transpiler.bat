@echo off
echo Construyendo Jasboot to C Transpiler (AOT)...

set CC=gcc
set CFLAGS=-std=c11 -Wall -Iinclude -I../jas-compiler-c/include

REM Fuentes del compilador original necesarios para el AST
set COMPILER_SRC=../jas-compiler-c/src/lexer.c ^
                 ../jas-compiler-c/src/parser.c ^
                 ../jas-compiler-c/src/nodes.c ^
                 ../jas-compiler-c/src/token_vec.c ^
                 ../jas-compiler-c/src/diagnostic.c ^
                 ../jas-compiler-c/src/keywords.c ^
                 ../jas-compiler-c/src/symbol_table.c ^
                 ../jas-compiler-c/src/sistema_llamadas.c

%CC% %CFLAGS% src/main.c src/codegen_c.c src/aot_usar_merge.c %COMPILER_SRC% -o jbc-to-c.exe

if %ERRORLEVEL% EQU 0 (
    echo Construccion exitosa: jbc-to-c.exe
) else (
    echo Error en la construccion.
)
