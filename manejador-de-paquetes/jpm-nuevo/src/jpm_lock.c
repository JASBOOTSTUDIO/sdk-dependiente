/*
 * JPM - Jasboot Package Manager
 * Módulo de Lock: jpm_lock.c
 * 
 * Gestión del archivo jpm.lock para resolución determinista de dependencias
 * Formato JSON para persistencia de versiones instaladas
 * Copyright (c) 2024 Jasboot Team
 */

#include "../include/jpm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* =============================================================================
 * FUNCIONES PRIVADAS DE SERIALIZACIÓN
 * =============================================================================
 */

/* Escribe una entrada de dependencia al archivo */
static void escribir_dependencia_json(FILE *f, const jpm_dependencia_t *dep, bool es_ultima) {
    fprintf(f, "      \"%s\": \"%s%s\"", dep->nombre, dep->operador, dep->version);
    if (!es_ultima) {
        fprintf(f, ",");
    }
    fprintf(f, "\n");
}

/* Escribe una entrada del lock al archivo */
static void escribir_entrada_json(FILE *f, const jpm_lock_entrada_t *entrada, bool es_ultima) {
    fprintf(f, "    \"%s\": {\n", entrada->nombre);
    fprintf(f, "      \"version\": \"%s\",\n", entrada->version);
    fprintf(f, "      \"resuelto\": \"%s\"", entrada->resuelto);
    
    if (strlen(entrada->hash) > 0) {
        fprintf(f, ",\n      \"hash\": \"%s\"", entrada->hash);
    }
    
    if (entrada->num_dependencias > 0 && entrada->dependencias) {
        fprintf(f, ",\n      \"dependencias\": {\n");
        for (size_t i = 0; i < entrada->num_dependencias; i++) {
            escribir_dependencia_json(f, &entrada->dependencias[i], 
                                     i == entrada->num_dependencias - 1);
        }
        fprintf(f, "      }");
    }
    
    fprintf(f, "\n    }");
    if (!es_ultima) {
        fprintf(f, ",");
    }
    fprintf(f, "\n");
}

/* Parsea objeto de dependencias desde JSON */
static int parsear_dependencias_json(void *obj_dependencias, jpm_dependencia_t **deps_out, 
                                     size_t *num_deps_out) {
    if (!obj_dependencias) {
        *deps_out = NULL;
        *num_deps_out = 0;
        return JPM_EXITO;
    }

    /* Contar número de dependencias */
    /* Nota: Esta es una implementación simplificada. 
     * En producción, necesitaríamos iterar sobre las claves del objeto JSON */
    
    /* Por ahora, asumimos un máximo de dependencias */
    jpm_dependencia_t *deps = (jpm_dependencia_t*)calloc(JPM_MAX_DEPENDENCIAS, 
                                                         sizeof(jpm_dependencia_t));
    if (!deps) {
        return JPM_ERROR_MEMORIA;
    }

    size_t count = 0;
    
    /* Aquí iríamos iterando sobre cada clave del objeto JSON
     * Pero como nuestra implementación JSON es simplificada, 
     * dejaremos esto como stub por ahora */
    
    *deps_out = deps;
    *num_deps_out = count;
    
    return JPM_EXITO;
}

/* Parsea una entrada del lock desde JSON */
static int parsear_entrada_json(const char *nombre, void *obj_entrada, 
                                jpm_lock_entrada_t *entrada) {
    if (!nombre || !obj_entrada || !entrada) {
        return JPM_ERROR_VALIDACION;
    }

    memset(entrada, 0, sizeof(jpm_lock_entrada_t));

    /* Copiar nombre */
    strncpy(entrada->nombre, nombre, JPM_MAX_NOMBRE - 1);
    entrada->nombre[JPM_MAX_NOMBRE - 1] = '\0';

    /* Obtener version */
    const char *version = jpm_json_obtener_string(obj_entrada, "version");
    if (version) {
        strncpy(entrada->version, version, JPM_MAX_VERSION - 1);
        entrada->version[JPM_MAX_VERSION - 1] = '\0';
    }

    /* Obtener resuelto */
    const char *resuelto = jpm_json_obtener_string(obj_entrada, "resuelto");
    if (resuelto) {
        strncpy(entrada->resuelto, resuelto, JPM_MAX_URL - 1);
        entrada->resuelto[JPM_MAX_URL - 1] = '\0';
    }

    /* Obtener hash (opcional) */
    const char *hash = jpm_json_obtener_string(obj_entrada, "hash");
    if (hash) {
        strncpy(entrada->hash, hash, 128);
        entrada->hash[128] = '\0';
    }

    /* Obtener dependencias (opcional) */
    void *obj_deps = jpm_json_obtener_objeto(obj_entrada, "dependencias");
    if (obj_deps) {
        parsear_dependencias_json(obj_deps, &entrada->dependencias, 
                                 &entrada->num_dependencias);
    } else {
        entrada->dependencias = NULL;
        entrada->num_dependencias = 0;
    }

    return JPM_EXITO;
}

