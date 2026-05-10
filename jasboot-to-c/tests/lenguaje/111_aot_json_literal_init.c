#include "jasboot_rt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>


int main(int argc, char **argv) {
    jb_init();
    jb_set_argv(argc, argv);
    jb_imprimir(jb_json_stringify(jb_new_nulo(), jb_new_entero(0)));
    jb_imprimir(jb_json_stringify(jb_new_nulo(), jb_new_entero(0)));
    jb_cleanup();
    return 0;
}
