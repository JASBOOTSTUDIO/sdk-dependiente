/*
 * jpm_metadata.c
 * Gestión de metadatos jasboot.json para Jasboot Package Manager
 * 
 * Funciones para crear, leer, escribir y validar metadatos de paquetes
 */

#include "../include/jpm.h"
#include <string.h>
#include <ctype.h>
#include <errno.h>

/* =============================================================================
 * FUNCIONES AUXILIARES INTERNAS
 * =============================================================================
 */

/* Copia string de forma segura */
static void copiar_string_seguro(char *destino, const char *origen, size_t max_len) {
    if (!destino || !origen) {
        return;
    }
    strncpy(destino, origen, max_len - 1);
    destino[max_len - 1] = '\0';
}

/* Parsea array de strings desde JSON */
static char** parsear_array_strings(void *array_json, size_t *num_elementos) {
    if (!array_json || !num_elementos) {
        return NULL;
    }
    
    /* El array JSON es un json_valor_t interno - necesitamos contar elementos */
    /* Por ahora, retornamos NULL - esto se expandirá cuando necesitemos palabras clave */
    *num_elementos = 0;
    return NULL;
}

/* Parsea dependencias desde objeto JSON */
static int parsear_dependencias(void *deps_json, jpm_dependencia_t **deps_out, 
                                size_t *num_deps) {
    if (!deps_json || !deps_out || !num_deps) {
        return JPM_ERROR_VALIDACION;
    }
    
    *deps_out = NULL;
    *num_deps = 0;
    
    /* Por ahora retornamos éxito sin dependencias */
    /* TODO: Implementar parsing completo de dependencias */
    return JPM_EXITO;
}

/* Convierte string a tipo de paquete */
static jpm_tipo_paquete_t string_a_tipo_paquete(const char *tipo_str) {
    if (!tipo_str) {
        return JPM_TIPO_BIBLIOTECA;
    }
    
    if (strcmp(tipo_str, "aplicacion") == 0 || strcmp(tipo_str, "aplicación") == 0) {
        return JPM_TIPO_APLICACION;
    } else if (strcmp(tipo_str, "plugin") == 0) {
        return JPM_TIPO_PLUGIN;
    } else if (strcmp(tipo_str, "herramienta") == 0) {
        return JPM_TIPO_HERRAMIENTA;
    }
    
    return JPM_TIPO_BIBLIOTECA;
}

/* Convierte tipo de paquete a string */
static const char* tipo_paquete_a_string(jpm_tipo_paquete_t tipo) {
    switch (tipo) {
        case JPM_TIPO_APLICACION: return "aplicacion";
        case JPM_TIPO_PLUGIN: return "plugin";
        case JPM_TIPO_HERRAMIENTA: return "herramienta";
        default: return "biblioteca";
    }
}

/* =============================================================================
 * API PÚBLICA - CREACIÓN Y LIBERACIÓN
 * =============================================================================
 */

jpm_metadatos_t* jpm_metadatos_crear(const char *nombre, const char *version) {
    if (!nombre || !version) {
        fprintf(stderr, "[ERROR JPM] Nombre y versión requeridos para crear metadatos\n");
        return NULL;
    }
    
    /* Validar nombre y versión */
    if (!jpm_validar_nombre(nombre)) {
        fprintf(stderr, "[ERROR JPM] Nombre de paquete inválido: %s\n", nombre);
        return NULL;
    }
    
    if (!jpm_validar_version(version)) {
        fprintf(stderr, "[ERROR JPM] Versión inválida: %s\n", version);
        return NULL;
    }
    
    jpm_metadatos_t *meta = (jpm_metadatos_t*)calloc(1, sizeof(jpm_metadatos_t));
    if (!meta) {
        fprintf(stderr, "[ERROR JPM] Error de memoria al crear metadatos\n");
        return NULL;
    }
    
    /* Copiar valores obligatorios */
    copiar_string_seguro(meta->nombre, nombre, JPM_MAX_NOMBRE);
    copiar_string_seguro(meta->version, version, JPM_MAX_VERSION);
    
    /* Valores por defecto */
    copiar_string_seguro(meta->descripcion, "Paquete Jasboot", JPM_MAX_DESCRIPCION);
    copiar_string_seguro(meta->autor, "Autor Desconocido", JPM_MAX_NOMBRE);
    copiar_string_seguro(meta->licencia, "MIT", JPM_MAX_NOMBRE);
    copiar_string_seguro(meta->principal, "src/main.jasb", JPM_MAX_RUTA);
    copiar_string_seguro(meta->jasboot_version, "^0.1.0", JPM_MAX_VERSION);
    
    meta->tipo = JPM_TIPO_BIBLIOTECA;
    meta->creado_en = time(NULL);
    meta->actualizado_en = meta->creado_en;
    
    meta->palabras_clave = NULL;
    meta->num_palabras_clave = 0;
    meta->dependencias = NULL;
    meta->num_dependencias = 0;
    meta->dependencias_desarrollo = NULL;
    meta->num_dependencias_desarrollo = 0;
    meta->scripts = NULL;
    meta->num_scripts = 0;
    
    return meta;
}