/* =============================================================================
 * FUNCIONES PÚBLICAS
 * =============================================================================
 */

jpm_lock_t* jpm_lock_crear(void) {
    jpm_lock_t *lock = (jpm_lock_t*)calloc(1, sizeof(jpm_lock_t));
    if (!lock) {
        fprintf(stderr, "[ERROR] Fallo al asignar memoria para lock\n");
        return NULL;
    }

    lock->entradas = NULL;
    lock->num_entradas = 0;
    lock->generado_en = time(NULL);

    return lock;
}

void jpm_lock_liberar(jpm_lock_t *lock) {
    if (!lock) return;

    if (lock->entradas) {
        for (size_t i = 0; i < lock->num_entradas; i++) {
            if (lock->entradas[i].dependencias) {
                free(lock->entradas[i].dependencias);
            }
        }
        free(lock->entradas);
    }

    free(lock);
}

jpm_lock_t* jpm_lock_leer(const char *ruta_archivo) {
    if (!ruta_archivo) {
        fprintf(stderr, "[ERROR] Ruta de archivo inválida\n");
        return NULL;
    }

    /* Verificar que el archivo existe */
    FILE *f = fopen(ruta_archivo, "r");
    if (!f) {
        fprintf(stderr, "[ERROR] No se pudo abrir archivo lock: %s\n", ruta_archivo);
        return NULL;
    }
    fclose(f);

    /* Leer archivo JSON */
    void *obj_root = jpm_json_leer_archivo(ruta_archivo);
    if (!obj_root) {
        fprintf(stderr, "[ERROR] Fallo al parsear JSON de lock\n");
        return NULL;
    }

    /* Crear estructura lock */
    jpm_lock_t *lock = jpm_lock_crear();
    if (!lock) {
        jpm_json_liberar(obj_root);
        return NULL;
    }

    /* Obtener versión del lock */
    const char *version = jpm_json_obtener_string(obj_root, "version");
    if (!version || strcmp(version, "1.0.0") != 0) {
        fprintf(stderr, "[AVISO] Versión de lock desconocida o no especificada\n");
    }

    /* Obtener objeto de paquetes */
    void *obj_paquetes = jpm_json_obtener_objeto(obj_root, "paquetes");
    if (!obj_paquetes) {
        fprintf(stderr, "[AVISO] No se encontraron paquetes en lock\n");
        jpm_json_liberar(obj_root);
        return lock;
    }

    /* Contar paquetes y reservar memoria
     * Nota: En una implementación completa de JSON, iteraríamos sobre las claves
     * Por ahora, usamos un enfoque simplificado */
    
    /* Por simplicidad, parseamos un máximo de entradas */
    lock->entradas = (jpm_lock_entrada_t*)calloc(JPM_MAX_DEPENDENCIAS, 
                                                  sizeof(jpm_lock_entrada_t));
    if (!lock->entradas) {
        jpm_lock_liberar(lock);
        jpm_json_liberar(obj_root);
        return NULL;
    }

    /* Aquí parseamos las entradas
     * En producción, iteraríamos sobre cada clave del objeto "paquetes" */
    lock->num_entradas = 0;

    jpm_json_liberar(obj_root);
    return lock;
}

