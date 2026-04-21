/*
 * JPM - Jasboot Package Manager
 * Módulo de Instalación: jpm_install.c
 * 
 * Instalación de paquetes desde archivos .jpkg locales y remotos
 * Gestión de dependencias y registro de instalación
 * Copyright (c) 2024 Jasboot Team
 */

#include "../include/jpm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#ifdef _WIN32
    #include <direct.h>
    #include <windows.h>
    #define PATH_SEP "\\"
    #define mkdir(path, mode) _mkdir(path)
#else
    #include <unistd.h>
    #define PATH_SEP "/"
#endif

/* =============================================================================
 * FUNCIONES PRIVADAS
 * =============================================================================
 */

/* Obtiene el directorio de instalación para un paquete */
static void obtener_directorio_instalacion(jpm_contexto_t *ctx, const char *nombre,
                                           char *ruta_salida, size_t tam_salida) {
    if (ctx->config.es_global) {
        snprintf(ruta_salida, tam_salida, "%s%s%s", 
                 ctx->config.directorio_global, PATH_SEP, nombre);
    } else {
        snprintf(ruta_salida, tam_salida, "%s%s%s%s%s",
                 ctx->directorio_trabajo, PATH_SEP, 
                 JPM_DIRECTORIO_DEPENDENCIAS, PATH_SEP, nombre);
    }
}

/* Verifica si un paquete ya está instalado */
static bool paquete_instalado(jpm_contexto_t *ctx, const char *nombre, const char *version) {
    char ruta_instalacion[JPM_MAX_RUTA];
    obtener_directorio_instalacion(ctx, nombre, ruta_instalacion, sizeof(ruta_instalacion));
    
    /* Verificar si existe el directorio */
    struct stat st;
    if (stat(ruta_instalacion, &st) != 0) {
        return false;
    }
    
    /* Verificar jasboot.json */
    char ruta_meta[JPM_MAX_RUTA];
    snprintf(ruta_meta, sizeof(ruta_meta), "%s%s%s", 
             ruta_instalacion, PATH_SEP, JPM_ARCHIVO_METADATOS);
    
    jpm_metadatos_t *meta = jpm_metadatos_leer(ruta_meta);
    if (!meta) {
        return false;
    }
    
    /* Comparar versión si se especificó */
    bool resultado = true;
    if (version && strlen(version) > 0) {
        resultado = (strcmp(meta->version, version) == 0);
    }
    
    jpm_metadatos_liberar(meta);
    return resultado;
}

/* Registra un paquete como instalado */
static int registrar_paquete_instalado(jpm_contexto_t *ctx, const char *nombre,
                                       const char *ruta_instalacion,
                                       const char *fuente) {
    /* Crear entrada de paquete instalado */
    jpm_paquete_instalado_t *nuevo = (jpm_paquete_instalado_t*)realloc(
        ctx->paquetes_instalados,
        sizeof(jpm_paquete_instalado_t) * (ctx->num_paquetes_instalados + 1)
    );
    
    if (!nuevo) {
        fprintf(stderr, "[ERROR] Fallo al registrar paquete instalado\n");
        return JPM_ERROR_MEMORIA;
    }
    
    ctx->paquetes_instalados = nuevo;
    jpm_paquete_instalado_t *paq = &ctx->paquetes_instalados[ctx->num_paquetes_instalados];
    
    /* Leer metadatos del paquete instalado */
    char ruta_meta[JPM_MAX_RUTA];
    snprintf(ruta_meta, sizeof(ruta_meta), "%s%s%s",
             ruta_instalacion, PATH_SEP, JPM_ARCHIVO_METADATOS);
    
    jpm_metadatos_t *meta = jpm_metadatos_leer(ruta_meta);
    if (meta) {
        memcpy(&paq->metadatos, meta, sizeof(jpm_metadatos_t));
        jpm_metadatos_liberar(meta);
    }
    
    strncpy(paq->ruta_instalacion, ruta_instalacion, JPM_MAX_RUTA - 1);
    paq->ruta_instalacion[JPM_MAX_RUTA - 1] = '\0';
    
    paq->fecha_instalacion = time(NULL);
    paq->es_global = ctx->config.es_global;
    
    if (fuente) {
        strncpy(paq->instalado_desde, fuente, JPM_MAX_URL - 1);
        paq->instalado_desde[JPM_MAX_URL - 1] = '\0';
    } else {
        paq->instalado_desde[0] = '\0';
    }
    
    ctx->num_paquetes_instalados++;
    
    JPM_LOG_INFO(ctx, "Paquete %s registrado como instalado", nombre);
    return JPM_EXITO;
}

