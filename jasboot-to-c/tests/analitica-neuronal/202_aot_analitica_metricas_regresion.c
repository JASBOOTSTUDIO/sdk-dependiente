#include "jasboot_rt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>

static jb_var_t jbf_AnalizadorBase_inicializar(jb_var_t este, jb_var_t nombre, jb_var_t version);
static jb_var_t jbf_AnalizadorBase_describir(jb_var_t este);
static jb_var_t jbf_AnalizadorBase_validar_lista_numerica(jb_var_t este, jb_var_t datos);
static jb_var_t jbf_AnalizadorBase_establecer_en_lista(jb_var_t este, jb_var_t origen, jb_var_t indice, jb_var_t nuevo_valor);
static jb_var_t jbf_MetricasRegresion_inicializar(jb_var_t este);
static jb_var_t jbf_MetricasRegresion_validar_listas_regresion(jb_var_t este, jb_var_t y_real, jb_var_t y_pred);
static jb_var_t jbf_MetricasRegresion_calcular_mae(jb_var_t este, jb_var_t y_real, jb_var_t y_pred);
static jb_var_t jbf_MetricasRegresion_calcular_mse(jb_var_t este, jb_var_t y_real, jb_var_t y_pred);
static jb_var_t jbf_MetricasRegresion_calcular_rmse(jb_var_t este, jb_var_t y_real, jb_var_t y_pred);
static jb_var_t jbf_MetricasRegresion_calcular_mape(jb_var_t este, jb_var_t y_real, jb_var_t y_pred);
static jb_var_t jbf_MetricasRegresion_calcular_max_error(jb_var_t este, jb_var_t y_real, jb_var_t y_pred);
static jb_var_t jbf_MetricasRegresion_calcular_media(jb_var_t este, jb_var_t datos);
static jb_var_t jbf_MetricasRegresion_calcular_r2_score(jb_var_t este, jb_var_t y_real, jb_var_t y_pred);
static jb_var_t jbf_MetricasRegresion_calcular_explained_variance(jb_var_t este, jb_var_t y_real, jb_var_t y_pred);
static jb_var_t jbf_MetricasRegresion_informe_metricas_regresion(jb_var_t este, jb_var_t y_real, jb_var_t y_pred);
static jb_var_t jbf_MetricasRegresion_comparar_modelos(jb_var_t este, jb_var_t y_real, jb_var_t y_pred1, jb_var_t y_pred2, jb_var_t nombre1, jb_var_t nombre2);
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

static jb_var_t jbf_MetricasRegresion_inicializar(jb_var_t este) {
    jbf_AnalizadorBase_inicializar(este, jb_new_texto("Métricas de Regresión"), jb_new_texto("V1.0"));
    return jb_new_nulo();
}

static jb_var_t jbf_MetricasRegresion_validar_listas_regresion(jb_var_t este, jb_var_t y_real, jb_var_t y_pred) {
    if (jb_truthy(jb_eq(y_real, jb_new_nulo()))) {
        jb_imprimir(jb_new_texto("[ERROR] Lista y_real es nula"));
        return jb_new_entero(0);
    }
    if (jb_truthy(jb_eq(y_pred, jb_new_nulo()))) {
        jb_imprimir(jb_new_texto("[ERROR] Lista y_pred es nula"));
        return jb_new_entero(0);
    }
    jb_var_t n_real = jb_list_len(y_real);
    jb_var_t n_pred = jb_list_len(y_pred);
    if (jb_truthy(jb_eq(n_real, jb_new_entero(0)))) {
        jb_imprimir(jb_new_texto("[ERROR] Lista y_real está vacía"));
        return jb_new_entero(0);
    }
    if (jb_truthy(jb_eq(n_pred, jb_new_entero(0)))) {
        jb_imprimir(jb_new_texto("[ERROR] Lista y_pred está vacía"));
        return jb_new_entero(0);
    }
    if (jb_truthy(jb_ne(n_real, n_pred))) {
        jb_imprimir(jb_add(jb_add(jb_add(jb_new_texto("[ERROR] Las listas tienen tamaños diferentes: "), jb_entero_a_texto(n_real)), jb_new_texto(" vs ")), jb_entero_a_texto(n_pred)));
        return jb_new_entero(0);
    }
    if (jb_truthy(jb_not(jbf_AnalizadorBase_validar_lista_numerica(este, y_real)))) {
        jb_imprimir(jb_new_texto("[ERROR] Lista y_real contiene valores no numéricos"));
        return jb_new_entero(0);
    }
    if (jb_truthy(jb_not(jbf_AnalizadorBase_validar_lista_numerica(este, y_pred)))) {
        jb_imprimir(jb_new_texto("[ERROR] Lista y_pred contiene valores no numéricos"));
        return jb_new_entero(0);
    }
    return jb_new_entero(1);
    return jb_new_nulo();
}

