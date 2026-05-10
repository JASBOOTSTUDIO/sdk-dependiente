#include "jasboot_rt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>

static jb_var_t MOD_EXPORT_K;

int main(int argc, char **argv) {
    jb_init();
    jb_set_argv(argc, argv);
    MOD_EXPORT_K = jb_new_entero(42);
    jb_imprimir(jb_texto_desde_numero(MOD_EXPORT_K));
    jb_cleanup();
    return 0;
}
