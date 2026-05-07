/**
 * JMN Búsqueda Introspectiva: búsqueda de texto en todos los conceptos almacenados
 * Permite buscar cadenas de texto en toda la memoria neuronal JMN
 */
#include "memoria_neuronal.h"
#include "jmn_interno.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

// La estructura JMNBusquedaIntrospectivaResultado está definida en memoria_neuronal.h

/**
 * Estructura de contexto para búsqueda interna
 */
typedef struct {
    const char* termino_busqueda;    // Término a buscar
    int longitud_termino;            // Longitud del término
    JMNBusquedaIntrospectivaResultado* resultados; // Array de resultados
    uint32_t capacidad_resultados;   // Capacidad del array
    uint32_t num_resultados;         // Número de resultados encontrados
    int case_sensitive;              // 0 = insensitive, 1 = sensitive
} JMNBusquedaContexto;

/**
 * Función auxiliar para convertir a minúsculas (case insensitive)
 */
static void a_minusculas(char* str) {
    if (!str) return;
    for (int i = 0; str[i]; i++) {
        str[i] = (char)tolower((unsigned char)str[i]);
    }
}

/**
 * Función auxiliar para buscar substring con opción case insensitive
 */
static char* buscar_substring(const char* haystack, const char* needle, int case_sensitive) {
    if (!haystack || !needle) return NULL;
    
    if (!case_sensitive) {
        // Crear copias para búsqueda case insensitive
        char* haystack_lower = strdup(haystack);
        char* needle_lower = strdup(needle);
        
        if (!haystack_lower || !needle_lower) {
            free(haystack_lower);
            free(needle_lower);
            return NULL;
        }
        
        a_minusculas(haystack_lower);
        a_minusculas(needle_lower);
        
        char* resultado = strstr(haystack_lower, needle_lower);
        char* resultado_final = NULL;
        
        if (resultado) {
            // Calcular posición en el string original
            ptrdiff_t offset = resultado - haystack_lower;
            resultado_final = (char*)(haystack + offset);
        }
        
        free(haystack_lower);
        free(needle_lower);
        return resultado_final;
    } else {
        return strstr(haystack, needle);
    }
}

/**
 * Callback para iterar sobre todos los textos y buscar coincidencias
 */
static int buscar_en_texto_callback(uint32_t id, const char* texto, void* user_data) {
    JMNBusquedaContexto* ctx = (JMNBusquedaContexto*)user_data;
    
    if (!texto || !ctx->termino_busqueda) return 0;
    
    // Buscar el término en el texto
    char* posicion = buscar_substring(texto, ctx->termino_busqueda, ctx->case_sensitive);
    
    if (posicion) {
        // Encontramos coincidencia, agregar a resultados
        if (ctx->num_resultados < ctx->capacidad_resultados) {
            JMNBusquedaIntrospectivaResultado* resultado = &ctx->resultados[ctx->num_resultados];
            resultado->id = id;
            strncpy(resultado->texto, texto, sizeof(resultado->texto) - 1);
            resultado->texto[sizeof(resultado->texto) - 1] = '\0';
            resultado->posicion = (int)(posicion - texto);
            ctx->num_resultados++;
            
            if (getenv("JASBOOT_DEBUG")) {
                fprintf(stderr, "[JMN BUSQUEDA] Coincidencia encontrada: ID=%u, texto='%s', posicion=%d\n", 
                        id, resultado->texto, resultado->posicion);
            }
        }
    }
    
    return 0; // Continuar iteración
}

/**
 * Función auxiliar para iterar sobre todos los textos almacenados
 */
static int jmn_iterar_todos_textos(JMNMemoria* mem, int (*callback)(uint32_t, const char*, void*), void* user_data) {
    if (!mem || !callback) return -1;
    
    int count = 0;
    for (uint32_t i = 0; i < mem->cap_textos; i++) {
        if (mem->textos[i].used && mem->textos[i].texto[0] != '\0') {
            if (callback(mem->textos[i].id, mem->textos[i].texto, user_data) != 0) {
                break; // Callback solicitó detenerse
            }
            count++;
        }
    }
    
    return count;
}

/**
 * Función principal de búsqueda introspectiva
 * 
 * @param mem Memoria neuronal JMN
 * @param termino Término de búsqueda
 * @param resultados Array donde se almacenarán los resultados
 * @param max_resultados Capacidad máxima del array de resultados
 * @param case_sensitive 0 = insensitive, 1 = sensitive
 * @return Número de resultados encontrados, -1 en caso de error
 */
