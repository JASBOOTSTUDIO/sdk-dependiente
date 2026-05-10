#ifndef AOT_USAR_MERGE_H
#define AOT_USAR_MERGE_H

#include "nodes.h"

/**
 * Fusiona en el AST las directivas `usar ... de "ruta.jasb"` (mismo criterio que jbc:
 * funciones con `enviar`, sub-modulos recursivos). Debe llamarse tras parser_parse y
 * antes de jbc_generate_c. Devuelve 0 si OK, distinto de 0 si error (mensajes en stderr).
 */
int aot_merge_usar_modules(ASTNode *program, const char *entry_source_path);

#endif
