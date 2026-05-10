#include "jasboot_rt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>


int main(int argc, char **argv) {
    jb_init();
    jb_set_argv(argc, argv);
    jb_crear_memoria(jb_new_texto("sdk-dependiente/jasboot-to-c/tests/lenguaje/119_mem.jmn"));
    jb_recordar(jb_new_texto("k1"), jb_new_texto("v1"));
    jb_imprimir(jb_mem_obtener_fuerza(jb_new_texto("k1"), jb_new_texto("v1")));
    (jb_consolidar_memoria(), jb_new_nulo());
    jb_cerrar_memoria();
    jb_cleanup();
    return 0;
}
