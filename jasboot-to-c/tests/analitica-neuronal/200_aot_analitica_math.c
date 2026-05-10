#include "jasboot_rt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>

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

int main(int argc, char **argv) {
    jb_init();
    jb_set_argv(argc, argv);
    jb_imprimir(jbf_raiz_cuadrada(jb_new_flotante(4)));
    jb_imprimir(jbf_potencia(jb_new_flotante(2), jb_new_flotante(3)));
    jb_imprimir(jbf_exponencial(jb_new_flotante(0)));
    jb_cleanup();
    return 0;
}