void jpm_metadatos_liberar(jpm_metadatos_t *meta) {
    if (!meta) {
        return;
    }
    
    /* Liberar palabras clave */
    if (meta->palabras_clave) {
        for (size_t i = 0; i < meta->num_palabras_clave; i++) {
            free(meta->palabras_clave[i]);
        }
        free(meta->palabras_clave);
    }
    
    /* Liberar dependencias */
    free(meta->dependencias);
    free(meta->dependencias_desarrollo);
    
    /* Liberar scripts */
    free(meta->scripts);
    
    /* Liberar estructura */
    free(meta);
}

/* =============================================================================
 * API PÚBLICA - LECTURA Y ESCRITURA DE ARCHIVOS
 * =============================================================================
 */

jpm_metadatos_t* jpm_metadatos_leer(const char *ruta_archivo) {
    if (!ruta_archivo) {
        fprintf(stderr, "[ERROR JPM] Ruta de archivo NULL en metadatos_leer\n");
        return NULL;
    }
    
    /* Leer y parsear JSON */
    void *json = jpm_json_leer_archivo(ruta_archivo);
    if (!json) {
        fprintf(stderr, "[ERROR JPM] No se pudo leer archivo JSON: %s\n", ruta_archivo);
        return NULL;
    }
    
    /* Obtener campos obligatorios */
    const char *nombre = jpm_json_obtener_string(json, "nombre");
    const char *version = jpm_json_obtener_string(json, "version");
    
    if (!nombre || !version) {
        fprintf(stderr, "[ERROR JPM] Faltan campos obligatorios 'nombre' y/o 'version'\n");
        jpm_json_liberar(json);
        return NULL;
    }
    
    /* Crear metadatos base */
    jpm_metadatos_t *meta = jpm_metadatos_crear(nombre, version);
    if (!meta) {
        jpm_json_liberar(json);
        return NULL;
    }
    
    /* Campos opcionales */
    const char *descripcion = jpm_json_obtener_string(json, "descripcion");
    if (descripcion) {
        copiar_string_seguro(meta->descripcion, descripcion, JPM_MAX_DESCRIPCION);
    }
    
    const char *autor = jpm_json_obtener_string(json, "autor");
    if (autor) {
        copiar_string_seguro(meta->autor, autor, JPM_MAX_NOMBRE);
    }
    
    const char *licencia = jpm_json_obtener_string(json, "licencia");
    if (licencia) {
        copiar_string_seguro(meta->licencia, licencia, JPM_MAX_NOMBRE);
    }
    
    const char *principal = jpm_json_obtener_string(json, "principal");
    if (principal) {
        copiar_string_seguro(meta->principal, principal, JPM_MAX_RUTA);
    }
    
    const char *jasboot_version = jpm_json_obtener_string(json, "jasboot");
    if (jasboot_version) {
        copiar_string_seguro(meta->jasboot_version, jasboot_version, JPM_MAX_VERSION);
    }
    
    const char *repositorio = jpm_json_obtener_string(json, "repositorio");
    if (repositorio) {
        copiar_string_seguro(meta->repositorio, repositorio, JPM_MAX_URL);
    }
    
    const char *homepage = jpm_json_obtener_string(json, "homepage");
    if (homepage) {
        copiar_string_seguro(meta->homepage, homepage, JPM_MAX_URL);
    }
    
    const char *tipo_str = jpm_json_obtener_string(json, "tipo");
    if (tipo_str) {
        meta->tipo = string_a_tipo_paquete(tipo_str);
    }
    
    /* Parsear dependencias */
    void *deps = jpm_json_obtener_objeto(json, "dependencias");
    if (deps) {
        parsear_dependencias(deps, &meta->dependencias, &meta->num_dependencias);
    }
    
    void *deps_dev = jpm_json_obtener_objeto(json, "dependenciasDev");
    if (deps_dev) {
        parsear_dependencias(deps_dev, &meta->dependencias_desarrollo, 
                           &meta->num_dependencias_desarrollo);
    }
    
    /* Palabras clave */
    void *palabras = jpm_json_obtener_array(json, "palabrasClave");
    if (palabras) {
        meta->palabras_clave = parsear_array_strings(palabras, &meta->num_palabras_clave);
    }
    
    jpm_json_liberar(json);
    return meta;
}

