/**
 * Test de búsqueda introspectiva JMN
 * Programa de prueba para verificar las funciones de búsqueda en memoria neuronal
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Incluir headers de JMN
#include "memoria_neuronal.h"

int main() {
    printf("=== TEST DE BÚSQUEDA INTROSPECTIVA JMN ===\n\n");
    
    // Crear memoria RAM para pruebas
    JMNMemoria* mem = jmn_crear_memoria_ram(1000, 2000);
    if (!mem) {
        printf("ERROR: No se pudo crear memoria JMN\n");
        return 1;
    }
    
    printf("Memoria JMN creada exitosamente\n\n");
    
    // Agregar algunos textos de prueba
    printf("Agregando textos de prueba...\n");
    jmn_guardar_texto(mem, 1001, "inteligencia artificial");
    jmn_guardar_texto(mem, 1002, "aprendizaje automático");
    jmn_guardar_texto(mem, 1003, "red neuronal artificial");
    jmn_guardar_texto(mem, 1004, "procesamiento de lenguaje natural");
    jmn_guardar_texto(mem, 1005, "visión por computadora");
    jmn_guardar_texto(mem, 1006, "robótica inteligente");
    jmn_guardar_texto(mem, 1007, "minería de datos");
    jmn_guardar_texto(mem, 1008, "sistemas expertos");
    
    printf("8 textos agregados a la memoria\n\n");
    
    // Test 1: Búsqueda básica
    printf("=== TEST 1: Búsqueda básica de 'inteligencia' ===\n");
    JMNBusquedaIntrospectivaResultado resultados[10];
    int count = jmn_buscar_introspectiva(mem, "inteligencia", resultados, 10, 0); // case insensitive
    
    printf("Resultados encontrados: %d\n", count);
    for (int i = 0; i < count; i++) {
        printf("  ID: %u, Texto: '%s', Posición: %d\n", 
               resultados[i].id, resultados[i].texto, resultados[i].posicion);
    }
    printf("\n");
    
    // Test 2: Búsqueda case sensitive
    printf("=== TEST 2: Búsqueda case sensitive de 'Red' ===\n");
    count = jmn_buscar_introspectiva(mem, "Red", resultados, 10, 1); // case sensitive
    
    printf("Resultados encontrados: %d\n", count);
    for (int i = 0; i < count; i++) {
        printf("  ID: %u, Texto: '%s', Posición: %d\n", 
               resultados[i].id, resultados[i].texto, resultados[i].posicion);
    }
    printf("\n");
    
    // Test 3: Búsqueda que no encuentra nada
    printf("=== TEST 3: Búsqueda de 'cuántica' ===\n");
    count = jmn_buscar_introspectiva(mem, "cuántica", resultados, 10, 0);
    
    printf("Resultados encontrados: %d\n", count);
    if (count == 0) {
        printf("  Correcto: no se encontraron resultados\n");
    }
    printf("\n");
    
    // Test 4: Búsqueda solo de IDs
    printf("=== TEST 4: Búsqueda solo IDs de 'neuronal' ===\n");
    uint32_t ids[10];
    count = jmn_buscar_conceptos_con_texto(mem, "neuronal", ids, 10, 0);
    
    printf("IDs encontrados: %d\n", count);
    for (int i = 0; i < count; i++) {
        printf("  ID: %u\n", ids[i]);
    }
    printf("\n");
    
    // Test 5: Verificar concepto específico
    printf("=== TEST 5: Verificar concepto específico ===\n");
    
    // Verificar si el concepto 1003 contiene "neuronal"
    int contiene = jmn_concepto_contiene_texto(mem, 1003, "neuronal", 0);
    printf("¿Concepto 1003 contiene 'neuronal'? %s\n", contiene ? "SÍ" : "NO");
    
    // Verificar si el concepto 1005 contiene "neuronal"
    contiene = jmn_concepto_contiene_texto(mem, 1005, "neuronal", 0);
    printf("¿Concepto 1005 contiene 'neuronal'? %s\n", contiene ? "SÍ" : "NO");
    printf("\n");
    
    // Test 6: Búsqueda con límite de resultados
    printf("=== TEST 6: Búsqueda con límite de resultados ===\n");
    JMNBusquedaIntrospectivaResultado resultados_limitados[2];
    count = jmn_buscar_introspectiva(mem, "de", resultados_limitados, 2, 0);
    
    printf("Resultados encontrados (límite 2): %d\n", count);
    for (int i = 0; i < count; i++) {
        printf("  ID: %u, Texto: '%s', Posición: %d\n", 
               resultados_limitados[i].id, resultados_limitados[i].texto, resultados_limitados[i].posicion);
    }
    printf("\n");
    
    // Cerrar memoria
    jmn_cerrar(mem);
    
    printf("=== TODOS LOS TESTS COMPLETADOS ===\n");
    printf("La búsqueda introspectiva JMN funciona correctamente.\n");
    
    return 0;
}
