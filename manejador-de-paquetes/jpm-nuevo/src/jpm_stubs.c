/*
 * JPM - Jasboot Package Manager
 * Módulo de Stubs: jpm_stubs.c
 * 
 * Implementaciones temporales de funciones no completadas
 * Copyright (c) 2024 Jasboot Team
 */

#include "../include/jpm.h"
#include <stdio.h>
#include <string.h>

/* =============================================================================
 * STUBS DE FUNCIONES DE RED Y REGISTRO
 * =============================================================================
 */

int jpm_descargar_archivo(const char *url, const char *ruta_destino, 
                          jpm_callback_progreso_t callback, void *datos_usuario) {
    (void)url;
    (void)ruta_destino;
    (void)callback;
    (void)datos_usuario;
    
    fprintf(stderr, "[ERROR] Descarga de archivos no implementada aún\n");
    fprintf(stderr, "La funcionalidad de descarga HTTP/HTTPS estará disponible en una versión futura.\n");
    fprintf(stderr, "Por ahora, solo se soporta instalación desde archivos locales .jpkg\n");
    
    return JPM_ERROR_GENERICO;
}

char* jpm_http_get(const char *url, int *codigo_estado) {
    (void)url;
    
    if (codigo_estado) {
        *codigo_estado = 0;
    }
    
    fprintf(stderr, "[ERROR] Cliente HTTP no implementado aún\n");
    return NULL;
}

char* jpm_http_post(const char *url, const char *datos, int *codigo_estado) {
    (void)url;
    (void)datos;
    
    if (codigo_estado) {
        *codigo_estado = 0;
    }
    
    fprintf(stderr, "[ERROR] Cliente HTTP no implementado aún\n");
    return NULL;
}

/* =============================================================================
 * STUBS DE FUNCIONES DE REGISTRO
 * =============================================================================
 */

jpm_metadatos_t** jpm_registro_buscar(jpm_contexto_t *ctx, const char *termino, 
                                      size_t *num_resultados) {
    (void)termino;
    
    if (num_resultados) {
        *num_resultados = 0;
    }
    
    JPM_LOG_ERROR(ctx, "Búsqueda en registro no implementada aún");
    JPM_LOG_INFO(ctx, "El registro central estará disponible en versiones futuras");
    
    return NULL;
}

jpm_metadatos_t* jpm_registro_obtener_info(jpm_contexto_t *ctx, const char *nombre) {
    (void)nombre;
    
    JPM_LOG_ERROR(ctx, "Consulta de registro no implementada aún");
    
    return NULL;
}

int jpm_registro_publicar(jpm_contexto_t *ctx, const char *archivo_jpkg) {
    (void)archivo_jpkg;
    
    JPM_LOG_ERROR(ctx, "Publicación al registro no implementada aún");
    JPM_LOG_INFO(ctx, "Para compartir tu paquete:");
    JPM_LOG_INFO(ctx, "  1. Empaqueta con: jpm pack");
    JPM_LOG_INFO(ctx, "  2. Comparte el archivo .jpkg manualmente");
    JPM_LOG_INFO(ctx, "  3. Otros pueden instalar con: jpm install tu-paquete.jpkg");
    
    return JPM_ERROR_GENERICO;
}

int jpm_registro_autenticar(jpm_contexto_t *ctx, const char *usuario, const char *password) {
    (void)usuario;
    (void)password;
    
    JPM_LOG_ERROR(ctx, "Autenticación no implementada aún");
    
    return JPM_ERROR_GENERICO;
}

int jpm_instalar_desde_registro(jpm_contexto_t *ctx, const char *nombre, const char *version) {
    (void)version;
    
    JPM_LOG_ERROR(ctx, "Instalación desde registro no implementada aún");
    JPM_LOG_INFO(ctx, "No se puede instalar '%s' desde el registro central", nombre);
    JPM_LOG_INFO(ctx, "Por ahora, solo se soporta instalación desde archivos locales:");
    JPM_LOG_INFO(ctx, "  jpm install ruta/al/paquete.jpkg");
    
    return JPM_ERROR_GENERICO;
}

/* =============================================================================
 * STUBS DE COMANDOS
 * =============================================================================
 */

int jpm_cmd_search(jpm_contexto_t *ctx, const char *termino) {
    JPM_LOG_INFO(ctx, "Buscando: %s", termino);
    JPM_LOG_ERROR(ctx, "El comando 'search' no está implementado aún");
    JPM_LOG_INFO(ctx, "");
    JPM_LOG_INFO(ctx, "Funcionalidad planificada para versiones futuras:");
    JPM_LOG_INFO(ctx, "  - Búsqueda en registro central");
    JPM_LOG_INFO(ctx, "  - Filtros por tipo, autor, licencia");
    JPM_LOG_INFO(ctx, "  - Ordenamiento por popularidad, fecha");
    JPM_LOG_INFO(ctx, "");
    JPM_LOG_INFO(ctx, "Por ahora, puedes:");
    JPM_LOG_INFO(ctx, "  - Listar paquetes instalados: jpm list");
    JPM_LOG_INFO(ctx, "  - Ver info de un paquete: jpm info <nombre>");
    
    return JPM_ERROR_GENERICO;
}