int jpm_metadatos_escribir(const jpm_metadatos_t *meta, const char *ruta_archivo) {
    if (!meta || !ruta_archivo) {
        fprintf(stderr, "[ERROR JPM] Parámetros NULL en metadatos_escribir\n");
        return JPM_ERROR_VALIDACION;
    }
    
    /* Validar metadatos antes de escribir */
    char error[512];
    if (!jpm_metadatos_validar(meta, error, sizeof(error))) {
        fprintf(stderr, "[ERROR JPM] Metadatos inválidos: %s\n", error);
        return JPM_ERROR_VALIDACION;
    }
    
    /* Abrir archivo para escritura */
    FILE *f = fopen(ruta_archivo, "w");
    if (!f) {
        fprintf(stderr, "[ERROR JPM] No se pudo crear archivo: %s - %s\n", 
                ruta_archivo, strerror(errno));
        return JPM_ERROR_ARCHIVO;
    }
    
    /* Escribir JSON manualmente (formato bonito) */
    fprintf(f, "{\n");
    fprintf(f, "  \"nombre\": \"%s\",\n", meta->nombre);
    fprintf(f, "  \"version\": \"%s\",\n", meta->version);
    fprintf(f, "  \"descripcion\": \"%s\",\n", meta->descripcion);
    fprintf(f, "  \"autor\": \"%s\",\n", meta->autor);
    fprintf(f, "  \"licencia\": \"%s\",\n", meta->licencia);
    fprintf(f, "  \"principal\": \"%s\",\n", meta->principal);
    fprintf(f, "  \"jasboot\": \"%s\",\n", meta->jasboot_version);
    fprintf(f, "  \"tipo\": \"%s\"", tipo_paquete_a_string(meta->tipo));
    
    /* Repositorio y homepage (opcionales) */
    if (meta->repositorio[0] != '\0') {
        fprintf(f, ",\n  \"repositorio\": \"%s\"", meta->repositorio);
    }
    if (meta->homepage[0] != '\0') {
        fprintf(f, ",\n  \"homepage\": \"%s\"", meta->homepage);
    }
    
    /* Palabras clave */
    if (meta->num_palabras_clave > 0 && meta->palabras_clave) {
        fprintf(f, ",\n  \"palabrasClave\": [");
        for (size_t i = 0; i < meta->num_palabras_clave; i++) {
            fprintf(f, "%s\"%s\"", i > 0 ? ", " : "", meta->palabras_clave[i]);
        }
        fprintf(f, "]");
    }
    
    /* Dependencias */
    if (meta->num_dependencias > 0 && meta->dependencias) {
        fprintf(f, ",\n  \"dependencias\": {\n");
        for (size_t i = 0; i < meta->num_dependencias; i++) {
            fprintf(f, "    \"%s\": \"%s%s\"",
                    meta->dependencias[i].nombre,
                    meta->dependencias[i].operador,
                    meta->dependencias[i].version);
            if (i < meta->num_dependencias - 1) {
                fprintf(f, ",");
            }
            fprintf(f, "\n");
        }
        fprintf(f, "  }");
    }
    
    /* Dependencias de desarrollo */
    if (meta->num_dependencias_desarrollo > 0 && meta->dependencias_desarrollo) {
        fprintf(f, ",\n  \"dependenciasDev\": {\n");
        for (size_t i = 0; i < meta->num_dependencias_desarrollo; i++) {
            fprintf(f, "    \"%s\": \"%s%s\"",
                    meta->dependencias_desarrollo[i].nombre,
                    meta->dependencias_desarrollo[i].operador,
                    meta->dependencias_desarrollo[i].version);
            if (i < meta->num_dependencias_desarrollo - 1) {
                fprintf(f, ",");
            }
            fprintf(f, "\n");
        }
        fprintf(f, "  }");
    }
    
    /* Scripts */
    if (meta->num_scripts > 0 && meta->scripts) {
        fprintf(f, ",\n  \"scripts\": {\n");
        for (size_t i = 0; i < meta->num_scripts; i++) {
            fprintf(f, "    \"%s\": \"%s\"",
                    meta->scripts[i].nombre,
                    meta->scripts[i].comando);
            if (i < meta->num_scripts - 1) {
                fprintf(f, ",");
            }
            fprintf(f, "\n");
        }
        fprintf(f, "  }");
    }
    
    fprintf(f, "\n}\n");
    
    fclose(f);
    return JPM_EXITO;
}

