#include "jasboot_rt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>


int main(int argc, char **argv) {
    jb_init();
    jb_set_argv(argc, argv);
    jb_crear_memoria(jb_new_texto("sdk-dependiente/jasboot-to-c/tests/lenguaje/117_mem.jmn"));
    jb_define_concepto(jb_new_texto("ia"), jb_new_texto("inteligencia"));
    jb_asociar(jb_new_texto("ia"), jb_new_texto("ml"), jb_new_flotante(0.7));
    jb_aprender_concepto(jb_new_texto("ia"), jb_new_flotante(0.2));
    jb_buscar(jb_new_texto("ia"));
    jb_imprimir(jb_resultado_global);
    jb_cerrar_memoria();
    jb_cleanup();
    return 0;
}