int jpm_lock_escribir(const jpm_lock_t *lock, const char *ruta_archivo) {
    if (!lock || !ruta_archivo) {
        fprintf(stderr, "[ERROR] Parámetros inválidos para escribir lock\n");
        return JPM_ERROR_VALIDACION;
    }

    FILE *f = fopen(ruta_archivo, "w");
    if (!f) {
        fprintf(stderr, "[ERROR] No se pudo crear archivo lock: %s\n", ruta_archivo);
        return JPM_ERROR_ARCHIVO;
    }

    /* Escribir estructura JSON */
    fprintf(f, "{\n");
    fprintf(f, "  \"version\": \"1.0.0\",\n");
    
    /* Timestamp */
    char timestamp[64];
    struct tm *tm_info = localtime(&lock->generado_en);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", tm_info);
    fprintf(f, "  \"generado\": \"%s\",\n", timestamp);
    
    /* Paquetes */
    fprintf(f, "  \"paquetes\": {\n");
    
    for (size_t i = 0; i < lock->num_entradas; i++) {
        escribir_entrada_json(f, &lock->entradas[i], i == lock->num_entradas - 1);
    }
    
    fprintf(f, "  }\n");
    fprintf(f, "}\n");

    fclose(f);

    printf("[INFO] Archivo lock actualizado: %s\n", ruta_archivo);
    return JPM_EXITO;
}

int jpm_lock_agregar_entrada(jpm_lock_t *lock, const jpm_lock_entrada_t *entrada) {
    if (!lock || !entrada) {
        fprintf(stderr, "[ERROR] Parámetros inválidos para agregar entrada\n");
        return JPM_ERROR_VALIDACION;
    }

    /* Verificar si la entrada ya existe */
    for (size_t i = 0; i < lock->num_entradas; i++) {
        if (strcmp(lock->entradas[i].nombre, entrada->nombre) == 0) {
            /* Actualizar entrada existente */
            
            /* Liberar dependencias anteriores si existen */
            if (lock->entradas[i].dependencias) {
                free(lock->entradas[i].dependencias);
            }
            
            /* Copiar nueva entrada */
            strncpy(lock->entradas[i].version, entrada->version, JPM_MAX_VERSION - 1);
            lock->entradas[i].version[JPM_MAX_VERSION - 1] = '\0';
            
            strncpy(lock->entradas[i].resuelto, entrada->resuelto, JPM_MAX_URL - 1);
            lock->entradas[i].resuelto[JPM_MAX_URL - 1] = '\0';
            
            strncpy(lock->entradas[i].hash, entrada->hash, 128);
            lock->entradas[i].hash[128] = '\0';
            
            /* Copiar dependencias */
            if (entrada->num_dependencias > 0 && entrada->dependencias) {
                lock->entradas[i].dependencias = (jpm_dependencia_t*)malloc(
                    sizeof(jpm_dependencia_t) * entrada->num_dependencias
                );
                if (lock->entradas[i].dependencias) {
                    memcpy(lock->entradas[i].dependencias, entrada->dependencias,
                           sizeof(jpm_dependencia_t) * entrada->num_dependencias);
                    lock->entradas[i].num_dependencias = entrada->num_dependencias;
                } else {
                    lock->entradas[i].num_dependencias = 0;
                }
            } else {
                lock->entradas[i].dependencias = NULL;
                lock->entradas[i].num_dependencias = 0;
            }
            
            return JPM_EXITO;
        }
    }

    /* Agregar nueva entrada */
    jpm_lock_entrada_t *nuevas_entradas = (jpm_lock_entrada_t*)realloc(
        lock->entradas,
        sizeof(jpm_lock_entrada_t) * (lock->num_entradas + 1)
    );

    if (!nuevas_entradas) {
        fprintf(stderr, "[ERROR] Fallo al expandir entradas del lock\n");
        return JPM_ERROR_MEMORIA;
    }

    lock->entradas = nuevas_entradas;

    /* Copiar entrada */
    jpm_lock_entrada_t *nueva = &lock->entradas[lock->num_entradas];
    memset(nueva, 0, sizeof(jpm_lock_entrada_t));

    strncpy(nueva->nombre, entrada->nombre, JPM_MAX_NOMBRE - 1);
    nueva->nombre[JPM_MAX_NOMBRE - 1] = '\0';

    strncpy(nueva->version, entrada->version, JPM_MAX_VERSION - 1);
    nueva->version[JPM_MAX_VERSION - 1] = '\0';

    strncpy(nueva->resuelto, entrada->resuelto, JPM_MAX_URL - 1);
    nueva->resuelto[JPM_MAX_URL - 1] = '\0';

    strncpy(nueva->hash, entrada->hash, 128);
    nueva->hash[128] = '\0';

    /* Copiar dependencias */
    if (entrada->num_dependencias > 0 && entrada->dependencias) {
        nueva->dependencias = (jpm_dependencia_t*)malloc(
            sizeof(jpm_dependencia_t) * entrada->num_dependencias
        );
        if (nueva->dependencias) {
            memcpy(nueva->dependencias, entrada->dependencias,
                   sizeof(jpm_dependencia_t) * entrada->num_dependencias);
            nueva->num_dependencias = entrada->num_dependencias;
        } else {
            nueva->num_dependencias = 0;
        }
    } else {
        nueva->dependencias = NULL;
        nueva->num_dependencias = 0;
    }

    lock->num_entradas++;

    return JPM_EXITO;
}