/* Instala dependencias de un paquete */
static int instalar_dependencias_recursivo(jpm_contexto_t *ctx, jpm_metadatos_t *meta,
                                           int profundidad) {
    if (profundidad > 10) {
        fprintf(stderr, "[AVISO] Profundidad máxima de dependencias alcanzada\n");
        return JPM_EXITO;
    }
    
    if (!meta || meta->num_dependencias == 0) {
        return JPM_EXITO;
    }
    
    JPM_LOG_INFO(ctx, "Instalando %zu dependencias (profundidad %d)", 
                 meta->num_dependencias, profundidad);
    
    for (size_t i = 0; i < meta->num_dependencias; i++) {
        jpm_dependencia_t *dep = &meta->dependencias[i];
        
        /* Saltar dependencias opcionales si no es modo forzado */
        if (dep->es_opcional && !ctx->config.forzar) {
            JPM_LOG_INFO(ctx, "Saltando dependencia opcional: %s", dep->nombre);
            continue;
        }
        
        /* Verificar si ya está instalado */
        if (paquete_instalado(ctx, dep->nombre, NULL)) {
            JPM_LOG_INFO(ctx, "Dependencia %s ya instalada", dep->nombre);
            continue;
        }
        
        JPM_LOG_INFO(ctx, "Instalando dependencia: %s %s %s",
                     dep->nombre, dep->operador, dep->version);
        
        /* Por ahora, solo intentamos instalar desde el registro */
        int resultado = jpm_instalar_desde_registro(ctx, dep->nombre, dep->version);
        
        if (resultado != JPM_EXITO && !dep->es_opcional) {
            fprintf(stderr, "[ERROR] Fallo al instalar dependencia: %s\n", dep->nombre);
            return resultado;
        }
    }
    
    return JPM_EXITO;
}

/* =============================================================================
 * FUNCIONES PÚBLICAS
 * =============================================================================
 */