/* =============================================================================
 * API PÚBLICA - VALIDACIÓN
 * =============================================================================
 */

bool jpm_metadatos_validar(const jpm_metadatos_t *meta, char *error, size_t error_len) {
    if (!meta) {
        if (error) {
            snprintf(error, error_len, "Metadatos NULL");
        }
        return false;
    }
    
    /* Validar nombre */
    if (meta->nombre[0] == '\0') {
        if (error) {
            snprintf(error, error_len, "Nombre de paquete vacío");
        }
        return false;
    }
    
    if (!jpm_validar_nombre(meta->nombre)) {
        if (error) {
            snprintf(error, error_len, "Nombre de paquete inválido: %s", meta->nombre);
        }
        return false;
    }
    
    /* Validar versión */
    if (meta->version[0] == '\0') {
        if (error) {
            snprintf(error, error_len, "Versión vacía");
        }
        return false;
    }
    
    if (!jpm_validar_version(meta->version)) {
        if (error) {
            snprintf(error, error_len, "Versión inválida: %s", meta->version);
        }
        return false;
    }
    
    /* Validar jasboot version */
    if (meta->jasboot_version[0] != '\0') {
        /* Remover operador (^, ~, >=, etc) si existe */
        const char *version_sin_op = meta->jasboot_version;
        while (*version_sin_op && !isdigit(*version_sin_op)) {
            version_sin_op++;
        }
        
        if (*version_sin_op && !jpm_validar_version(version_sin_op)) {
            if (error) {
                snprintf(error, error_len, "Versión Jasboot inválida: %s", 
                        meta->jasboot_version);
            }
            return false;
        }
    }
    
    /* Validar archivo principal existe (si hay ruta) */
    if (meta->principal[0] != '\0') {
        /* Solo verificar que no sea demasiado largo */
        if (strlen(meta->principal) >= JPM_MAX_RUTA) {
            if (error) {
                snprintf(error, error_len, "Ruta de archivo principal demasiado larga");
            }
            return false;
        }
    }
    
    /* Validar dependencias */
    for (size_t i = 0; i < meta->num_dependencias; i++) {
        if (!jpm_validar_nombre(meta->dependencias[i].nombre)) {
            if (error) {
                snprintf(error, error_len, "Nombre de dependencia inválido: %s", 
                        meta->dependencias[i].nombre);
            }
            return false;
        }
    }
    
    return true;
}

