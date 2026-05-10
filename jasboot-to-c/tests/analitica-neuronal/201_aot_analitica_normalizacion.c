#include "jasboot_rt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>

static jb_var_t jbf_AnalizadorBase_inicializar(jb_var_t este, jb_var_t nombre, jb_var_t version);
static jb_var_t jbf_AnalizadorBase_describir(jb_var_t este);
static jb_var_t jbf_AnalizadorBase_validar_lista_numerica(jb_var_t este, jb_var_t datos);
static jb_var_t jbf_AnalizadorBase_establecer_en_lista(jb_var_t este, jb_var_t origen, jb_var_t indice, jb_var_t nuevo_valor);
static jb_var_t jbf_Normalizacion_inicializar(jb_var_t este);
static jb_var_t jbf_Normalizacion_min_max_scaler_fit(jb_var_t este, jb_var_t datos);
static jb_var_t jbf_Normalizacion_min_max_scaler_transform(jb_var_t este, jb_var_t datos, jb_var_t scaler);
static jb_var_t jbf_Normalizacion_standard_scaler_fit(jb_var_t este, jb_var_t datos);
static jb_var_t jbf_Normalizacion_standard_scaler_transform(jb_var_t este, jb_var_t datos, jb_var_t scaler);
static jb_var_t jbf_Normalizacion_robust_scaler_fit(jb_var_t este, jb_var_t datos);
static jb_var_t jbf_Normalizacion_robust_scaler_transform(jb_var_t este, jb_var_t datos, jb_var_t scaler);
static jb_var_t jbf_Normalizacion_normalize_l2(jb_var_t este, jb_var_t datos);
static jb_var_t jbf_Normalizacion_inverse_transform(jb_var_t este, jb_var_t datos_norm, jb_var_t scaler);
static jb_var_t jbf_Normalizacion_calcular_minimo(jb_var_t este, jb_var_t datos);
static jb_var_t jbf_Normalizacion_calcular_maximo(jb_var_t este, jb_var_t datos);
static jb_var_t jbf_Normalizacion_calcular_media(jb_var_t este, jb_var_t datos);
static jb_var_t jbf_Normalizacion_calcular_desviacion_estandar(jb_var_t este, jb_var_t datos);
static jb_var_t jbf_Normalizacion_calcular_mediana(jb_var_t este, jb_var_t datos);
static jb_var_t jbf_Normalizacion_calcular_iqr(jb_var_t este, jb_var_t datos);
static jb_var_t jbf_Normalizacion_calcular_percentil(jb_var_t este, jb_var_t datos_ordenados, jb_var_t p);
static jb_var_t jbf_Normalizacion_calcular_norma_l2(jb_var_t este, jb_var_t datos);
static jb_var_t jbf_Normalizacion_ordenar_lista(jb_var_t este, jb_var_t datos);
static jb_var_t jbf_Normalizacion_inverse_min_max(jb_var_t este, jb_var_t datos_norm, jb_var_t scaler);
static jb_var_t jbf_Normalizacion_inverse_standard(jb_var_t este, jb_var_t datos_norm, jb_var_t scaler);
static jb_var_t jbf_Normalizacion_inverse_robust(jb_var_t este, jb_var_t datos_norm, jb_var_t scaler);
static jb_var_t jbf_valor_absoluto(jb_var_t x);
static jb_var_t jbf_raiz_cuadrada(jb_var_t n);
static jb_var_t jbf_potencia(jb_var_t base_val, jb_var_t exp_val);
static jb_var_t jbf_exponencial(jb_var_t x);
static jb_var_t jbf_valor_a_texto(jb_var_t val);
static jb_var_t jbf_producto_punto(jb_var_t vector_a, jb_var_t vector_b);
static jb_var_t jbf_similitud_coseno(jb_var_t vector_a, jb_var_t vector_b);

static jb_var_t jbf_valor_absoluto(jb_var_t x) {
    if (jb_truthy(jb_lt(x, jb_new_entero(0)))) {
        return jb_sub(jb_new_entero(0), x);
    } else {
        return x;
    }
    return jb_new_nulo();
}

