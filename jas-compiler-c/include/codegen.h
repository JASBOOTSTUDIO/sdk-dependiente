/* CodeGen - emisión de IR .jbo */

#ifndef CODEGEN_H
#define CODEGEN_H

#include "nodes.h"
#include <stddef.h>
#include <stdint.h>

typedef struct CodeGen CodeGen;

CodeGen *codegen_create(void);
void codegen_free(CodeGen *cg);

/* Registrar firma de función externa (de módulos usar) para inferencia de tipos */
void codegen_register_external_func(CodeGen *cg, const char *name, const char *return_type);

const char *codegen_get_error(CodeGen *cg, int *out_line, int *out_col);
/* Si no es NULL, el error esta en ese .jasb (modulo fusionado); si NULL usar el archivo de entrada. */
const char *codegen_get_error_unit_path(const CodeGen *cg);

/* Errores semanticos acumulados en un solo pase (p. ej. varias variables no declaradas). */
size_t codegen_collected_diag_count(const CodeGen *cg);
/* idx < count. msg y unit_path son validos hasta codegen_free (no liberar). */
void codegen_collected_diag_at(const CodeGen *cg, size_t idx,
                               const char **msg, int *line, int *col, const char **unit_path);

/* Genera binario .jbo; retorna buffer (ownership al caller, free) y longitud */
uint8_t *codegen_generate(CodeGen *cg, ASTNode *ast, size_t *out_len);

#endif