/* =============================================================================
 * API PÚBLICA - MODIFICACIÓN
 * =============================================================================
 */

int jpm_metadatos_agregar_dependencia(jpm_metadatos_t *meta, const char *nombre, 
                                      const char *version, bool es_desarrollo) {
    if (!meta || !nombre || !version) {
        fprintf(stderr, "[ERROR JPM] Parámetros NULL en agregar_dependencia\n");
        return JPM_ERROR_VALIDACION;
    }
    
    /* Validar nombre y versión */
    if (!jpm_validar_nombre(nombre)) {
        fprintf(stderr, "[ERROR JPM] Nombre de dependencia inválido: %s\n", nombre);
        return JPM_ERROR_VALIDACION;
    }
    
    /* Determinar operador y versión real */
    const char *operador = "^";
    const char *version_real = version;
    
    /* Extraer operador si está presente */
    if (version[0] == '^' || version[0] == '~' || version[0] == '=' || 
        version[0] == '>' || version[0] == '<') {
        
        if (version[0] == '>' && version[1] == '=') {
            operador = ">=";
            version_real = version + 2;
        } else if (version[0] == '<' && version[1] == '=') {
            operador = "<=";
            version_real = version + 2;
        } else {
            char op[2] = {version[0], '\0'};
            operador = strdup(op);
            version_real = version + 1;
        }
    }
    
    /* Validar versión */
    if (!jpm_validar_version(version_real)) {
        fprintf(stderr, "[ERROR JPM] Versión de dependencia inválida: %s\n", version_real);
        return JPM_ERROR_VALIDACION;
    }
    
    /* Seleccionar array de dependencias */
    jpm_dependencia_t **deps = es_desarrollo ? 
        &meta->dependencias_desarrollo : &meta->dependencias;
    size_t *num_deps = es_desarrollo ? 
        &meta->num_dependencias_desarrollo : &meta->num_dependencias;
    
    /* Verificar si ya existe */
    for (size_t i = 0; i < *num_deps; i++) {
        if (strcmp((*deps)[i].nombre, nombre) == 0) {
            /* Actualizar versión existente */
            copiar_string_seguro((*deps)[i].version, version_real, JPM_MAX_VERSION);
            copiar_string_seguro((*deps)[i].operador, operador, sizeof((*deps)[i].operador));
            fprintf(stderr, "[INFO JPM] Dependencia actualizada: %s@%s%s\n", 
                    nombre, operador, version_real);
            return JPM_EXITO;
        }
    }
    
    /* Verificar límite */
    if (*num_deps >= JPM_MAX_DEPENDENCIAS) {
        fprintf(stderr, "[ERROR JPM] Se alcanzó el límite máximo de dependencias\n");
        return JPM_ERROR_VALIDACION;
    }
    
    /* Expandir array */
    size_t nueva_capacidad = *num_deps + 1;
    jpm_dependencia_t *nuevo = (jpm_dependencia_t*)realloc(*deps, 
                                nueva_capacidad * sizeof(jpm_dependencia_t));
    if (!nuevo) {
        fprintf(stderr, "[ERROR JPM] Error de memoria al agregar dependencia\n");
        return JPM_ERROR_MEMORIA;
    }
    
    *deps = nuevo;
    
    /* Agregar nueva dependencia */
    jpm_dependencia_t *dep = &(*deps)[*num_deps];
    memset(dep, 0, sizeof(jpm_dependencia_t));
    
    copiar_string_seguro(dep->nombre, nombre, JPM_MAX_NOMBRE);
    copiar_string_seguro(dep->version, version_real, JPM_MAX_VERSION);
    copiar_string_seguro(dep->operador, operador, sizeof(dep->operador));
    dep->es_desarrollo = es_desarrollo;
    dep->es_opcional = false;
    
    (*num_deps)++;
    
    fprintf(stderr, "[INFO JPM] Dependencia agregada: %s@%s%s\n", 
            nombre, operador, version_real);
    
    return JPM_EXITO;
}