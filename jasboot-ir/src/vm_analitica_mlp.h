#ifndef VM_ANALITICA_MLP_H
#define VM_ANALITICA_MLP_H

#include "vm.h"
#include <stdint.h>

/* Entrena in-place un MLP 1 capa oculta (ReLU) + salida sigmoid.
 * Registros base..base+6 (uint32 list ids salvo lr y epochs):
 *   base+0: lista pesos_capas (2 matrices anidadas)
 *   base+1: lista sesgos_capas
 *   base+2: lista X (muestras = listas de n_in flotantes)
 *   base+3: lista y (un flotante por muestra)
 *   base+4: lista capas_ocultas (primer elemento = H, flotante o entero en celda)
 *   base+5: lr (bits float32 en registro bajo)
 *   base+6: epochs (entero en registro completo, se usa bajo 32 bits)
 * Devuelve MSE del ultimo epoch (float). Escribe pesos/sesgos en JMN/listas. */
float vm_analitica_mlp_fit_native(VM* vm, uint8_t base_reg);

#endif
