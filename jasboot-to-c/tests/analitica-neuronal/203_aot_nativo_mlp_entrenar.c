#include "jasboot_rt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>


int main(int argc, char **argv) {
    jb_init();
    jb_set_argv(argc, argv);
    jb_var_t capas_oc = jb_new_list();
    jb_list_push(&capas_oc, jb_new_entero(1));
    jb_var_t X = jb_new_list();
    jb_var_t fila_x = jb_new_list();
    jb_list_push(&fila_x, jb_new_flotante(1));
    jb_list_push(&X, fila_x);
    jb_var_t yy = jb_new_list();
    jb_list_push(&yy, jb_new_flotante(0.5));
    jb_var_t pesos = jb_new_list();
    jb_var_t L0 = jb_new_list();
    jb_var_t r0 = jb_new_list();
    jb_list_push(&r0, jb_new_flotante(0.1));
    jb_list_push(&L0, r0);
    jb_var_t L1 = jb_new_list();
    jb_var_t r1 = jb_new_list();
    jb_list_push(&r1, jb_new_flotante(0.1));
    jb_list_push(&L1, r1);
    jb_list_push(&pesos, L0);
    jb_list_push(&pesos, L1);
    jb_var_t sesgos = jb_new_list();
    jb_var_t S0 = jb_new_list();
    jb_list_push(&S0, jb_new_flotante(0));
    jb_var_t S1 = jb_new_list();
    jb_list_push(&S1, jb_new_flotante(0));
    jb_list_push(&sesgos, S0);
    jb_list_push(&sesgos, S1);
    jb_var_t mse = jb_nativo_mlp_entrenar(pesos, sesgos, X, yy, capas_oc, jb_new_flotante(0.3), jb_new_entero(50));
    jb_imprimir(jb_decimal(mse, jb_new_entero(6)));
    jb_cleanup();
    return 0;
}
