#include "jasboot_rt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>


int main(int argc, char **argv) {
    jb_init();
    jb_set_argv(argc, argv);
    jb_var_t pin_guardado = jb_new_texto("1234");
    jb_var_t saldo = jb_new_flotante(1000);
    jb_var_t salir = jb_new_entero(0);
    jb_var_t intentos_pin = jb_new_entero(0);
    jb_var_t autorizado = jb_new_entero(0);
    jb_var_t historial = jb_new_list();
    jb_var_t promo = jb_new_map();
    jb_map_put(&promo, jb_new_texto("msg"), jb_new_texto("Bienvenido al demo AOT"));
    jb_var_t demo_fl = jb_new_flotante(1.25);
    jb_var_t ts_demo = jb_new_entero(1700000000);
    jb_var_t cien_arc = jb_new_flotante(100);
    jb_imprimir(jb_new_texto("--- Cajero automático (Jasboot → C AOT) ---"));
    jb_imprimir(jb_map_get(promo, jb_new_texto("msg")));
    jb_imprimir(jb_new_texto("Ingrese su PIN (4 dígitos)."));
    while (jb_truthy(jb_land(jb_lt(intentos_pin, jb_new_entero(3)), jb_eq(autorizado, jb_new_entero(0))))) {
        jb_var_t pin_ing = jb_leer_entrada();
        if (jb_truthy(jb_eq(pin_ing, pin_guardado))) {
            jb_assign(&autorizado, jb_new_entero(1));
        } else {
            jb_assign(&intentos_pin, jb_add(intentos_pin, jb_new_entero(1)));
            jb_imprimir(jb_concat(jb_new_texto("PIN incorrecto. Intento "), jb_texto_desde_numero(intentos_pin)));
        }
    }
    if (jb_truthy(jb_eq(autorizado, jb_new_entero(0)))) {
        jb_imprimir(jb_new_texto("Tarjeta bloqueada. Adiós."));
        jb_assign(&salir, jb_new_entero(1));
    }
    if (jb_truthy(jb_eq(autorizado, jb_new_entero(1)))) {
        (jb_warn_aot("llamada AOT no implementada: limpiar_consola"), jb_new_nulo());
        jb_imprimir(jb_new_texto("--- Acceso concedido ---"));
        jb_var_t ref = jb_formatear_timestamp(ts_demo, jb_new_texto("%Y-%m-%d"));
        jb_imprimir(jb_concat(jb_new_texto("Operación referencia (fecha fija prueba): "), ref));
        while (jb_truthy(jb_eq(salir, jb_new_entero(0)))) {
            jb_imprimir(jb_new_texto("\n=== MENÚ ==="));
            jb_imprimir(jb_new_texto("1  Consultar saldo"));
            jb_imprimir(jb_new_texto("2  Depósito"));
            jb_imprimir(jb_new_texto("3  Retiro"));
            jb_imprimir(jb_new_texto("4  Historial de movimientos"));
            jb_imprimir(jb_new_texto("5  Panel técnico (bits / math)"));
            jb_imprimir(jb_new_texto("6  Ingreso por ingresar_texto (alias teclado)"));
            jb_imprimir(jb_new_texto("0  Salir"));
            jb_imprimir_sin_salto(jb_new_texto("Escriba opción y pulse Enter: "));
            jb_var_t linea = jb_new_nulo();
            jb_ingresar_texto(&linea);
            jb_var_t op = jb_str_a_entero(linea);
            {
                jb_var_t __sel = op;
                if (jb_truthy(jb_eq(__sel, jb_new_entero(1)))) {
                    jb_imprimir(jb_concat(jb_new_texto("Saldo actual: "), jb_decimal(saldo, jb_new_entero(2))));
                }
                else if (jb_truthy(jb_eq(__sel, jb_new_entero(2)))) {
                    jb_imprimir(jb_new_texto("Importe a depositar (ej. 100.50):"));
                    jb_var_t tdep = jb_leer_entrada();
                    jb_var_t dep = jb_str_a_flotante(tdep);
                    if (jb_truthy(jb_gt(dep, jb_new_flotante(0)))) {
                        jb_assign(&saldo, jb_add(saldo, dep));
                        jb_list_push(&historial, jb_concat(jb_new_texto("DEP +"), tdep));
                        jb_imprimir(jb_new_texto("Depósito aplicado."));
                    } else {
                        jb_imprimir(jb_new_texto("Importe inválido."));
                    }
                }
                else if (jb_truthy(jb_eq(__sel, jb_new_entero(3)))) {
                    jb_imprimir(jb_new_texto("Importe a retirar:"));
                    jb_var_t tre = jb_leer_entrada();
                    jb_var_t ret = jb_str_a_flotante(tre);
                    if (jb_truthy(jb_le(ret, jb_new_flotante(0)))) {
                        jb_imprimir(jb_new_texto("Importe inválido."));
                    }
                    if (jb_truthy(jb_gt(ret, jb_new_flotante(0)))) {
                        if (jb_truthy(jb_gt(ret, saldo))) {
                            jb_imprimir(jb_new_texto("Fondos insuficientes."));
                        }
                    }
                    if (jb_truthy(jb_gt(ret, jb_new_flotante(0)))) {
                        if (jb_truthy(jb_le(ret, saldo))) {
                            jb_assign(&saldo, jb_sub(saldo, ret));
                            jb_list_push(&historial, jb_concat(jb_new_texto("RET -"), tre));
                            jb_imprimir(jb_new_texto("Retiro realizado."));
                        }
                    }
                }
                else if (jb_truthy(jb_eq(__sel, jb_new_entero(4)))) {
                    jb_var_t nh = jb_list_len(historial);
                    if (jb_truthy(jb_eq(nh, jb_new_entero(0)))) {
                        jb_imprimir(jb_new_texto("(sin movimientos)"));
                    } else {
                        jb_imprimir(jb_new_texto("--- Historial ---"));
                        {
                            jb_var_t __coll = historial;
                            jb_var_t __len = jb_list_len(__coll);
                            for (int64_t __i = 0; __i < __len.u.i64; __i++) {
                                jb_var_t mov = jb_list_get(__coll, jb_new_entero(__i));
                                jb_imprimir(mov);
                            }
                        }
                    }
                }
                else if (jb_truthy(jb_eq(__sel, jb_new_entero(5)))) {
                    jb_imprimir(jb_new_texto("--- Panel técnico ---"));
                    jb_imprimir(jb_texto_desde_numero(jb_bit_shl(jb_new_entero(1), jb_new_entero(8))));
                    jb_imprimir(jb_texto_desde_numero(jb_bit_shr(jb_new_entero(256), jb_new_entero(4))));
                    jb_imprimir(jb_texto_desde_numero(jb_sin(jb_new_flotante(0))));
                    jb_imprimir(jb_texto_desde_numero(jb_cos(jb_new_flotante(0))));
                    jb_imprimir(jb_texto_desde_numero(jb_log10(cien_arc)));
                    jb_imprimir(jb_texto_desde_numero(jb_exp(jb_new_flotante(0))));
                    jb_imprimir(jb_concat(jb_new_texto("Flotante demo (decimal): "), jb_decimal(demo_fl, jb_new_entero(4))));
                }
                else if (jb_truthy(jb_eq(__sel, jb_new_entero(6)))) {
                    jb_imprimir(jb_new_texto("Escriba un alias (una línea):"));
                    jb_var_t alias = jb_new_nulo();
                    jb_ingresar_texto(&alias);
                    if (jb_truthy(jb_eq(jb_texto_len(alias), jb_new_entero(0)))) {
                        jb_imprimir(jb_new_texto("(vacío ignorado)"));
                    } else {
                        jb_imprimir(jb_concat(jb_new_texto("Alias recibido: "), jb_minusculas(alias)));
                    }
                }
                else if (jb_truthy(jb_eq(__sel, jb_new_entero(0)))) {
                    jb_imprimir(jb_new_texto("Gracias por usar el cajero Jasboot."));
                    jb_assign(&salir, jb_new_entero(1));
                }
                else {
                    if (jb_truthy(jb_contiene_texto(jb_minusculas(linea), jb_new_texto("sal")))) {
                        jb_imprimir(jb_new_texto("Saliendo…"));
                        jb_assign(&salir, jb_new_entero(1));
                    } else {
                        jb_imprimir(jb_new_texto("Opción no reconocida."));
                    }
                }
            }
        }
    }
    jb_cleanup();
    return 0;
}
