# Compila el transpilador jasboot-to-c (AOT). Ejecutar desde este directorio.
CC ?= gcc
CFLAGS ?= -std=c11 -Wall -O2 -Iinclude -I../jas-compiler-c/include

COMPILER_SRC := ../jas-compiler-c/src/lexer.c \
	../jas-compiler-c/src/parser.c \
	../jas-compiler-c/src/nodes.c \
	../jas-compiler-c/src/token_vec.c \
	../jas-compiler-c/src/diagnostic.c \
	../jas-compiler-c/src/keywords.c \
	../jas-compiler-c/src/sistema_llamadas.c \
	../jas-compiler-c/src/symbol_table.c

TRANS_SRC := src/main.c src/codegen_c.c src/aot_usar_merge.c

.PHONY: all clean

all: jbc-to-c

jbc-to-c: $(TRANS_SRC) $(COMPILER_SRC)
	$(CC) $(CFLAGS) $(TRANS_SRC) $(COMPILER_SRC) -o $@

clean:
	rm -f jbc-to-c