int jpm_instalar_desde_archivo(jpm_contexto_t *ctx, const char *ruta_archivo) {
    if (!ctx || !ruta_archivo) {
        fprintf(stderr, "[ERROR] Parámetros inválidos para instalación\n");
        return JPM_ERROR_VALIDACION;
    }
    
    /* Verificar que el archivo existe */
    struct stat st;
    if (stat(ruta_archivo, &st) != 0) {
        fprintf(stderr, "[ERROR] Archivo no encontrado: %s\n", ruta_archivo);
        return JPM_ERROR_NO_ENCONTRADO;
    }
    
    JPM_LOG_INFO(ctx, "Instalando desde archivo: %s", ruta_archivo);
    
    /* Crear directorio temporal para extracción */
    char dir_temp[JPM_MAX_RUTA];
    snprintf(dir_temp, sizeof(dir_temp), "%s%s%s%stemp_%ld",
             ctx->directorio_trabajo, PATH_SEP,
             JPM_DIRECTORIO_TEMP, PATH_SEP,
             (long)time(NULL));
    
    if (jpm_crear_directorio_recursivo(dir_temp) != JPM_EXITO) {
        fprintf(stderr, "[ERROR] No se pudo crear directorio temporal\n");
        return JPM_ERROR_ARCHIVO;
    }
    
    /* Extraer paquete a directorio temporal */
    JPM_LOG_INFO(ctx, "Extrayendo paquete...");
    int resultado = jpm_extraer(ruta_archivo, dir_temp);
    
    if (resultado != JPM_EXITO) {
        fprintf(stderr, "[ERROR] Fallo al extraer paquete\n");
        jpm_eliminar_directorio(dir_temp);
        return resultado;
    }
    
    /* Leer metadatos del paquete */
    char ruta_meta[JPM_MAX_RUTA];
    snprintf(ruta_meta, sizeof(ruta_meta), "%s%s%s",
             dir_temp, PATH_SEP, JPM_ARCHIVO_METADATOS);
    
    jpm_metadatos_t *meta = jpm_metadatos_leer(ruta_meta);
    if (!meta) {
        fprintf(stderr, "[ERROR] No se pudieron leer metadatos del paquete\n");
        jpm_eliminar_directorio(dir_temp);
        return JPM_ERROR_JSON;
    }
    
    /* Validar metadatos */
    char error[512];
    if (!jpm_metadatos_validar(meta, error, sizeof(error))) {
        fprintf(stderr, "[ERROR] Metadatos inválidos: %s\n", error);
        jpm_metadatos_liberar(meta);
        jpm_eliminar_directorio(dir_temp);
        return JPM_ERROR_VALIDACION;
    }
    
    JPM_LOG_INFO(ctx, "Paquete: %s v%s", meta->nombre, meta->version);
    
    /* Verificar si ya está instalado */
    if (paquete_instalado(ctx, meta->nombre, meta->version) && !ctx->config.forzar) {
        JPM_LOG_INFO(ctx, "El paquete %s v%s ya está instalado", 
                     meta->nombre, meta->version);
        jpm_metadatos_liberar(meta);
        jpm_eliminar_directorio(dir_temp);
        return JPM_EXITO;
    }
    
    /* Verificar integridad si hay hash en metadatos */
    if (strlen(meta->hash_sha512) > 0) {
        JPM_LOG_INFO(ctx, "Verificando integridad...");
        if (!jpm_verificar_integridad(ruta_archivo, meta->hash_sha512)) {
            fprintf(stderr, "[ERROR] Verificación de integridad falló\n");
            jpm_metadatos_liberar(meta);
            jpm_eliminar_directorio(dir_temp);
            return JPM_ERROR_VALIDACION;
        }
    }
    
    /* Obtener directorio de instalación final */
    char dir_instalacion[JPM_MAX_RUTA];
    obtener_directorio_instalacion(ctx, meta->nombre, dir_instalacion, sizeof(dir_instalacion));
    
    /* Si ya existe, eliminarlo primero (reinstalación) */
    if (jpm_archivo_existe(dir_instalacion)) {
        JPM_LOG_INFO(ctx, "Eliminando instalación anterior...");
        jpm_eliminar_directorio(dir_instalacion);
    }
    
    /* Crear directorio padre si no existe */
    char dir_padre[JPM_MAX_RUTA];
    if (ctx->config.es_global) {
        strncpy(dir_padre, ctx->config.directorio_global, JPM_MAX_RUTA - 1);
    } else {
        snprintf(dir_padre, sizeof(dir_padre), "%s%s%s",
                 ctx->directorio_trabajo, PATH_SEP, JPM_DIRECTORIO_DEPENDENCIAS);
    }
    jpm_crear_directorio_recursivo(dir_padre);
    
    /* Mover directorio temporal a ubicación final */
    JPM_LOG_INFO(ctx, "Instalando en: %s", dir_instalacion);
    
    if (ctx->config.simular) {
        JPM_LOG_INFO(ctx, "[DRY-RUN] Se movería de %s a %s", dir_temp, dir_instalacion);
        resultado = JPM_EXITO;
    } else {
        resultado = jpm_copiar_directorio(dir_temp, dir_instalacion);
        if (resultado != JPM_EXITO) {
            fprintf(stderr, "[ERROR] Fallo al copiar archivos a destino\n");
            jpm_metadatos_liberar(meta);
            jpm_eliminar_directorio(dir_temp);
            return resultado;
        }
    }
    
    /* Limpiar directorio temporal */
    jpm_eliminar_directorio(dir_temp);
    
    /* Instalar dependencias */
    if (meta->num_dependencias > 0 && !ctx->config.simular) {
        JPM_LOG_INFO(ctx, "Instalando dependencias...");
        resultado = instalar_dependencias_recursivo(ctx, meta, 0);
        if (resultado != JPM_EXITO) {
            fprintf(stderr, "[AVISO] Algunas dependencias no se instalaron\n");
        }
    }
    
    /* Registrar paquete instalado */
    if (!ctx->config.simular) {
        registrar_paquete_instalado(ctx, meta->nombre, dir_instalacion, ruta_archivo);
    }
    
    /* Actualizar archivo lock */
    if (ctx->archivo_lock && !ctx->config.simular) {
        jpm_lock_entrada_t entrada = {0};
        strncpy(entrada.nombre, meta->nombre, JPM_MAX_NOMBRE - 1);
        strncpy(entrada.version, meta->version, JPM_MAX_VERSION - 1);
        strncpy(entrada.resuelto, ruta_archivo, JPM_MAX_URL - 1);
        strncpy(entrada.hash, meta->hash_sha512, 128);
        
        jpm_lock_agregar_entrada(ctx->archivo_lock, &entrada);
        
        /* Guardar archivo lock */
        char ruta_lock[JPM_MAX_RUTA];
        snprintf(ruta_lock, sizeof(ruta_lock), "%s%s%s",
                 ctx->directorio_trabajo, PATH_SEP, JPM_ARCHIVO_LOCK);
        jpm_lock_escribir(ctx->archivo_lock, ruta_lock);
    }
    
    printf("\n[EXITO] Paquete %s v%s instalado correctamente\n", 
           meta->nombre, meta->version);
    
    jpm_metadatos_liberar(meta);
    return JPM_EXITO;
}

