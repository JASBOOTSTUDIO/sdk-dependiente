#ifndef CODEGEN_C_H
#define CODEGEN_C_H

#include "../../jas-compiler-c/include/nodes.h"
#include <stdio.h>

/* Genera código C a partir de un nodo del AST (modo permisivo: stubs jb_warn_aot). */
void jbc_generate_c(ASTNode* node, FILE* out);

/*
 * strict != 0: aborta el proceso (exit 2) si el programa usa construcciones no soportadas
 * en AOT (en lugar de emitir jb_warn_aot). out_path: ruta del .c generado; si no es NULL,
 * se intenta borrar el archivo parcial antes de salir.
 */
void jbc_generate_c_opts(ASTNode* node, FILE* out, const char* out_path, int strict);

#endif