int jmn_buscar_introspectiva(JMNMemoria* mem, const char* termino, 
                           JMNBusquedaIntrospectivaResultado* resultados, 
                           uint32_t max_resultados, int case_sensitive) {
    
    if (!mem || !termino || !resultados || max_resultados == 0) {
        return -1;
    }
    
    if (strlen(termino) == 0) {
        return -1;
    }
    
    // Inicializar contexto de búsqueda
    JMNBusquedaContexto ctx = {
        .termino_busqueda = termino,
        .longitud_termino = (int)strlen(termino),
        .resultados = resultados,
        .capacidad_resultados = max_resultados,
        .num_resultados = 0,
        .case_sensitive = case_sensitive
    };
    
    if (getenv("JASBOOT_DEBUG")) {
        fprintf(stderr, "[JMN BUSQUEDA] Iniciando búsqueda introspectiva: término='%s', case_sensitive=%d\n", 
                termino, case_sensitive);
    }
    
    // Iterar sobre todos los textos buscando coincidencias
    int textos_procesados = jmn_iterar_todos_textos(mem, buscar_en_texto_callback, &ctx);
    
    if (getenv("JASBOOT_DEBUG")) {
        fprintf(stderr, "[JMN BUSQUEDA] Búsqueda completada: %d textos procesados, %u coincidencias encontradas\n", 
                textos_procesados, ctx.num_resultados);
    }
    
    return (int)ctx.num_resultados;
}

/**
 * Función simplificada que solo devuelve los IDs de conceptos que contienen el término
 * 
 * @param mem Memoria neuronal JMN
 * @param termino Término de búsqueda
 * @param ids Array donde se almacenarán los IDs encontrados
 * @param max_ids Capacidad máxima del array
 * @param case_sensitive 0 = insensitive, 1 = sensitive
 * @return Número de IDs encontrados, -1 en caso de error
 */
int jmn_buscar_conceptos_con_texto(JMNMemoria* mem, const char* termino, 
                                  uint32_t* ids, uint32_t max_ids, int case_sensitive) {
    
    if (!mem || !termino || !ids || max_ids == 0) {
        return -1;
    }
    
    // Usar buffer temporal para resultados completos
    JMNBusquedaIntrospectivaResultado* temp_resultados = 
        malloc(sizeof(JMNBusquedaIntrospectivaResultado) * max_ids);
    
    if (!temp_resultados) {
        return -1;
    }
    
    int count = jmn_buscar_introspectiva(mem, termino, temp_resultados, max_ids, case_sensitive);
    
    if (count > 0) {
        // Extraer solo los IDs
        for (int i = 0; i < count; i++) {
            ids[i] = temp_resultados[i].id;
        }
    }
    
    free(temp_resultados);
    return count;
}

/**
 * Función para verificar si un concepto específico contiene un término de búsqueda
 * 
 * @param mem Memoria neuronal JMN
 * @param id_concepto ID del concepto a verificar
 * @param termino Término de búsqueda
 * @param case_sensitive 0 = insensitive, 1 = sensitive
 * @return 1 si contiene el término, 0 si no lo contiene, -1 si hay error
 */
int jmn_concepto_contiene_texto(JMNMemoria* mem, uint32_t id_concepto, 
                               const char* termino, int case_sensitive) {
    
    if (!mem || id_concepto == 0 || !termino) {
        return -1;
    }
    
    char buffer[256];
    int resultado = jmn_obtener_texto(mem, id_concepto, buffer, sizeof(buffer));
    
    if (resultado <= 0) {
        return 0; // No se encontró el texto
    }
    
    char* posicion = buscar_substring(buffer, termino, case_sensitive);
    return (posicion != NULL) ? 1 : 0;
}

/**
 * Búsqueda introspectiva que devuelve lista de IDs (sin metadata adicional)
 * 
 * @param mem Memoria neuronal JMN
 * @param termino Término de búsqueda
 * @param ids Array donde se almacenarán los IDs encontrados
 * @param max_resultados Capacidad máxima del array
 * @param case_sensitive 0 = insensitive, 1 = sensitive
 * @return Número de IDs encontrados, -1 en caso de error
 */
int jmn_buscar_introspectiva_lista(JMNMemoria* mem, const char* termino,
                                   uint32_t* ids, uint32_t max_resultados,
                                   int case_sensitive) {
    
    if (!mem || !termino || !ids || max_resultados == 0) {
        return -1;
    }
    
    if (strlen(termino) == 0) {
        return -1;
    }
    
    // Usar buffer temporal para resultados completos
    JMNBusquedaIntrospectivaResultado* temp_resultados = 
        malloc(sizeof(JMNBusquedaIntrospectivaResultado) * max_resultados);
    
    if (!temp_resultados) {
        return -1;
    }
    
    int count = jmn_buscar_introspectiva(mem, termino, temp_resultados, max_resultados, case_sensitive);
    
    if (count > 0) {
        // Extraer solo los IDs
        for (int i = 0; i < count; i++) {
            ids[i] = temp_resultados[i].id;
        }
    }
    
    free(temp_resultados);
    return count;
}

/**
 * Búsqueda introspectiva con control de case sensitive (primer resultado)
 * 
 * @param mem Memoria neuronal JMN
 * @param termino Término de búsqueda
 * @param resultado Estructura donde se almacenará el resultado
 * @param case_sensitive 0 = insensitive, 1 = sensitive
 * @return 1 si se encontró, 0 si no, -1 en caso de error
 */
int jmn_buscar_introspectiva_cs(JMNMemoria* mem, const char* termino,
                                JMNBusquedaIntrospectivaResultado* resultado,
                                int case_sensitive) {
    
    if (!mem || !termino || !resultado) {
        return -1;
    }
    
    int count = jmn_buscar_introspectiva(mem, termino, resultado, 1, case_sensitive);
    return count > 0 ? 1 : 0;
}