/* Funciones auxiliares */

int jpm_lock_obtener_entrada(const jpm_lock_t *lock, const char *nombre,
                             jpm_lock_entrada_t *entrada_salida) {
    if (!lock || !nombre || !entrada_salida) {
        return JPM_ERROR_VALIDACION;
    }

    for (size_t i = 0; i < lock->num_entradas; i++) {
        if (strcmp(lock->entradas[i].nombre, nombre) == 0) {
            memcpy(entrada_salida, &lock->entradas[i], sizeof(jpm_lock_entrada_t));
            
            /* Copiar dependencias si existen */
            if (lock->entradas[i].num_dependencias > 0 && lock->entradas[i].dependencias) {
                entrada_salida->dependencias = (jpm_dependencia_t*)malloc(
                    sizeof(jpm_dependencia_t) * lock->entradas[i].num_dependencias
                );
                if (entrada_salida->dependencias) {
                    memcpy(entrada_salida->dependencias, lock->entradas[i].dependencias,
                           sizeof(jpm_dependencia_t) * lock->entradas[i].num_dependencias);
                }
            }
            
            return JPM_EXITO;
        }
    }

    return JPM_ERROR_NO_ENCONTRADO;
}

bool jpm_lock_contiene_paquete(const jpm_lock_t *lock, const char *nombre) {
    if (!lock || !nombre) {
        return false;
    }

    for (size_t i = 0; i < lock->num_entradas; i++) {
        if (strcmp(lock->entradas[i].nombre, nombre) == 0) {
            return true;
        }
    }

    return false;
}

int jpm_lock_eliminar_entrada(jpm_lock_t *lock, const char *nombre) {
    if (!lock || !nombre) {
        return JPM_ERROR_VALIDACION;
    }

    for (size_t i = 0; i < lock->num_entradas; i++) {
        if (strcmp(lock->entradas[i].nombre, nombre) == 0) {
            /* Liberar dependencias */
            if (lock->entradas[i].dependencias) {
                free(lock->entradas[i].dependencias);
            }

            /* Mover entradas siguientes */
            if (i < lock->num_entradas - 1) {
                memmove(&lock->entradas[i], &lock->entradas[i + 1],
                       sizeof(jpm_lock_entrada_t) * (lock->num_entradas - i - 1));
            }

            lock->num_entradas--;

            /* Reducir tamaño del array */
            if (lock->num_entradas > 0) {
                jpm_lock_entrada_t *nuevas = (jpm_lock_entrada_t*)realloc(
                    lock->entradas,
                    sizeof(jpm_lock_entrada_t) * lock->num_entradas
                );
                if (nuevas) {
                    lock->entradas = nuevas;
                }
            } else {
                free(lock->entradas);
                lock->entradas = NULL;
            }

            return JPM_EXITO;
        }
    }

    return JPM_ERROR_NO_ENCONTRADO;
}

void jpm_lock_listar_paquetes(const jpm_lock_t *lock) {
    if (!lock) {
        fprintf(stderr, "[ERROR] Lock inválido\n");
        return;
    }

    if (lock->num_entradas == 0) {
        printf("[INFO] No hay paquetes en el lock\n");
        return;
    }

    printf("\nPaquetes en jpm.lock:\n");
    printf("─────────────────────────────────────────────────────\n");

    for (size_t i = 0; i < lock->num_entradas; i++) {
        const jpm_lock_entrada_t *entrada = &lock->entradas[i];
        printf("  %s@%s\n", entrada->nombre, entrada->version);
        printf("    Resuelto: %s\n", entrada->resuelto);
        
        if (strlen(entrada->hash) > 0) {
            printf("    Hash: %.16s...\n", entrada->hash);
        }
        
        if (entrada->num_dependencias > 0) {
            printf("    Dependencias: %zu\n", entrada->num_dependencias);
        }
        
        if (i < lock->num_entradas - 1) {
            printf("\n");
        }
    }

    printf("─────────────────────────────────────────────────────\n");
    printf("Total: %zu paquetes\n\n", lock->num_entradas);
}