int jpm_cmd_install(jpm_contexto_t *ctx, const char *fuente) {
    if (!ctx || !fuente) {
        fprintf(stderr, "[ERROR] Parámetros inválidos\n");
        return JPM_ERROR_VALIDACION;
    }
    
    /* Determinar tipo de fuente */
    
    /* 1. Si es un archivo local .jpkg */
    if (strstr(fuente, ".jpkg") != NULL && jpm_archivo_existe(fuente)) {
        return jpm_instalar_desde_archivo(ctx, fuente);
    }
    
    /* 2. Si es una URL */
    if (strncmp(fuente, "http://", 7) == 0 || strncmp(fuente, "https://", 8) == 0) {
        JPM_LOG_INFO(ctx, "Descargando desde URL: %s", fuente);
        
        /* Descargar a directorio temporal */
        char archivo_temp[JPM_MAX_RUTA];
        snprintf(archivo_temp, sizeof(archivo_temp), "%s%s%s%spaquete_%ld.jpkg",
                 ctx->directorio_trabajo, PATH_SEP,
                 JPM_DIRECTORIO_TEMP, PATH_SEP,
                 (long)time(NULL));
        
        jpm_crear_directorio_recursivo(ctx->config.directorio_cache);
        
        int resultado = jpm_descargar_archivo(fuente, archivo_temp, NULL, NULL);
        if (resultado != JPM_EXITO) {
            fprintf(stderr, "[ERROR] Fallo al descargar paquete\n");
            return resultado;
        }
        
        resultado = jpm_instalar_desde_archivo(ctx, archivo_temp);
        
        /* Limpiar archivo temporal */
        remove(archivo_temp);
        
        return resultado;
    }
    
    /* 3. Si es un nombre de paquete, buscar en registro */
    JPM_LOG_INFO(ctx, "Buscando en registro: %s", fuente);
    
    /* Extraer nombre y versión si están en formato nombre@version */
    char nombre[JPM_MAX_NOMBRE];
    char version[JPM_MAX_VERSION] = "";
    
    const char *arroba = strchr(fuente, '@');
    if (arroba) {
        size_t len_nombre = arroba - fuente;
        if (len_nombre >= JPM_MAX_NOMBRE) len_nombre = JPM_MAX_NOMBRE - 1;
        
        strncpy(nombre, fuente, len_nombre);
        nombre[len_nombre] = '\0';
        
        strncpy(version, arroba + 1, JPM_MAX_VERSION - 1);
        version[JPM_MAX_VERSION - 1] = '\0';
    } else {
        strncpy(nombre, fuente, JPM_MAX_NOMBRE - 1);
        nombre[JPM_MAX_NOMBRE - 1] = '\0';
    }
    
    return jpm_instalar_desde_registro(ctx, nombre, version[0] ? version : NULL);
}

