#include "jasboot_rt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>


int main(int argc, char **argv) {
    jb_init();
    jb_set_argv(argc, argv);
    jb_var_t p = jb_new_map();
    jb_put_member_leaf(&p, "x", jb_new_entero(3));
    jb_put_member_leaf(&p, "yy", jb_new_entero(4));
    jb_imprimir(jb_texto_desde_numero(jb_add(jb_member_get(p, "x"), jb_member_get(p, "yy"))));
    jb_cleanup();
    return 0;
}
