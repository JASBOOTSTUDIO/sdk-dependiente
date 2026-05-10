#include "jasboot_rt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>

static jb_var_t jbf_pl_ui_banner(jb_var_t modo);
static jb_var_t jbf_pl_ui_subtitulo(jb_var_t modo);
static jb_var_t jbf_pl_ui_opciones(jb_var_t modo);
static jb_var_t jbf_pl_json_texto_inicial(jb_var_t modo);
static jb_var_t jbf_pl_json_tipar_raiz(jb_var_t raw);
static jb_var_t jbf_pl_json_conteo_items(jb_var_t raw);
static jb_var_t jbf_pl_bin_ruta_temp(jb_var_t modo);
static jb_var_t jbf_pl_bin_escribir_marcador(jb_var_t ruta);
static jb_var_t jbf_pl_bin_verificar_marcador(jb_var_t ruta);
static jb_var_t jbf_pl_bin_estres_escritura(jb_var_t ruta, jb_var_t n);

static jb_var_t jbf_pl_ui_banner(jb_var_t modo) {
    return jb_new_texto("========== Laboratorio JSON + binario (3 modulos) ==========");
    return jb_new_nulo();
}

static jb_var_t jbf_pl_ui_subtitulo(jb_var_t modo) {
    return jb_new_texto("Menu: JSON (modulo 2), binario en disco (modulo 3), textos aqui (modulo 1).");
    return jb_new_nulo();
}

static jb_var_t jbf_pl_ui_opciones(jb_var_t modo) {
    return jb_new_texto("1 Ayuda | 2 JSON demo | 3 Bin marcador | 4 Bin estres N | 5 JSON linea propia | 6 Borrar bin demo | 0 Salir");
    return jb_new_nulo();
}

static jb_var_t jbf_pl_json_texto_inicial(jb_var_t modo) {
    return jb_new_texto("{\"version\":1,\"items\":[{\"id\":1,\"nombre\":\"alfa\"},{\"id\":2,\"nombre\":\"beta\"}],\"blob_meta\":{\"bytes\":4}}");
    return jb_new_nulo();
}

static jb_var_t jbf_pl_json_tipar_raiz(jb_var_t raw) {
    jb_var_t j = jb_json_parse(raw);
    return jb_json_tipo(j);
    return jb_new_nulo();
}

static jb_var_t jbf_pl_json_conteo_items(jb_var_t raw) {
    jb_var_t j = jb_json_parse(raw);
    jb_var_t arr = jb_json_objeto_obtener(j, jb_new_texto("items"));
    return jb_json_lista_tamano(arr);
    return jb_new_nulo();
}

static jb_var_t jbf_pl_bin_ruta_temp(jb_var_t modo) {
    return jb_new_texto("tmp_pesado_lab_magic.bin");
    return jb_new_nulo();
}

static jb_var_t jbf_pl_bin_escribir_marcador(jb_var_t ruta) {
    jb_var_t h = jb_fs_abrir(ruta, jb_new_texto("wb"));
    jb_fs_escribir_byte(jb_new_entero(90), h);
    jb_fs_escribir_byte(jb_new_entero(66), h);
    jb_fs_escribir_byte(jb_new_entero(49), h);
    jb_fs_cerrar(h);
    return jb_new_entero(1);
    return jb_new_nulo();
}

static jb_var_t jbf_pl_bin_verificar_marcador(jb_var_t ruta) {
    if (jb_truthy(jb_eq(jb_existe_archivo(ruta), jb_new_entero(0)))) {
        return jb_new_entero(0);
    }
    jb_var_t h = jb_fs_abrir(ruta, jb_new_texto("rb"));
    jb_var_t ba = jb_fs_leer_byte(h);
    jb_var_t bb = jb_fs_leer_byte(h);
    jb_var_t bc = jb_fs_leer_byte(h);
    jb_fs_cerrar(h);
    if (jb_truthy(jb_land(jb_land(jb_eq(ba, jb_new_entero(90)), jb_eq(bb, jb_new_entero(66))), jb_eq(bc, jb_new_entero(49))))) {
        return jb_new_entero(1);
    }
    return jb_new_entero(0);
    return jb_new_nulo();
}

static jb_var_t jbf_pl_bin_estres_escritura(jb_var_t ruta, jb_var_t n) {
    jb_var_t h = jb_fs_abrir(ruta, jb_new_texto("wb"));
    jb_var_t cnt = jb_new_entero(0);
    while (jb_truthy(jb_lt(cnt, n))) {
        jb_fs_escribir_byte(jb_mod(cnt, jb_new_entero(256)), h);
        jb_assign(&cnt, jb_add(cnt, jb_new_entero(1)));
    }
    jb_fs_cerrar(h);
    return n;
    return jb_new_nulo();
}

