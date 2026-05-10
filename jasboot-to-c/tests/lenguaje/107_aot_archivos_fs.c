#include "jasboot_rt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>


int main(int argc, char **argv) {
    jb_init();
    jb_set_argv(argc, argv);
    jb_var_t ruta = jb_new_texto("tmp_107_fs.txt");
    jb_var_t h = jb_fs_abrir(ruta, jb_new_texto("w"));
    jb_fs_escribir(jb_new_texto("prueba con jasboot-to-c\n"), h);
    jb_fs_escribir(jb_new_texto("uno\n"), h);
    jb_fs_escribir(jb_new_texto("dos\n"), h);
    jb_fs_escribir(jb_new_texto("tres\n"), h);
    jb_fs_cerrar(h);
    jb_imprimir(jb_existe_archivo(ruta));
    jb_assign(&h, jb_fs_abrir(ruta, jb_new_texto("r")));
    jb_imprimir(jb_fs_leer_linea(h));
    jb_imprimir(jb_fs_leer_linea(h));
    jb_var_t l3 = jb_fs_leer_linea(h);
    jb_imprimir(jb_entero_a_texto(jb_texto_len(l3)));
    jb_fs_cerrar(h);
    jb_cleanup();
    return 0;
}