int jpm_cmd_publish(jpm_contexto_t *ctx) {
    JPM_LOG_INFO(ctx, "Preparando publicación...");
    JPM_LOG_ERROR(ctx, "El comando 'publish' no está implementado aún");
    JPM_LOG_INFO(ctx, "");
    JPM_LOG_INFO(ctx, "Para compartir tu paquete manualmente:");
    JPM_LOG_INFO(ctx, "  1. Empaqueta tu proyecto:");
    JPM_LOG_INFO(ctx, "     jpm pack");
    JPM_LOG_INFO(ctx, "");
    JPM_LOG_INFO(ctx, "  2. Comparte el archivo .jpkg generado");
    JPM_LOG_INFO(ctx, "");
    JPM_LOG_INFO(ctx, "  3. Otros pueden instalar con:");
    JPM_LOG_INFO(ctx, "     jpm install tu-paquete-1.0.0.jpkg");
    JPM_LOG_INFO(ctx, "");
    JPM_LOG_INFO(ctx, "El registro central estará disponible en versiones futuras");
    
    return JPM_ERROR_GENERICO;
}

int jpm_cmd_update(jpm_contexto_t *ctx, const char *nombre) {
    if (nombre) {
        JPM_LOG_INFO(ctx, "Actualizando paquete: %s", nombre);
    } else {
        JPM_LOG_INFO(ctx, "Actualizando todos los paquetes...");
    }
    
    JPM_LOG_ERROR(ctx, "El comando 'update' no está implementado aún");
    JPM_LOG_INFO(ctx, "");
    JPM_LOG_INFO(ctx, "Funcionalidad planificada:");
    JPM_LOG_INFO(ctx, "  - Verificar versiones nuevas en registro");
    JPM_LOG_INFO(ctx, "  - Actualizar automáticamente paquetes");
    JPM_LOG_INFO(ctx, "  - Respetar rangos de versión semántica");
    JPM_LOG_INFO(ctx, "");
    JPM_LOG_INFO(ctx, "Por ahora, para actualizar manualmente:");
    JPM_LOG_INFO(ctx, "  1. Desinstala: jpm uninstall <paquete>");
    JPM_LOG_INFO(ctx, "  2. Instala la nueva versión: jpm install nueva-version.jpkg");
    
    return JPM_ERROR_GENERICO;
}

int jpm_cmd_run(jpm_contexto_t *ctx, const char *script, int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    JPM_LOG_INFO(ctx, "Ejecutando script: %s", script);
    JPM_LOG_ERROR(ctx, "El comando 'run' no está implementado aún");
    JPM_LOG_INFO(ctx, "");
    JPM_LOG_INFO(ctx, "Funcionalidad planificada:");
    JPM_LOG_INFO(ctx, "  - Ejecutar scripts definidos en jasboot.json");
    JPM_LOG_INFO(ctx, "  - Paso de argumentos a scripts");
    JPM_LOG_INFO(ctx, "  - Variables de entorno personalizadas");
    JPM_LOG_INFO(ctx, "");
    JPM_LOG_INFO(ctx, "Por ahora, ejecuta scripts manualmente:");
    JPM_LOG_INFO(ctx, "  Revisa la sección 'scripts' en jasboot.json");
    JPM_LOG_INFO(ctx, "  Ejecuta los comandos directamente en tu terminal");
    
    return JPM_ERROR_GENERICO;
}

/* =============================================================================
 * STUBS DE RESOLUCIÓN DE DEPENDENCIAS
 * =============================================================================
 */

int jpm_resolver_dependencias(jpm_contexto_t *ctx, jpm_metadatos_t *meta, 
                              jpm_lock_entrada_t **entradas, size_t *num_entradas) {
    (void)meta;
    
    if (entradas) {
        *entradas = NULL;
    }
    if (num_entradas) {
        *num_entradas = 0;
    }
    
    JPM_LOG_AVISO(ctx, "Resolución automática de dependencias no implementada completamente");
    JPM_LOG_INFO(ctx, "Las dependencias deben instalarse manualmente por ahora");
    
    return JPM_EXITO;
}

/* =============================================================================
 * STUBS DE PARSEO DE VERSIONES SEMÁNTICAS
 * =============================================================================
 */

int jpm_parsear_version_semantica(const char *version, jpm_version_sem_t *v) {
    if (!version || !v) return JPM_ERROR_VALIDACION;
    
    /* Parseo básico */
    int num = sscanf(version, "%d.%d.%d", &v->mayor, &v->menor, &v->parche);
    if (num != 3) {
        return JPM_ERROR_VALIDACION;
    }
    
    v->pre_release[0] = '\0';
    v->build[0] = '\0';
    
    /* TODO: Parsear pre-release y build metadata */
    
    return JPM_EXITO;
}

bool jpm_version_cumple_operador(const jpm_version_sem_t *version, 
                                 const char *operador, 
                                 const jpm_version_sem_t *requisito) {
    if (!version || !operador || !requisito) return false;
    
    /* Comparación simple */
    int cmp = 0;
    
    if (version->mayor != requisito->mayor) {
        cmp = (version->mayor > requisito->mayor) ? 1 : -1;
    } else if (version->menor != requisito->menor) {
        cmp = (version->menor > requisito->menor) ? 1 : -1;
    } else if (version->parche != requisito->parche) {
        cmp = (version->parche > requisito->parche) ? 1 : -1;
    }
    
    /* Operadores */
    if (strcmp(operador, ">=") == 0) {
        return cmp >= 0;
    } else if (strcmp(operador, "<=") == 0) {
        return cmp <= 0;
    } else if (strcmp(operador, ">") == 0) {
        return cmp > 0;
    } else if (strcmp(operador, "<") == 0) {
        return cmp < 0;
    } else if (strcmp(operador, "=") == 0 || strcmp(operador, "==") == 0) {
        return cmp == 0;
    }
    
    return false;
}