static jb_var_t jbf_raiz_cuadrada(jb_var_t n) {
    if (jb_truthy(jb_lt(n, jb_new_flotante(0)))) {
        return jb_new_flotante(0);
    }
    if (jb_truthy(jb_eq(n, jb_new_flotante(0)))) {
        return jb_new_flotante(0);
    }
    jb_var_t x = n;
    jb_var_t i = jb_new_flotante(0);
    while (jb_truthy(jb_lt(i, jb_new_flotante(20)))) {
        jb_var_t div_res = jb_div(n, x);
        jb_var_t suma_res = jb_add(x, div_res);
        jb_assign(&x, jb_mul(jb_new_flotante(0.5), suma_res));
        jb_assign(&i, jb_add(i, jb_new_flotante(1)));
    }
    return x;
    return jb_new_nulo();
}

static jb_var_t jbf_potencia(jb_var_t base_val, jb_var_t exp_val) {
    if (jb_truthy(jb_eq(exp_val, jb_new_entero(0)))) {
        return jb_new_flotante(1);
    }
    if (jb_truthy(jb_eq(exp_val, jb_new_entero(1)))) {
        return base_val;
    }
    jb_var_t res = jb_new_flotante(1);
    jb_var_t exp_abs = exp_val;
    if (jb_truthy(jb_lt(exp_val, jb_new_entero(0)))) {
        jb_assign(&exp_abs, jb_sub(jb_new_entero(0), exp_val));
    }
    jb_var_t i = jb_new_flotante(0);
    while (jb_truthy(jb_lt(i, exp_abs))) {
        jb_assign(&res, jb_mul(res, base_val));
        jb_assign(&i, jb_add(i, jb_new_flotante(1)));
    }
    if (jb_truthy(jb_lt(exp_val, jb_new_entero(0)))) {
        return jb_div(jb_new_flotante(1), res);
    } else {
        return res;
    }
    return jb_new_nulo();
}