static jb_var_t jbf_MetricasRegresion_calcular_mae(jb_var_t este, jb_var_t y_real, jb_var_t y_pred) {
    if (jb_truthy(jb_not(jbf_MetricasRegresion_validar_listas_regresion(este, y_real, y_pred)))) {
        return jb_sub(jb_new_entero(0), jb_new_flotante(1));
    }
    jb_var_t n = jb_list_len(y_real);
    jb_var_t suma_errores = jb_new_flotante(0);
    jb_var_t i = jb_new_entero(0);
    while (jb_truthy(jb_lt(i, n))) {
        jb_var_t real = jb_list_get(y_real, i);
        jb_var_t pred = jb_list_get(y_pred, i);
        jb_var_t error = jb_sub(real, pred);
        jb_assign(&suma_errores, jb_add(suma_errores, jbf_valor_absoluto(error)));
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    return jb_div(suma_errores, n);
    return jb_new_nulo();
}

static jb_var_t jbf_MetricasRegresion_calcular_mse(jb_var_t este, jb_var_t y_real, jb_var_t y_pred) {
    if (jb_truthy(jb_not(jbf_MetricasRegresion_validar_listas_regresion(este, y_real, y_pred)))) {
        return jb_sub(jb_new_entero(0), jb_new_flotante(1));
    }
    jb_var_t n = jb_list_len(y_real);
    jb_var_t suma_errores_cuadrados = jb_new_flotante(0);
    jb_var_t i = jb_new_entero(0);
    while (jb_truthy(jb_lt(i, n))) {
        jb_var_t real = jb_list_get(y_real, i);
        jb_var_t pred = jb_list_get(y_pred, i);
        jb_var_t error = jb_sub(real, pred);
        jb_assign(&suma_errores_cuadrados, jb_add(suma_errores_cuadrados, jb_mul(error, error)));
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    return jb_div(suma_errores_cuadrados, n);
    return jb_new_nulo();
}

static jb_var_t jbf_MetricasRegresion_calcular_rmse(jb_var_t este, jb_var_t y_real, jb_var_t y_pred) {
    jb_var_t mse = jbf_MetricasRegresion_calcular_mse(este, y_real, y_pred);
    if (jb_truthy(jb_lt(mse, jb_new_flotante(0)))) {
        return jb_sub(jb_new_entero(0), jb_new_flotante(1));
    }
    return jbf_raiz_cuadrada(mse);
    return jb_new_nulo();
}

static jb_var_t jbf_MetricasRegresion_calcular_mape(jb_var_t este, jb_var_t y_real, jb_var_t y_pred) {
    if (jb_truthy(jb_not(jbf_MetricasRegresion_validar_listas_regresion(este, y_real, y_pred)))) {
        return jb_sub(jb_new_entero(0), jb_new_flotante(1));
    }
    jb_var_t n = jb_list_len(y_real);
    jb_var_t suma_porcentajes = jb_new_flotante(0);
    jb_var_t i = jb_new_entero(0);
    jb_var_t valores_validos = jb_new_entero(0);
    while (jb_truthy(jb_lt(i, n))) {
        jb_var_t real = jb_list_get(y_real, i);
        jb_var_t pred = jb_list_get(y_pred, i);
        if (jb_truthy(jb_gt(jbf_valor_absoluto(real), jb_new_flotante(0.0001)))) {
            jb_var_t error = jb_sub(real, pred);
            jb_var_t porcentaje = jbf_valor_absoluto(jb_div(error, real));
            jb_assign(&suma_porcentajes, jb_add(suma_porcentajes, porcentaje));
            jb_assign(&valores_validos, jb_add(valores_validos, jb_new_entero(1)));
        }
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    if (jb_truthy(jb_eq(valores_validos, jb_new_entero(0)))) {
        jb_imprimir(jb_new_texto("[ADVERTENCIA] No hay valores válidos para calcular MAPE (todos los valores reales son cero)"));
        return jb_new_flotante(0);
    }
    return jb_mul(jb_div(suma_porcentajes, valores_validos), jb_new_flotante(100));
    return jb_new_nulo();
}

static jb_var_t jbf_MetricasRegresion_calcular_max_error(jb_var_t este, jb_var_t y_real, jb_var_t y_pred) {
    if (jb_truthy(jb_not(jbf_MetricasRegresion_validar_listas_regresion(este, y_real, y_pred)))) {
        return jb_sub(jb_new_entero(0), jb_new_flotante(1));
    }
    jb_var_t n = jb_list_len(y_real);
    jb_var_t max_err = jb_new_flotante(0);
    jb_var_t i = jb_new_entero(0);
    while (jb_truthy(jb_lt(i, n))) {
        jb_var_t real = jb_list_get(y_real, i);
        jb_var_t pred = jb_list_get(y_pred, i);
        jb_var_t error = jbf_valor_absoluto(jb_sub(real, pred));
        if (jb_truthy(jb_gt(error, max_err))) {
            jb_assign(&max_err, error);
        }
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    return max_err;
    return jb_new_nulo();
}

static jb_var_t jbf_MetricasRegresion_calcular_media(jb_var_t este, jb_var_t datos) {
    jb_var_t n = jb_list_len(datos);
    if (jb_truthy(jb_eq(n, jb_new_entero(0)))) {
        return jb_new_flotante(0);
    }
    jb_var_t suma = jb_new_flotante(0);
    jb_var_t i = jb_new_entero(0);
    while (jb_truthy(jb_lt(i, n))) {
        jb_assign(&suma, jb_add(suma, jb_list_get(datos, i)));
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    return jb_div(suma, n);
    return jb_new_nulo();
}

static jb_var_t jbf_MetricasRegresion_calcular_r2_score(jb_var_t este, jb_var_t y_real, jb_var_t y_pred) {
    if (jb_truthy(jb_not(jbf_MetricasRegresion_validar_listas_regresion(este, y_real, y_pred)))) {
        return jb_sub(jb_new_entero(0), jb_new_flotante(999));
    }
    jb_var_t n = jb_list_len(y_real);
    jb_var_t media_real = jbf_MetricasRegresion_calcular_media(este, y_real);
    jb_var_t ss_res = jb_new_flotante(0);
    jb_var_t i = jb_new_entero(0);
    while (jb_truthy(jb_lt(i, n))) {
        jb_var_t real = jb_list_get(y_real, i);
        jb_var_t pred = jb_list_get(y_pred, i);
        jb_var_t residuo = jb_sub(real, pred);
        jb_assign(&ss_res, jb_add(ss_res, jb_mul(residuo, residuo)));
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    jb_var_t ss_tot = jb_new_flotante(0);
    jb_assign(&i, jb_new_entero(0));
    while (jb_truthy(jb_lt(i, n))) {
        jb_var_t real = jb_list_get(y_real, i);
        jb_var_t diferencia = jb_sub(real, media_real);
        jb_assign(&ss_tot, jb_add(ss_tot, jb_mul(diferencia, diferencia)));
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    if (jb_truthy(jb_eq(ss_tot, jb_new_flotante(0)))) {
        jb_imprimir(jb_new_texto("[ADVERTENCIA] SS_tot es cero, todos los valores reales son iguales"));
        return jb_new_flotante(1);
    }
    return jb_sub(jb_new_flotante(1), jb_div(ss_res, ss_tot));
    return jb_new_nulo();
}

static jb_var_t jbf_MetricasRegresion_calcular_explained_variance(jb_var_t este, jb_var_t y_real, jb_var_t y_pred) {
    if (jb_truthy(jb_not(jbf_MetricasRegresion_validar_listas_regresion(este, y_real, y_pred)))) {
        return jb_sub(jb_new_entero(0), jb_new_flotante(999));
    }
    jb_var_t n = jb_list_len(y_real);
    jb_var_t residuos = jb_new_list();
    jb_var_t i = jb_new_entero(0);
    while (jb_truthy(jb_lt(i, n))) {
        jb_var_t real = jb_list_get(y_real, i);
        jb_var_t pred = jb_list_get(y_pred, i);
        jb_list_push(&residuos, jb_sub(real, pred));
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    jb_var_t media_real = jbf_MetricasRegresion_calcular_media(este, y_real);
    jb_var_t var_real = jb_new_flotante(0);
    jb_assign(&i, jb_new_entero(0));
    while (jb_truthy(jb_lt(i, n))) {
        jb_var_t real = jb_list_get(y_real, i);
        jb_var_t diff = jb_sub(real, media_real);
        jb_assign(&var_real, jb_add(var_real, jb_mul(diff, diff)));
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    jb_assign(&var_real, jb_div(var_real, n));
    jb_var_t media_residuos = jbf_MetricasRegresion_calcular_media(este, residuos);
    jb_var_t var_residuos = jb_new_flotante(0);
    jb_assign(&i, jb_new_entero(0));
    while (jb_truthy(jb_lt(i, n))) {
        jb_var_t residuo = jb_list_get(residuos, i);
        jb_var_t diff = jb_sub(residuo, media_residuos);
        jb_assign(&var_residuos, jb_add(var_residuos, jb_mul(diff, diff)));
        jb_assign(&i, jb_add(i, jb_new_entero(1)));
    }
    jb_assign(&var_residuos, jb_div(var_residuos, n));
    if (jb_truthy(jb_eq(var_real, jb_new_flotante(0)))) {
        jb_imprimir(jb_new_texto("[ADVERTENCIA] Varianza de y_real es cero"));
        return jb_new_flotante(1);
    }
    return jb_sub(jb_new_flotante(1), jb_div(var_residuos, var_real));
    return jb_new_nulo();
}

static jb_var_t jbf_MetricasRegresion_informe_metricas_regresion(jb_var_t este, jb_var_t y_real, jb_var_t y_pred) {
    jb_var_t informe = jb_new_map();
    if (jb_truthy(jb_not(jbf_MetricasRegresion_validar_listas_regresion(este, y_real, y_pred)))) {
        jb_map_put(&informe, jb_new_texto("error"), jb_new_texto("Listas inválidas"));
        return informe;
    }
    jb_var_t mae = jbf_MetricasRegresion_calcular_mae(este, y_real, y_pred);
    jb_var_t mse = jbf_MetricasRegresion_calcular_mse(este, y_real, y_pred);
    jb_var_t rmse = jbf_MetricasRegresion_calcular_rmse(este, y_real, y_pred);
    jb_var_t mape = jbf_MetricasRegresion_calcular_mape(este, y_real, y_pred);
    jb_var_t r2 = jbf_MetricasRegresion_calcular_r2_score(este, y_real, y_pred);
    jb_var_t explained_var = jbf_MetricasRegresion_calcular_explained_variance(este, y_real, y_pred);
    jb_var_t max_err = jbf_MetricasRegresion_calcular_max_error(este, y_real, y_pred);
    jb_map_put(&informe, jb_new_texto("mae"), mae);
    jb_map_put(&informe, jb_new_texto("mse"), mse);
    jb_map_put(&informe, jb_new_texto("rmse"), rmse);
    jb_map_put(&informe, jb_new_texto("mape"), mape);
    jb_map_put(&informe, jb_new_texto("r2_score"), r2);
    jb_map_put(&informe, jb_new_texto("explained_variance"), explained_var);
    jb_map_put(&informe, jb_new_texto("max_error"), max_err);
    jb_map_put(&informe, jb_new_texto("n_muestras"), jb_list_len(y_real));
    jb_imprimir(jb_new_texto("=========================================="));
    jb_imprimir(jb_new_texto("INFORME DE MÉTRICAS DE REGRESIÓN"));
    jb_imprimir(jb_new_texto("=========================================="));
    jb_imprimir(jb_add(jb_new_texto("Número de muestras: "), jb_entero_a_texto(jb_list_len(y_real))));
    jb_imprimir(jb_new_texto("------------------------------------------"));
    jb_imprimir(jb_new_texto("Métricas de Error:"));
    jb_imprimir(jb_add(jb_new_texto("  MAE (Error Absoluto Medio):      "), jb_entero_a_texto(mae)));
    jb_imprimir(jb_add(jb_new_texto("  MSE (Error Cuadrático Medio):    "), jb_entero_a_texto(mse)));
    jb_imprimir(jb_add(jb_new_texto("  RMSE (Raíz Error Cuadrático):    "), jb_entero_a_texto(rmse)));
    jb_imprimir(jb_add(jb_add(jb_new_texto("  MAPE (Error Porcentual):         "), jb_entero_a_texto(mape)), jb_new_texto("%")));
    jb_imprimir(jb_add(jb_new_texto("  Error Máximo:                    "), jb_entero_a_texto(max_err)));
    jb_imprimir(jb_new_texto("------------------------------------------"));
    jb_imprimir(jb_new_texto("Métricas de Bondad de Ajuste:"));
    jb_imprimir(jb_add(jb_new_texto("  R² Score:                        "), jb_entero_a_texto(r2)));
    jb_imprimir(jb_add(jb_new_texto("  Varianza Explicada:              "), jb_entero_a_texto(explained_var)));
    jb_imprimir(jb_new_texto("=========================================="));
    return informe;
    return jb_new_nulo();
}

static jb_var_t jbf_MetricasRegresion_comparar_modelos(jb_var_t este, jb_var_t y_real, jb_var_t y_pred1, jb_var_t y_pred2, jb_var_t nombre1, jb_var_t nombre2) {
    jb_var_t comparacion = jb_new_map();
    if (jb_truthy(jb_not(jbf_MetricasRegresion_validar_listas_regresion(este, y_real, y_pred1)))) {
        jb_map_put(&comparacion, jb_new_texto("error"), jb_new_texto("Predicciones del modelo 1 inválidas"));
        return comparacion;
    }
    if (jb_truthy(jb_not(jbf_MetricasRegresion_validar_listas_regresion(este, y_real, y_pred2)))) {
        jb_map_put(&comparacion, jb_new_texto("error"), jb_new_texto("Predicciones del modelo 2 inválidas"));
        return comparacion;
    }
    jb_var_t mae1 = jbf_MetricasRegresion_calcular_mae(este, y_real, y_pred1);
    jb_var_t mae2 = jbf_MetricasRegresion_calcular_mae(este, y_real, y_pred2);
    jb_var_t rmse1 = jbf_MetricasRegresion_calcular_rmse(este, y_real, y_pred1);
    jb_var_t rmse2 = jbf_MetricasRegresion_calcular_rmse(este, y_real, y_pred2);
    jb_var_t r2_1 = jbf_MetricasRegresion_calcular_r2_score(este, y_real, y_pred1);
    jb_var_t r2_2 = jbf_MetricasRegresion_calcular_r2_score(este, y_real, y_pred2);
    jb_map_put(&comparacion, jb_new_texto("modelo1_nombre"), nombre1);
    jb_map_put(&comparacion, jb_new_texto("modelo1_mae"), mae1);
    jb_map_put(&comparacion, jb_new_texto("modelo1_rmse"), rmse1);
    jb_map_put(&comparacion, jb_new_texto("modelo1_r2"), r2_1);
    jb_map_put(&comparacion, jb_new_texto("modelo2_nombre"), nombre2);
    jb_map_put(&comparacion, jb_new_texto("modelo2_mae"), mae2);
    jb_map_put(&comparacion, jb_new_texto("modelo2_rmse"), rmse2);
    jb_map_put(&comparacion, jb_new_texto("modelo2_r2"), r2_2);
    jb_var_t mejor = jb_new_texto("");
    if (jb_truthy(jb_gt(r2_1, r2_2))) {
        jb_assign(&mejor, nombre1);
    } else {
        if (jb_truthy(jb_gt(r2_2, r2_1))) {
            jb_assign(&mejor, nombre2);
        } else {
            jb_assign(&mejor, jb_new_texto("Empate"));
        }
    }
    jb_map_put(&comparacion, jb_new_texto("mejor_modelo"), mejor);
    jb_imprimir(jb_new_texto("=========================================="));
    jb_imprimir(jb_new_texto("COMPARACIÓN DE MODELOS DE REGRESIÓN"));
    jb_imprimir(jb_new_texto("=========================================="));
    jb_imprimir(jb_add(jb_new_texto("Modelo 1: "), nombre1));
    jb_imprimir(jb_add(jb_new_texto("  MAE:   "), jb_entero_a_texto(mae1)));
    jb_imprimir(jb_add(jb_new_texto("  RMSE:  "), jb_entero_a_texto(rmse1)));
    jb_imprimir(jb_add(jb_new_texto("  R²:    "), jb_entero_a_texto(r2_1)));
    jb_imprimir(jb_new_texto("------------------------------------------"));
    jb_imprimir(jb_add(jb_new_texto("Modelo 2: "), nombre2));
    jb_imprimir(jb_add(jb_new_texto("  MAE:   "), jb_entero_a_texto(mae2)));
    jb_imprimir(jb_add(jb_new_texto("  RMSE:  "), jb_entero_a_texto(rmse2)));
    jb_imprimir(jb_add(jb_new_texto("  R²:    "), jb_entero_a_texto(r2_2)));
    jb_imprimir(jb_new_texto("=========================================="));
    jb_imprimir(jb_add(jb_new_texto("Mejor modelo (por R²): "), mejor));
    jb_imprimir(jb_new_texto("=========================================="));
    return comparacion;
    return jb_new_nulo();
}

int main(int argc, char **argv) {
    jb_init();
    jb_set_argv(argc, argv);
    jb_var_t m = jb_new_map();
    jb_assign(&m, ({ jb_var_t __co = jb_new_map(); jbf_MetricasRegresion_inicializar(__co); __co; }));
    jb_var_t y1 = jb_new_nulo();
    jb_assign(&y1, jb_new_list());
    jb_list_push(&y1, jb_new_flotante(1));
    jb_list_push(&y1, jb_new_flotante(2));
    jb_var_t y2 = jb_new_nulo();
    jb_assign(&y2, jb_new_list());
    jb_list_push(&y2, jb_new_flotante(1));
    jb_list_push(&y2, jb_new_flotante(2));
    jb_imprimir(jbf_MetricasRegresion_calcular_mae(m, y1, y2));
    jb_cleanup();
    return 0;
}