int jpm_cmd_install_todas(jpm_contexto_t *ctx) {
    if (!ctx) {
        fprintf(stderr, "[ERROR] Contexto inválido\n");
        return JPM_ERROR_VALIDACION;
    }
    
    /* Leer jasboot.json del proyecto actual */
    char ruta_meta[JPM_MAX_RUTA];
    snprintf(ruta_meta, sizeof(ruta_meta), "%s%s%s",
             ctx->directorio_trabajo, PATH_SEP, JPM_ARCHIVO_METADATOS);
    
    if (!jpm_archivo_existe(ruta_meta)) {
        fprintf(stderr, "[ERROR] No se encontró %s en el directorio actual\n",
                JPM_ARCHIVO_METADATOS);
        fprintf(stderr, "Ejecute 'jpm init' primero para crear un proyecto\n");
        return JPM_ERROR_NO_ENCONTRADO;
    }
    
    jpm_metadatos_t *meta = jpm_metadatos_leer(ruta_meta);
    if (!meta) {
        fprintf(stderr, "[ERROR] No se pudo leer %s\n", JPM_ARCHIVO_METADATOS);
        return JPM_ERROR_JSON;
    }
    
    ctx->metadatos_proyecto = meta;
    
    /* Cargar archivo lock si existe */
    char ruta_lock[JPM_MAX_RUTA];
    snprintf(ruta_lock, sizeof(ruta_lock), "%s%s%s",
             ctx->directorio_trabajo, PATH_SEP, JPM_ARCHIVO_LOCK);
    
    if (jpm_archivo_existe(ruta_lock)) {
        ctx->archivo_lock = jpm_lock_leer(ruta_lock);
    } else {
        ctx->archivo_lock = jpm_lock_crear();
    }
    
    if (meta->num_dependencias == 0 && meta->num_dependencias_desarrollo == 0) {
        printf("[INFO] No hay dependencias para instalar\n");
        return JPM_EXITO;
    }
    
    printf("[INFO] Instalando dependencias de %s v%s\n", meta->nombre, meta->version);
    printf("[INFO] Total: %zu dependencias, %zu de desarrollo\n",
           meta->num_dependencias, meta->num_dependencias_desarrollo);
    
    /* Instalar dependencias de producción */
    int resultado = instalar_dependencias_recursivo(ctx, meta, 0);
    
    if (resultado != JPM_EXITO) {
        fprintf(stderr, "[ERROR] Fallo al instalar algunas dependencias\n");
        return resultado;
    }
    
    /* Instalar dependencias de desarrollo si no estamos en producción */
    if (meta->num_dependencias_desarrollo > 0 && !ctx->config.simular) {
        JPM_LOG_INFO(ctx, "Instalando %zu dependencias de desarrollo",
                     meta->num_dependencias_desarrollo);
        
        for (size_t i = 0; i < meta->num_dependencias_desarrollo; i++) {
            jpm_dependencia_t *dep = &meta->dependencias_desarrollo[i];
            
            if (paquete_instalado(ctx, dep->nombre, NULL)) {
                JPM_LOG_INFO(ctx, "Dependencia de desarrollo %s ya instalada", dep->nombre);
                continue;
            }
            
            JPM_LOG_INFO(ctx, "Instalando dependencia de desarrollo: %s", dep->nombre);
            jpm_instalar_desde_registro(ctx, dep->nombre, dep->version);
        }
    }
    
    printf("\n[EXITO] Todas las dependencias instaladas\n");
    return JPM_EXITO;
}