static jb_var_t jbf_exponencial(jb_var_t x) {
    jb_var_t res = jb_new_flotante(1);
    jb_var_t term = jb_new_flotante(1);
    jb_var_t i = jb_new_entero(1);
    while (jb_truthy(jb_lt(i, jb_new_entero(40)))) {
        jb_var_t fi = jb_mul(i, jb_new_flotante(1));
        jb_assign(&term, jb_div(jb_mul(term, x), fi));
        jb_assign(&res, jb_add(res, term));
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    return res;
    return jb_new_nulo();
}

static jb_var_t jbf_valor_a_texto(jb_var_t val) {
    jb_var_t resultado_texto = jb_new_texto("");
    return resultado_texto;
    return jb_new_nulo();
}

static jb_var_t jbf_producto_punto(jb_var_t vector_a, jb_var_t vector_b) {
    jb_var_t n_a = jb_list_len(vector_a);
    jb_var_t n_b = jb_list_len(vector_b);
    if (jb_truthy(jb_lor(jb_ne(n_a, n_b), jb_eq(n_a, jb_new_entero(0))))) {
        return jb_new_flotante(0);
    }
    jb_var_t dot = jb_new_flotante(0);
    jb_var_t i = jb_new_entero(0);
    while (jb_truthy(jb_lt(i, n_a))) {
        jb_var_t val_a = jb_list_get(vector_a, i);
        jb_var_t val_b = jb_list_get(vector_b, i);
        jb_assign(&dot, jb_add(dot, jb_mul(val_a, val_b)));
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    return dot;
    return jb_new_nulo();
}

static jb_var_t jbf_similitud_coseno(jb_var_t vector_a, jb_var_t vector_b) {
    jb_var_t dot = jbf_producto_punto(vector_a, vector_b);
    jb_var_t mag_a = jbf_raiz_cuadrada(jbf_producto_punto(vector_a, vector_a));
    jb_var_t mag_b = jbf_raiz_cuadrada(jbf_producto_punto(vector_b, vector_b));
    jb_var_t den = jb_mul(mag_a, mag_b);
    if (jb_truthy(jb_eq(den, jb_new_flotante(0)))) {
        return jb_new_flotante(0);
    }
    return jb_div(dot, den);
    return jb_new_nulo();
}

static jb_var_t jbf_AnalizadorBase_inicializar(jb_var_t este, jb_var_t nombre, jb_var_t version) {
    jb_put_member_leaf(&este, "nombre_analizador", nombre);
    jb_put_member_leaf(&este, "version_analizador", version);
    return jb_new_nulo();
}

static jb_var_t jbf_AnalizadorBase_describir(jb_var_t este) {
    jb_imprimir(jb_add(jb_add(jb_add(jb_add(jb_new_texto("--- "), jb_member_get(este, "nombre_analizador")), jb_new_texto(" (")), jb_member_get(este, "version_analizador")), jb_new_texto(") ---")));
    return jb_new_nulo();
}

static jb_var_t jbf_AnalizadorBase_validar_lista_numerica(jb_var_t este, jb_var_t datos) {
    if (jb_truthy(jb_lor(jb_eq(datos, jb_new_nulo()), jb_eq(jb_list_len(datos), jb_new_entero(0))))) {
        return jb_new_entero(0);
    }
    jb_var_t i_val = jb_new_entero(0);
    while (jb_truthy(jb_lt(i_val, jb_list_len(datos)))) {
        jb_var_t v_val = jb_list_get(datos, i_val);
        if (jb_truthy(jb_lor(jb_eq(v_val, jb_new_nulo()), jb_eq(v_val, jb_new_texto("indefinido"))))) {
            return jb_new_entero(0);
        }
        jb_assign(&i_val, jb_add(i_val, jb_new_entero(1)));
    }
    return jb_new_entero(1);
    return jb_new_nulo();
}

static jb_var_t jbf_AnalizadorBase_establecer_en_lista(jb_var_t este, jb_var_t origen, jb_var_t indice, jb_var_t nuevo_valor) {
    jb_var_t nueva = jb_new_list();
    jb_var_t n = jb_list_len(origen);
    jb_var_t i = jb_new_entero(0);
    while (jb_truthy(jb_lt(i, n))) {
        if (jb_truthy(jb_eq(i, indice))) {
            jb_list_push(&nueva, nuevo_valor);
        } else {
            jb_list_push(&nueva, jb_list_get(origen, i));
        }
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    return nueva;
    return jb_new_nulo();
}

static jb_var_t jbf_Normalizacion_inicializar(jb_var_t este) {
    jb_put_member_leaf(&este, "nombre_analizador", jb_new_texto("Normalizacion"));
    jb_put_member_leaf(&este, "version_analizador", jb_new_texto("1.0.0"));
    return jb_new_nulo();
}

static jb_var_t jbf_Normalizacion_min_max_scaler_fit(jb_var_t este, jb_var_t datos) {
    if (jb_truthy(jb_not(jbf_AnalizadorBase_validar_lista_numerica(este, datos)))) {
        jb_imprimir(jb_new_texto("[ERROR] Datos inválidos para min_max_scaler_fit"));
        return jb_new_nulo();
    }
    jb_var_t min_val = jbf_Normalizacion_calcular_minimo(este, datos);
    jb_var_t max_val = jbf_Normalizacion_calcular_maximo(este, datos);
    jb_var_t scaler = jb_new_map();
    jb_map_put(&scaler, jb_new_texto("min"), min_val);
    jb_map_put(&scaler, jb_new_texto("max"), max_val);
    jb_map_put(&scaler, jb_new_texto("tipo"), jb_new_texto("min_max"));
    return scaler;
    return jb_new_nulo();
}

static jb_var_t jbf_Normalizacion_min_max_scaler_transform(jb_var_t este, jb_var_t datos, jb_var_t scaler) {
    if (jb_truthy(jb_lor(jb_not(jbf_AnalizadorBase_validar_lista_numerica(este, datos)), jb_eq(scaler, jb_new_nulo())))) {
        jb_imprimir(jb_new_texto("[ERROR] Datos o scaler inválidos para min_max_scaler_transform"));
        return jb_new_nulo();
    }
    jb_var_t min_val = jb_map_get(scaler, jb_new_texto("min"));
    jb_var_t max_val = jb_map_get(scaler, jb_new_texto("max"));
    jb_var_t rango = jb_sub(max_val, min_val);
    if (jb_truthy(jb_eq(rango, jb_new_flotante(0)))) {
        jb_imprimir(jb_new_texto("[WARN] Rango es cero, retornando datos sin cambios"));
        return datos;
    }
    jb_var_t datos_norm = jb_new_list();
    jb_var_t i = jb_new_entero(0);
    jb_var_t n = jb_list_len(datos);
    while (jb_truthy(jb_lt(i, n))) {
        jb_var_t x = jb_list_get(datos, i);
        jb_var_t x_norm = jb_div(jb_sub(x, min_val), rango);
        jb_list_push(&datos_norm, x_norm);
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    return datos_norm;
    return jb_new_nulo();
}

static jb_var_t jbf_Normalizacion_standard_scaler_fit(jb_var_t este, jb_var_t datos) {
    if (jb_truthy(jb_not(jbf_AnalizadorBase_validar_lista_numerica(este, datos)))) {
        jb_imprimir(jb_new_texto("[ERROR] Datos inválidos para standard_scaler_fit"));
        return jb_new_nulo();
    }
    jb_var_t media = jbf_Normalizacion_calcular_media(este, datos);
    jb_var_t desviacion = jbf_Normalizacion_calcular_desviacion_estandar(este, datos);
    jb_var_t scaler = jb_new_map();
    jb_map_put(&scaler, jb_new_texto("media"), media);
    jb_map_put(&scaler, jb_new_texto("desviacion"), desviacion);
    jb_map_put(&scaler, jb_new_texto("tipo"), jb_new_texto("standard"));
    return scaler;
    return jb_new_nulo();
}

static jb_var_t jbf_Normalizacion_standard_scaler_transform(jb_var_t este, jb_var_t datos, jb_var_t scaler) {
    if (jb_truthy(jb_lor(jb_not(jbf_AnalizadorBase_validar_lista_numerica(este, datos)), jb_eq(scaler, jb_new_nulo())))) {
        jb_imprimir(jb_new_texto("[ERROR] Datos o scaler inválidos para standard_scaler_transform"));
        return jb_new_nulo();
    }
    jb_var_t media = jb_map_get(scaler, jb_new_texto("media"));
    jb_var_t desviacion = jb_map_get(scaler, jb_new_texto("desviacion"));
    if (jb_truthy(jb_eq(desviacion, jb_new_flotante(0)))) {
        jb_imprimir(jb_new_texto("[WARN] Desviación estándar es cero, retornando datos sin cambios"));
        return datos;
    }
    jb_var_t datos_norm = jb_new_list();
    jb_var_t i = jb_new_entero(0);
    jb_var_t n = jb_list_len(datos);
    while (jb_truthy(jb_lt(i, n))) {
        jb_var_t x = jb_list_get(datos, i);
        jb_var_t x_norm = jb_div(jb_sub(x, media), desviacion);
        jb_list_push(&datos_norm, x_norm);
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    return datos_norm;
    return jb_new_nulo();
}

static jb_var_t jbf_Normalizacion_robust_scaler_fit(jb_var_t este, jb_var_t datos) {
    if (jb_truthy(jb_not(jbf_AnalizadorBase_validar_lista_numerica(este, datos)))) {
        jb_imprimir(jb_new_texto("[ERROR] Datos inválidos para robust_scaler_fit"));
        return jb_new_nulo();
    }
    jb_var_t mediana = jbf_Normalizacion_calcular_mediana(este, datos);
    jb_var_t iqr = jbf_Normalizacion_calcular_iqr(este, datos);
    jb_var_t scaler = jb_new_map();
    jb_map_put(&scaler, jb_new_texto("mediana"), mediana);
    jb_map_put(&scaler, jb_new_texto("iqr"), iqr);
    jb_map_put(&scaler, jb_new_texto("tipo"), jb_new_texto("robust"));
    return scaler;
    return jb_new_nulo();
}

static jb_var_t jbf_Normalizacion_robust_scaler_transform(jb_var_t este, jb_var_t datos, jb_var_t scaler) {
    if (jb_truthy(jb_lor(jb_not(jbf_AnalizadorBase_validar_lista_numerica(este, datos)), jb_eq(scaler, jb_new_nulo())))) {
        jb_imprimir(jb_new_texto("[ERROR] Datos o scaler inválidos para robust_scaler_transform"));
        return jb_new_nulo();
    }
    jb_var_t mediana = jb_map_get(scaler, jb_new_texto("mediana"));
    jb_var_t iqr = jb_map_get(scaler, jb_new_texto("iqr"));
    if (jb_truthy(jb_eq(iqr, jb_new_flotante(0)))) {
        jb_imprimir(jb_new_texto("[WARN] IQR es cero, retornando datos sin cambios"));
        return datos;
    }
    jb_var_t datos_norm = jb_new_list();
    jb_var_t i = jb_new_entero(0);
    jb_var_t n = jb_list_len(datos);
    while (jb_truthy(jb_lt(i, n))) {
        jb_var_t x = jb_list_get(datos, i);
        jb_var_t x_norm = jb_div(jb_sub(x, mediana), iqr);
        jb_list_push(&datos_norm, x_norm);
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    return datos_norm;
    return jb_new_nulo();
}

static jb_var_t jbf_Normalizacion_normalize_l2(jb_var_t este, jb_var_t datos) {
    if (jb_truthy(jb_not(jbf_AnalizadorBase_validar_lista_numerica(este, datos)))) {
        jb_imprimir(jb_new_texto("[ERROR] Datos inválidos para normalize_l2"));
        return jb_new_nulo();
    }
    jb_var_t norma = jbf_Normalizacion_calcular_norma_l2(este, datos);
    if (jb_truthy(jb_eq(norma, jb_new_flotante(0)))) {
        jb_imprimir(jb_new_texto("[WARN] Norma L2 es cero, retornando datos sin cambios"));
        return datos;
    }
    jb_var_t datos_norm = jb_new_list();
    jb_var_t i = jb_new_entero(0);
    jb_var_t n = jb_list_len(datos);
    while (jb_truthy(jb_lt(i, n))) {
        jb_var_t x = jb_list_get(datos, i);
        jb_var_t x_norm = jb_div(x, norma);
        jb_list_push(&datos_norm, x_norm);
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    return datos_norm;
    return jb_new_nulo();
}

static jb_var_t jbf_Normalizacion_inverse_transform(jb_var_t este, jb_var_t datos_norm, jb_var_t scaler) {
    if (jb_truthy(jb_lor(jb_not(jbf_AnalizadorBase_validar_lista_numerica(este, datos_norm)), jb_eq(scaler, jb_new_nulo())))) {
        jb_imprimir(jb_new_texto("[ERROR] Datos o scaler inválidos para inverse_transform"));
        return jb_new_nulo();
    }
    jb_var_t tipo = jb_map_get(scaler, jb_new_texto("tipo"));
    jb_var_t datos_orig = jb_new_nulo();
    if (jb_truthy(jb_eq(tipo, jb_new_texto("min_max")))) {
        jb_assign(&datos_orig, jbf_Normalizacion_inverse_min_max(este, datos_norm, scaler));
    } else {
        if (jb_truthy(jb_eq(tipo, jb_new_texto("standard")))) {
            jb_assign(&datos_orig, jbf_Normalizacion_inverse_standard(este, datos_norm, scaler));
        } else {
            if (jb_truthy(jb_eq(tipo, jb_new_texto("robust")))) {
                jb_assign(&datos_orig, jbf_Normalizacion_inverse_robust(este, datos_norm, scaler));
            } else {
                jb_imprimir(jb_add(jb_new_texto("[ERROR] Tipo de scaler desconocido: "), tipo));
                return jb_new_nulo();
            }
        }
    }
    return datos_orig;
    return jb_new_nulo();
}

static jb_var_t jbf_Normalizacion_calcular_minimo(jb_var_t este, jb_var_t datos) {
    jb_var_t min_val = jb_list_get(datos, jb_new_entero(0));
    jb_var_t i = jb_new_entero(1);
    jb_var_t n = jb_list_len(datos);
    while (jb_truthy(jb_lt(i, n))) {
        jb_var_t x = jb_list_get(datos, i);
        if (jb_truthy(jb_lt(x, min_val))) {
            jb_assign(&min_val, x);
        }
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    return min_val;
    return jb_new_nulo();
}

static jb_var_t jbf_Normalizacion_calcular_maximo(jb_var_t este, jb_var_t datos) {
    jb_var_t max_val = jb_list_get(datos, jb_new_entero(0));
    jb_var_t i = jb_new_entero(1);
    jb_var_t n = jb_list_len(datos);
    while (jb_truthy(jb_lt(i, n))) {
        jb_var_t x = jb_list_get(datos, i);
        if (jb_truthy(jb_gt(x, max_val))) {
            jb_assign(&max_val, x);
        }
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    return max_val;
    return jb_new_nulo();
}

static jb_var_t jbf_Normalizacion_calcular_media(jb_var_t este, jb_var_t datos) {
    jb_var_t suma = jb_new_flotante(0);
    jb_var_t i = jb_new_entero(0);
    jb_var_t n = jb_list_len(datos);
    while (jb_truthy(jb_lt(i, n))) {
        jb_var_t x = jb_list_get(datos, i);
        jb_assign(&suma, jb_add(suma, x));
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    return jb_div(suma, n);
    return jb_new_nulo();
}

static jb_var_t jbf_Normalizacion_calcular_desviacion_estandar(jb_var_t este, jb_var_t datos) {
    jb_var_t media = jbf_Normalizacion_calcular_media(este, datos);
    jb_var_t suma_cuadrados = jb_new_flotante(0);
    jb_var_t i = jb_new_entero(0);
    jb_var_t n = jb_list_len(datos);
    while (jb_truthy(jb_lt(i, n))) {
        jb_var_t x = jb_list_get(datos, i);
        jb_var_t diff = jb_sub(x, media);
        jb_assign(&suma_cuadrados, jb_add(suma_cuadrados, jb_mul(diff, diff)));
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    jb_var_t varianza = jb_div(suma_cuadrados, n);
    return jbf_raiz_cuadrada(varianza);
    return jb_new_nulo();
}

static jb_var_t jbf_Normalizacion_calcular_mediana(jb_var_t este, jb_var_t datos) {
    jb_var_t datos_ordenados = jbf_Normalizacion_ordenar_lista(este, datos);
    jb_var_t n = jb_list_len(datos_ordenados);
    jb_var_t mitad = jb_div(n, jb_new_entero(2));
    if (jb_truthy(jb_eq(jb_mod(n, jb_new_entero(2)), jb_new_entero(0)))) {
        jb_var_t val1 = jb_list_get(datos_ordenados, jb_sub(mitad, jb_new_entero(1)));
        jb_var_t val2 = jb_list_get(datos_ordenados, mitad);
        return jb_div(jb_add(val1, val2), jb_new_flotante(2));
    } else {
        return jb_list_get(datos_ordenados, mitad);
    }
    return jb_new_nulo();
}

static jb_var_t jbf_Normalizacion_calcular_iqr(jb_var_t este, jb_var_t datos) {
    jb_var_t datos_ordenados = jbf_Normalizacion_ordenar_lista(este, datos);
    jb_var_t q1 = jbf_Normalizacion_calcular_percentil(este, datos_ordenados, jb_new_flotante(25));
    jb_var_t q3 = jbf_Normalizacion_calcular_percentil(este, datos_ordenados, jb_new_flotante(75));
    return jb_sub(q3, q1);
    return jb_new_nulo();
}

static jb_var_t jbf_Normalizacion_calcular_percentil(jb_var_t este, jb_var_t datos_ordenados, jb_var_t p) {
    jb_var_t n = jb_list_len(datos_ordenados);
    jb_var_t indice = jb_mul(jb_div(p, jb_new_flotante(100)), jb_sub(n, jb_new_entero(1)));
    jb_var_t indice_bajo = indice;
    jb_var_t fraccion = jb_sub(indice, indice_bajo);
    if (jb_truthy(jb_ge(indice_bajo, jb_sub(n, jb_new_entero(1))))) {
        return jb_list_get(datos_ordenados, jb_sub(n, jb_new_entero(1)));
    }
    jb_var_t val_bajo = jb_list_get(datos_ordenados, indice_bajo);
    jb_var_t val_alto = jb_list_get(datos_ordenados, jb_add(indice_bajo, jb_new_entero(1)));
    return jb_add(val_bajo, jb_mul(fraccion, jb_sub(val_alto, val_bajo)));
    return jb_new_nulo();
}

static jb_var_t jbf_Normalizacion_calcular_norma_l2(jb_var_t este, jb_var_t datos) {
    jb_var_t suma_cuadrados = jb_new_flotante(0);
    jb_var_t i = jb_new_entero(0);
    jb_var_t n = jb_list_len(datos);
    while (jb_truthy(jb_lt(i, n))) {
        jb_var_t x = jb_list_get(datos, i);
        jb_assign(&suma_cuadrados, jb_add(suma_cuadrados, jb_mul(x, x)));
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    return jbf_raiz_cuadrada(suma_cuadrados);
    return jb_new_nulo();
}

static jb_var_t jbf_Normalizacion_ordenar_lista(jb_var_t este, jb_var_t datos) {
    jb_var_t copia = jb_new_list();
    jb_var_t i = jb_new_entero(0);
    jb_var_t n = jb_list_len(datos);
    while (jb_truthy(jb_lt(i, n))) {
        jb_list_push(&copia, jb_list_get(datos, i));
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    jb_var_t j = jb_new_entero(0);
    while (jb_truthy(jb_lt(j, jb_sub(n, jb_new_entero(1))))) {
        jb_var_t k = jb_new_entero(0);
        while (jb_truthy(jb_lt(k, jb_sub(jb_sub(n, j), jb_new_entero(1))))) {
            jb_var_t val1 = jb_list_get(copia, k);
            jb_var_t val2 = jb_list_get(copia, jb_add(k, jb_new_entero(1)));
            if (jb_truthy(jb_gt(val1, val2))) {
                jb_assign(&copia, jbf_AnalizadorBase_establecer_en_lista(este, copia, k, val2));
                jb_assign(&copia, jbf_AnalizadorBase_establecer_en_lista(este, copia, jb_add(k, jb_new_entero(1)), val1));
            }
            jb_assign(&k, jb_add(k, jb_new_entero(1)));
        }
        jb_assign(&j, jb_add(j, jb_new_entero(1)));
    }
    return copia;
    return jb_new_nulo();
}

static jb_var_t jbf_Normalizacion_inverse_min_max(jb_var_t este, jb_var_t datos_norm, jb_var_t scaler) {
    jb_var_t min_val = jb_map_get(scaler, jb_new_texto("min"));
    jb_var_t max_val = jb_map_get(scaler, jb_new_texto("max"));
    jb_var_t rango = jb_sub(max_val, min_val);
    jb_var_t datos_orig = jb_new_list();
    jb_var_t i = jb_new_entero(0);
    jb_var_t n = jb_list_len(datos_norm);
    while (jb_truthy(jb_lt(i, n))) {
        jb_var_t x_norm = jb_list_get(datos_norm, i);
        jb_var_t x_orig = jb_add(jb_mul(x_norm, rango), min_val);
        jb_list_push(&datos_orig, x_orig);
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    return datos_orig;
    return jb_new_nulo();
}

static jb_var_t jbf_Normalizacion_inverse_standard(jb_var_t este, jb_var_t datos_norm, jb_var_t scaler) {
    jb_var_t media = jb_map_get(scaler, jb_new_texto("media"));
    jb_var_t desviacion = jb_map_get(scaler, jb_new_texto("desviacion"));
    jb_var_t datos_orig = jb_new_list();
    jb_var_t i = jb_new_entero(0);
    jb_var_t n = jb_list_len(datos_norm);
    while (jb_truthy(jb_lt(i, n))) {
        jb_var_t x_norm = jb_list_get(datos_norm, i);
        jb_var_t x_orig = jb_add(jb_mul(x_norm, desviacion), media);
        jb_list_push(&datos_orig, x_orig);
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    return datos_orig;
    return jb_new_nulo();
}

static jb_var_t jbf_Normalizacion_inverse_robust(jb_var_t este, jb_var_t datos_norm, jb_var_t scaler) {
    jb_var_t mediana = jb_map_get(scaler, jb_new_texto("mediana"));
    jb_var_t iqr = jb_map_get(scaler, jb_new_texto("iqr"));
    jb_var_t datos_orig = jb_new_list();
    jb_var_t i = jb_new_entero(0);
    jb_var_t n = jb_list_len(datos_norm);
    while (jb_truthy(jb_lt(i, n))) {
        jb_var_t x_norm = jb_list_get(datos_norm, i);
        jb_var_t x_orig = jb_add(jb_mul(x_norm, iqr), mediana);
        jb_list_push(&datos_orig, x_orig);
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    return datos_orig;
    return jb_new_nulo();
}

int main(int argc, char **argv) {
    jb_init();
    jb_set_argv(argc, argv);
    jb_var_t n = jb_new_map();
    jb_assign(&n, ({ jb_var_t __co = jb_new_map(); jbf_Normalizacion_inicializar(__co); __co; }));
    jbf_Normalizacion_inicializar(n);
    jb_var_t d = jb_new_nulo();
    jb_assign(&d, jb_new_list());
    jb_list_push(&d, jb_new_flotante(1));
    jb_list_push(&d, jb_new_flotante(3));
    jb_var_t sc = jb_new_nulo();
    jb_assign(&sc, jbf_Normalizacion_min_max_scaler_fit(n, d));
    jb_var_t t = jb_new_nulo();
    jb_assign(&t, jbf_Normalizacion_min_max_scaler_transform(n, d, sc));
    jb_imprimir(jb_list_get(t, jb_new_entero(0)));
    jb_imprimir(jb_list_get(t, jb_new_entero(1)));
    jb_cleanup();
    return 0;
}