/**
 * Calcular score de relevancia basado en la posición y longitud
 */
static float calcular_relevancia(int posicion, int longitud_texto, int longitud_match, int es_clave) {
    float score = 1.0f;
    
    // Bonus si se encuentra en la clave
    if (es_clave) {
        score *= 1.3f;
    }
    
    // Bonus si aparece al principio
    if (posicion == 0) {
        score *= 1.2f;
    } else if (posicion < 10) {
        score *= 1.1f;
    }
    
    // Bonus por porcentaje de match
    if (longitud_texto > 0) {
        float porcentaje = (float)longitud_match / (float)longitud_texto;
        score *= (0.5f + porcentaje * 0.5f);
    }
    
    // Normalizar a rango 0.0-1.0
    if (score > 1.0f) score = 1.0f;
    if (score < 0.0f) score = 0.0f;
    
    return score;
}

/**
 * Callback para búsqueda detallada con metadata
 */
typedef struct {
    const char* termino_busqueda;
    int longitud_termino;
    JMNBusquedaDetalladaResultado* resultados;
    uint32_t capacidad_resultados;
    uint32_t num_resultados;
    int case_sensitive;
} JMNBusquedaDetalladaContexto;

static int buscar_detallado_callback(uint32_t id, const char* texto, void* user_data) {
    JMNBusquedaDetalladaContexto* ctx = (JMNBusquedaDetalladaContexto*)user_data;
    
    if (!texto || !texto[0]) {
        return 1; // Continuar iteración
    }
    
    if (ctx->num_resultados >= ctx->capacidad_resultados) {
        return 0; // Detener iteración - capacidad llena
    }
    
    // Buscar el término en el texto
    char* posicion = buscar_substring(texto, ctx->termino_busqueda, ctx->case_sensitive);
    
    if (posicion) {
        JMNBusquedaDetalladaResultado* res = &ctx->resultados[ctx->num_resultados];
        
        res->id = id;
        strncpy(res->texto, texto, sizeof(res->texto) - 1);
        res->texto[sizeof(res->texto) - 1] = '\0';
        res->posicion = (int)(posicion - texto);
        res->longitud_match = ctx->longitud_termino;
        
        // Determinar si es clave o valor (heurística simple)
        // En JMN, las claves suelen ser más cortas y tener underscores
        int tiene_underscores = strchr(texto, '_') != NULL;
        int es_corta = strlen(texto) < 50;
        res->es_clave = (tiene_underscores && es_corta) ? 1 : 0;
        
        // Calcular relevancia
        res->relevancia = calcular_relevancia(
            res->posicion, 
            (int)strlen(texto),
            res->longitud_match,
            res->es_clave
        );
        
        ctx->num_resultados++;
        
        if (getenv("JASBOOT_DEBUG")) {
            fprintf(stderr, "[JMN BUSQUEDA DETALLADA] Coincidencia: ID=%u, pos=%d, relevancia=%.2f\n",
                    res->id, res->posicion, res->relevancia);
        }
    }
    
    return 1; // Continuar iteración
}

/**
 * Búsqueda introspectiva detallada con metadata completa
 * 
 * @param mem Memoria neuronal JMN
 * @param termino Término de búsqueda
 * @param resultados Array donde se almacenarán los resultados con metadata
 * @param max_resultados Capacidad máxima del array
 * @param case_sensitive 0 = insensitive, 1 = sensitive
 * @return Número de resultados encontrados, -1 en caso de error
 */
int jmn_buscar_introspectiva_detallada(JMNMemoria* mem, const char* termino,
                                       JMNBusquedaDetalladaResultado* resultados,
                                       uint32_t max_resultados,
                                       int case_sensitive) {
    
    if (!mem || !termino || !resultados || max_resultados == 0) {
        return -1;
    }
    
    if (strlen(termino) == 0) {
        return -1;
    }
    
    // Inicializar contexto de búsqueda
    JMNBusquedaDetalladaContexto ctx = {
        .termino_busqueda = termino,
        .longitud_termino = (int)strlen(termino),
        .resultados = resultados,
        .capacidad_resultados = max_resultados,
        .num_resultados = 0,
        .case_sensitive = case_sensitive
    };
    
    if (getenv("JASBOOT_DEBUG")) {
        fprintf(stderr, "[JMN BUSQUEDA DETALLADA] Iniciando: término='%s', max=%u, case_sensitive=%d\n", 
                termino, max_resultados, case_sensitive);
    }
    
    // Iterar sobre todos los textos buscando coincidencias
    int textos_procesados = jmn_iterar_todos_textos(mem, buscar_detallado_callback, &ctx);
    
    if (getenv("JASBOOT_DEBUG")) {
        fprintf(stderr, "[JMN BUSQUEDA DETALLADA] Completada: %d textos procesados, %u coincidencias\n", 
                textos_procesados, ctx.num_resultados);
    }
    
    return (int)ctx.num_resultados;
}