int main(int argc, char **argv) {
    jb_init();
    jb_set_argv(argc, argv);
    jb_var_t salir = jb_new_entero(0);
    jb_imprimir(jbf_pl_ui_banner(jb_new_entero(0)));
    jb_imprimir(jbf_pl_ui_subtitulo(jb_new_entero(0)));
    jb_imprimir(jbf_pl_ui_opciones(jb_new_entero(0)));
    while (jb_truthy(jb_eq(salir, jb_new_entero(0)))) {
        jb_imprimir_sin_salto(jb_new_texto("> "));
        jb_var_t linea = jb_new_nulo();
        jb_ingresar_texto(&linea);
        jb_var_t op = jb_str_a_entero(linea);
        {
            jb_var_t __sel = op;
            if (jb_truthy(jb_eq(__sel, jb_new_entero(1)))) {
                jb_imprimir(jb_new_texto("--- Ayuda ---"));
                jb_imprimir(jbf_pl_ui_banner(jb_new_entero(0)));
                jb_imprimir(jbf_pl_ui_opciones(jb_new_entero(0)));
            }
            else if (jb_truthy(jb_eq(__sel, jb_new_entero(2)))) {
                jb_imprimir(jb_new_texto("--- JSON demo ---"));
                jb_var_t seed = jbf_pl_json_texto_inicial(jb_new_entero(0));
                jb_imprimir(seed);
                jb_imprimir(jb_new_texto("Tipo raiz JSON (codigo interno VM/AOT):"));
                jb_imprimir(jb_texto_desde_numero(jbf_pl_json_tipar_raiz(seed)));
                jb_imprimir(jb_new_texto("Items en arreglo raiz items (0 si no aplica):"));
                jb_imprimir(jb_texto_desde_numero(jbf_pl_json_conteo_items(seed)));
            }
            else if (jb_truthy(jb_eq(__sel, jb_new_entero(3)))) {
                jb_imprimir(jb_new_texto("--- Bin marcador ---"));
                jb_var_t ruta = jbf_pl_bin_ruta_temp(jb_new_entero(0));
                jb_imprimir(jb_texto_desde_numero(jbf_pl_bin_escribir_marcador(ruta)));
                jb_imprimir(jb_texto_desde_numero(jbf_pl_bin_verificar_marcador(ruta)));
            }
            else if (jb_truthy(jb_eq(__sel, jb_new_entero(4)))) {
                jb_imprimir(jb_new_texto("Escriba N (bytes a escribir en estres, p. ej. 200):"));
                jb_var_t tn = jb_new_nulo();
                jb_ingresar_texto(&tn);
                jb_var_t nn = jb_str_a_entero(tn);
                jb_var_t r2 = jbf_pl_bin_ruta_temp(jb_new_entero(0));
                jb_imprimir(jb_texto_desde_numero(jbf_pl_bin_estres_escritura(r2, nn)));
                jb_imprimir(jb_texto_desde_numero(jb_fs_tamano(r2)));
            }
            else if (jb_truthy(jb_eq(__sel, jb_new_entero(5)))) {
                jb_imprimir(jb_new_texto("Pegue UNA linea JSON valida:"));
                jb_var_t jl = jb_new_nulo();
                jb_ingresar_texto(&jl);
                jb_imprimir(jb_new_texto("Tipo raiz de su JSON (codigo interno):"));
                jb_imprimir(jb_texto_desde_numero(jbf_pl_json_tipar_raiz(jl)));
            }
            else if (jb_truthy(jb_eq(__sel, jb_new_entero(6)))) {
                jb_imprimir(jb_new_texto("--- Limpieza bin demo ---"));
                jb_var_t rz = jbf_pl_bin_ruta_temp(jb_new_entero(0));
                jb_imprimir(jb_texto_desde_numero(jb_fs_borrar(rz)));
            }
            else if (jb_truthy(jb_eq(__sel, jb_new_entero(0)))) {
                jb_imprimir(jb_new_texto("Fin del laboratorio. Hasta pronto."));
                jb_assign(&salir, jb_new_entero(1));
            }
            else {
                jb_imprimir(jb_new_texto("Opcion no reconocida. Pruebe 1-6 o 0."));
            }
        }
    }
    jb_cleanup();
    return 0;
}