int jpm_cmd_pack(jpm_contexto_t *ctx, const char *directorio) {
    if (!ctx) {
        fprintf(stderr, "[ERROR] Contexto inválido\n");
        return JPM_ERROR_VALIDACION;
    }
    
    const char *dir_empaquetar = directorio ? directorio : ctx->directorio_trabajo;
    
    /* Verificar que existe jasboot.json */
    char ruta_meta[JPM_MAX_RUTA];
    snprintf(ruta_meta, sizeof(ruta_meta), "%s%s%s",
             dir_empaquetar, PATH_SEP, JPM_ARCHIVO_METADATOS);
    
    if (!jpm_archivo_existe(ruta_meta)) {
        fprintf(stderr, "[ERROR] No se encontró %s en %s\n",
                JPM_ARCHIVO_METADATOS, dir_empaquetar);
        return JPM_ERROR_NO_ENCONTRADO;
    }
    
    /* Leer metadatos para obtener nombre y versión */
    jpm_metadatos_t *meta = jpm_metadatos_leer(ruta_meta);
    if (!meta) {
        fprintf(stderr, "[ERROR] No se pudo leer %s\n", JPM_ARCHIVO_METADATOS);
        return JPM_ERROR_JSON;
    }
    
    /* Validar metadatos */
    char error[512];
    if (!jpm_metadatos_validar(meta, error, sizeof(error))) {
        fprintf(stderr, "[ERROR] Metadatos inválidos: %s\n", error);
        jpm_metadatos_liberar(meta);
        return JPM_ERROR_VALIDACION;
    }
    
    /* Generar nombre de archivo */
    char archivo_salida[JPM_MAX_RUTA];
    snprintf(archivo_salida, sizeof(archivo_salida), "%s-%s.jpkg",
             meta->nombre, meta->version);
    
    JPM_LOG_INFO(ctx, "Empaquetando %s v%s", meta->nombre, meta->version);
    
    if (ctx->config.simular) {
        printf("[DRY-RUN] Se crearía el archivo: %s\n", archivo_salida);
        jpm_metadatos_liberar(meta);
        return JPM_EXITO;
    }
    
    /* Empaquetar */
    int resultado = jpm_empaquetar(dir_empaquetar, archivo_salida);
    
    if (resultado != JPM_EXITO) {
        fprintf(stderr, "[ERROR] Fallo al empaquetar\n");
        jpm_metadatos_liberar(meta);
        return resultado;
    }
    
    /* Calcular hash del paquete */
    char hash[129];
    if (jpm_calcular_hash(archivo_salida, hash, sizeof(hash)) == JPM_EXITO) {
        printf("[INFO] SHA-512: %s\n", hash);
        
        /* Actualizar metadatos con el hash */
        strncpy(meta->hash_sha512, hash, 128);
        meta->hash_sha512[128] = '\0';
        
        jpm_metadatos_escribir(meta, ruta_meta);
    }
    
    /* Información del archivo generado */
    size_t tamano = jpm_obtener_tamano_archivo(archivo_salida);
    char tamano_str[64];
    jpm_formatear_bytes(tamano, tamano_str, sizeof(tamano_str));
    
    printf("\n[EXITO] Paquete creado: %s (%s)\n", archivo_salida, tamano_str);
    
    jpm_metadatos_liberar(meta);
    return JPM_EXITO;
}