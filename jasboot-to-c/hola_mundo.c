#include "runtime/jasboot_rt.h"

int main() {
    jb_init();
        jb_imprimir(jb_new_texto("Test 1: Hola Mundo Nativo"));
    jb_cleanup();
    return 0;
}
