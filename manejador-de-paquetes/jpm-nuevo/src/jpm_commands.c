/*
 * jpm_commands.c
 * Implementación de comandos CLI para Jasboot Package Manager
 * 
 * Comandos: init, list, info, clean, uninstall
 */

#include "../include/jpm.h"
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#define rmdir _rmdir
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#else
#include <unistd.h>
#endif

/* =============================================================================
 * FUNCIONES AUXILIARES INTERNAS
 * =============================================================================
 */

/* Crea archivo con contenido */
static int crear_archivo_con_contenido(const char *ruta, const char *contenido) {
    if (!ruta || !contenido) {
        return JPM_ERROR_VALIDACION;
    }
    
    FILE *f = fopen(ruta, "w");
    if (!f) {
        fprintf(stderr, "[ERROR JPM] No se pudo crear archivo: %s - %s\n", 
                ruta, strerror(errno));
        return JPM_ERROR_ARCHIVO;
    }
    
    fprintf(f, "%s", contenido);
    fclose(f);
    
    return JPM_EXITO;
}

/* Verifica si archivo existe */
static bool archivo_existe(const char *ruta) {
    if (!ruta) {
        return false;
    }
    struct stat st;
    return stat(ruta, &st) == 0;
}

/* Verifica si es directorio */
static bool es_directorio(const char *ruta) {
    if (!ruta) {
        return false;
    }
    struct stat st;
    if (stat(ruta, &st) != 0) {
        return false;
    }
    return S_ISDIR(st.st_mode);
}

/* Obtiene tamaño de archivo */
static size_t obtener_tamano_archivo(const char *ruta) {
    if (!ruta) {
        return 0;
    }
    struct stat st;
    if (stat(ruta, &st) != 0) {
        return 0;
    }
    return st.st_size;
}

/* Formatea bytes a string legible */
static void formatear_bytes(size_t bytes, char *buffer, size_t buffer_len) {
    if (!buffer || buffer_len == 0) {
        return;
    }
    
    if (bytes < 1024) {
        snprintf(buffer, buffer_len, "%zu B", bytes);
    } else if (bytes < 1024 * 1024) {
        snprintf(buffer, buffer_len, "%.2f KB", bytes / 1024.0);
    } else if (bytes < 1024 * 1024 * 1024) {
        snprintf(buffer, buffer_len, "%.2f MB", bytes / (1024.0 * 1024.0));
    } else {
        snprintf(buffer, buffer_len, "%.2f GB", bytes / (1024.0 * 1024.0 * 1024.0));
    }
}

/* Elimina archivo */
static int eliminar_archivo(const char *ruta) {
    if (!ruta) {
        return JPM_ERROR_VALIDACION;
    }
    
    if (remove(ruta) != 0) {
        fprintf(stderr, "[ERROR JPM] No se pudo eliminar archivo: %s - %s\n", 
                ruta, strerror(errno));
        return JPM_ERROR_ARCHIVO;
    }
    
    return JPM_EXITO;
}

/* Elimina directorio recursivamente */
static int eliminar_directorio_recursivo(const char *ruta) {
    if (!ruta) {
        return JPM_ERROR_VALIDACION;
    }
    
    if (!es_directorio(ruta)) {
        return eliminar_archivo(ruta);
    }
    
    DIR *dir = opendir(ruta);
    if (!dir) {
        fprintf(stderr, "[ERROR JPM] No se pudo abrir directorio: %s - %s\n", 
                ruta, strerror(errno));
        return JPM_ERROR_ARCHIVO;
    }
    
    struct dirent *entrada;
    char ruta_completa[JPM_MAX_RUTA];
    
    while ((entrada = readdir(dir)) != NULL) {
        /* Saltar . y .. */
        if (strcmp(entrada->d_name, ".") == 0 || strcmp(entrada->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(ruta_completa, sizeof(ruta_completa), "%s/%s", ruta, entrada->d_name);
        
        if (es_directorio(ruta_completa)) {
            eliminar_directorio_recursivo(ruta_completa);
        } else {
            eliminar_archivo(ruta_completa);
        }
    }
    
    closedir(dir);
    
    if (rmdir(ruta) != 0) {
        fprintf(stderr, "[ERROR JPM] No se pudo eliminar directorio: %s - %s\n", 
                ruta, strerror(errno));
        return JPM_ERROR_ARCHIVO;
    }
    
    return JPM_EXITO;
}

/* Calcula tamaño de directorio recursivamente */
static size_t calcular_tamano_directorio(const char *ruta) {
    if (!ruta || !es_directorio(ruta)) {
        return 0;
    }
    
    size_t total = 0;
    DIR *dir = opendir(ruta);
    if (!dir) {
        return 0;
    }
    
    struct dirent *entrada;
    char ruta_completa[JPM_MAX_RUTA];
    
    while ((entrada = readdir(dir)) != NULL) {
        if (strcmp(entrada->d_name, ".") == 0 || strcmp(entrada->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(ruta_completa, sizeof(ruta_completa), "%s/%s", ruta, entrada->d_name);
        
        if (es_directorio(ruta_completa)) {
            total += calcular_tamano_directorio(ruta_completa);
        } else {
            total += obtener_tamano_archivo(ruta_completa);
        }
    }
    
    closedir(dir);
    return total;
}

/* =============================================================================
 * COMANDO: jpm init
 * Inicializa nuevo paquete con estructura completa
 * =============================================================================
 */

int jpm_cmd_init(jpm_contexto_t *ctx, const char *nombre) {
    if (!ctx) {
        fprintf(stderr, "[ERROR JPM] Contexto NULL en cmd_init\n");
        return JPM_ERROR_VALIDACION;
    }
    
    if (!nombre || nombre[0] == '\0') {
        fprintf(stderr, "[ERROR JPM] Nombre de paquete requerido\n");
        fprintf(stderr, "Uso: jpm init <nombre>\n");
        return JPM_ERROR_VALIDACION;
    }
    
    /* Validar nombre */
    if (!jpm_validar_nombre(nombre)) {
        fprintf(stderr, "[ERROR JPM] Nombre de paquete inválido: %s\n", nombre);
        fprintf(stderr, "El nombre debe:\n");
        fprintf(stderr, "  - Contener solo letras, números, guiones y guiones bajos\n");
        fprintf(stderr, "  - Comenzar con letra o número\n");
        fprintf(stderr, "  - Tener entre 1 y 255 caracteres\n");
        return JPM_ERROR_VALIDACION;
    }
    
    jpm_log(ctx, JPM_LOG_INFO, "Inicializando paquete '%s'...\n", nombre);
    
    /* Verificar si ya existe jasboot.json */
    char ruta_json[JPM_MAX_RUTA];
    snprintf(ruta_json, sizeof(ruta_json), "%s", JPM_ARCHIVO_METADATOS);
    
    if (archivo_existe(ruta_json)) {
        fprintf(stderr, "[ERROR JPM] Ya existe un archivo %s en este directorio\n", 
                JPM_ARCHIVO_METADATOS);
        return JPM_ERROR_YA_EXISTE;
    }
    
    /* Crear metadatos */
    jpm_metadatos_t *meta = jpm_metadatos_crear(nombre, "0.1.0");
    if (!meta) {
        fprintf(stderr, "[ERROR JPM] No se pudieron crear metadatos\n");
        return JPM_ERROR_GENERICO;
    }
    
    /* Crear estructura de directorios */
    jpm_log(ctx, JPM_LOG_INFO, "Creando estructura de directorios...\n");
    
    if (mkdir("src", 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "[AVISO JPM] No se pudo crear directorio 'src': %s\n", 
                strerror(errno));
    }
    
    if (mkdir("tests", 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "[AVISO JPM] No se pudo crear directorio 'tests': %s\n", 
                strerror(errno));
    }
    
    if (mkdir("docs", 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "[AVISO JPM] No se pudo crear directorio 'docs': %s\n", 
                strerror(errno));
    }
    
    /* Crear archivo principal */
    jpm_log(ctx, JPM_LOG_INFO, "Creando archivo principal...\n");
    
    char contenido_main[1024];
    snprintf(contenido_main, sizeof(contenido_main),
        "# %s\n"
        "# Punto de entrada principal\n\n"
        "funcion principal() retorna entero {\n"
        "    imprimir(\"Hola desde %s\")\n"
        "    retornar 0\n"
        "}\n",
        nombre, nombre);
    
    if (crear_archivo_con_contenido("src/main.jasb", contenido_main) != JPM_EXITO) {
        jpm_metadatos_liberar(meta);
        return JPM_ERROR_ARCHIVO;
    }
    
    /* Crear README.md */
    jpm_log(ctx, JPM_LOG_INFO, "Creando README.md...\n");
    
    char contenido_readme[2048];
    snprintf(contenido_readme, sizeof(contenido_readme),
        "# %s\n\n"
        "Paquete Jasboot\n\n"
        "## Instalación\n\n"
        "```bash\n"
        "jpm install %s\n"
        "```\n\n"
        "## Uso\n\n"
        "```jasboot\n"
        "# Importar el paquete\n"
        "importar \"%s\"\n"
        "```\n\n"
        "## Licencia\n\n"
        "MIT\n",
        nombre, nombre, nombre);
    
    if (crear_archivo_con_contenido("README.md", contenido_readme) != JPM_EXITO) {
        jpm_metadatos_liberar(meta);
        return JPM_ERROR_ARCHIVO;
    }
    
    /* Crear .gitignore */
    jpm_log(ctx, JPM_LOG_INFO, "Creando .gitignore...\n");
    
    const char *contenido_gitignore = 
        "# Dependencias\n"
        "a-modulos/\n\n"
        "# Cache de JPM\n"
        ".jpm-cache/\n"
        ".jpm-temp/\n\n"
        "# Archivos compilados\n"
        "*.jbo\n"
        "*.jpkg\n\n"
        "# Archivos de memoria neuronal\n"
        "*.jmn\n\n"
        "# Sistema\n"
        ".DS_Store\n"
        "Thumbs.db\n";
    
    if (crear_archivo_con_contenido(".gitignore", contenido_gitignore) != JPM_EXITO) {
        jpm_metadatos_liberar(meta);
        return JPM_ERROR_ARCHIVO;
    }
    
    /* Crear .jpmignore */
    jpm_log(ctx, JPM_LOG_INFO, "Creando .jpmignore...\n");
    
    const char *contenido_jpmignore = 
        "# Archivos y directorios a ignorar al empaquetar\n"
        "a-modulos/\n"
        ".jpm-cache/\n"
        ".jpm-temp/\n"
        "tests/\n"
        "*.jbo\n"
        "*.jmn\n"
        ".git/\n"
        ".gitignore\n";
    
    if (crear_archivo_con_contenido(".jpmignore", contenido_jpmignore) != JPM_EXITO) {
        jpm_metadatos_liberar(meta);
        return JPM_ERROR_ARCHIVO;
    }
    
    /* Escribir jasboot.json */
    jpm_log(ctx, JPM_LOG_INFO, "Creando %s...\n", JPM_ARCHIVO_METADATOS);
    
    int resultado = jpm_metadatos_escribir(meta, ruta_json);
    jpm_metadatos_liberar(meta);
    
    if (resultado != JPM_EXITO) {
        fprintf(stderr, "[ERROR JPM] No se pudo escribir %s\n", JPM_ARCHIVO_METADATOS);
        return resultado;
    }
    
    /* Mensaje de éxito */
    jpm_log(ctx, JPM_LOG_INFO, "\n✓ Paquete '%s' inicializado correctamente\n\n", nombre);
    jpm_log(ctx, JPM_LOG_INFO, "Estructura creada:\n");
    jpm_log(ctx, JPM_LOG_INFO, "  %s\n", JPM_ARCHIVO_METADATOS);
    jpm_log(ctx, JPM_LOG_INFO, "  README.md\n");
    jpm_log(ctx, JPM_LOG_INFO, "  .gitignore\n");
    jpm_log(ctx, JPM_LOG_INFO, "  .jpmignore\n");
    jpm_log(ctx, JPM_LOG_INFO, "  src/main.jasb\n");
    jpm_log(ctx, JPM_LOG_INFO, "  tests/\n");
    jpm_log(ctx, JPM_LOG_INFO, "  docs/\n\n");
    jpm_log(ctx, JPM_LOG_INFO, "Próximos pasos:\n");
    jpm_log(ctx, JPM_LOG_INFO, "  1. Edita src/main.jasb con tu código\n");
    jpm_log(ctx, JPM_LOG_INFO, "  2. Actualiza README.md con documentación\n");
    jpm_log(ctx, JPM_LOG_INFO, "  3. Ejecuta 'jpm install' para instalar dependencias\n");
    jpm_log(ctx, JPM_LOG_INFO, "  4. Ejecuta 'jpm pack' para empaquetar tu código\n\n");
    
    return JPM_EXITO;
}

/* =============================================================================
 * COMANDO: jpm list
 * Lista paquetes instalados en a-modulos/
 * =============================================================================
 */

int jpm_cmd_list(jpm_contexto_t *ctx) {
    if (!ctx) {
        fprintf(stderr, "[ERROR JPM] Contexto NULL en cmd_list\n");
        return JPM_ERROR_VALIDACION;
    }
    
    jpm_log(ctx, JPM_LOG_INFO, "Paquetes instalados en %s:\n\n", 
            JPM_DIRECTORIO_DEPENDENCIAS);
    
    /* Verificar si existe el directorio */
    if (!es_directorio(JPM_DIRECTORIO_DEPENDENCIAS)) {
        jpm_log(ctx, JPM_LOG_INFO, "No hay paquetes instalados.\n");
        jpm_log(ctx, JPM_LOG_INFO, "Ejecuta 'jpm install <paquete>' para instalar paquetes.\n");
        return JPM_EXITO;
    }
    
    /* Abrir directorio */
    DIR *dir = opendir(JPM_DIRECTORIO_DEPENDENCIAS);
    if (!dir) {
        fprintf(stderr, "[ERROR JPM] No se pudo abrir directorio %s: %s\n", 
                JPM_DIRECTORIO_DEPENDENCIAS, strerror(errno));
        return JPM_ERROR_ARCHIVO;
    }
    
    struct dirent *entrada;
    int contador = 0;
    char ruta_paquete[JPM_MAX_RUTA];
    char ruta_json[JPM_MAX_RUTA];
    
    while ((entrada = readdir(dir)) != NULL) {
        /* Saltar . y .. */
        if (strcmp(entrada->d_name, ".") == 0 || strcmp(entrada->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(ruta_paquete, sizeof(ruta_paquete), "%s/%s", 
                JPM_DIRECTORIO_DEPENDENCIAS, entrada->d_name);
        
        /* Solo mostrar directorios */
        if (!es_directorio(ruta_paquete)) {
            continue;
        }
        
        /* Buscar jasboot.json */
        snprintf(ruta_json, sizeof(ruta_json), "%s/%s", 
                ruta_paquete, JPM_ARCHIVO_METADATOS);
        
        if (!archivo_existe(ruta_json)) {
            /* Paquete sin metadatos - mostrar solo nombre */
            jpm_log(ctx, JPM_LOG_INFO, "  - %s (sin metadatos)\n", entrada->d_name);
            contador++;
            continue;
        }
        
        /* Leer metadatos */
        jpm_metadatos_t *meta = jpm_metadatos_leer(ruta_json);
        if (!meta) {
            jpm_log(ctx, JPM_LOG_INFO, "  - %s (metadatos inválidos)\n", entrada->d_name);
            contador++;
            continue;
        }
        
        /* Mostrar información del paquete */
        jpm_log(ctx, JPM_LOG_INFO, "  - %s@%s", meta->nombre, meta->version);
        
        if (meta->descripcion[0] != '\0') {
            jpm_log(ctx, JPM_LOG_INFO, " - %s", meta->descripcion);
        }
        
        jpm_log(ctx, JPM_LOG_INFO, "\n");
        
        jpm_metadatos_liberar(meta);
        contador++;
    }
    
    closedir(dir);
    
    if (contador == 0) {
        jpm_log(ctx, JPM_LOG_INFO, "No hay paquetes instalados.\n");
    } else {
        jpm_log(ctx, JPM_LOG_INFO, "\nTotal: %d paquete%s instalado%s\n", 
                contador, contador == 1 ? "" : "s", contador == 1 ? "" : "s");
    }
    
    return JPM_EXITO;
}

/* =============================================================================
 * COMANDO: jpm info
 * Muestra información detallada de un paquete
 * =============================================================================
 */

int jpm_cmd_info(jpm_contexto_t *ctx, const char *nombre) {
    if (!ctx) {
        fprintf(stderr, "[ERROR JPM] Contexto NULL en cmd_info\n");
        return JPM_ERROR_VALIDACION;
    }
    
    if (!nombre || nombre[0] == '\0') {
        fprintf(stderr, "[ERROR JPM] Nombre de paquete requerido\n");
        fprintf(stderr, "Uso: jpm info <paquete>\n");
        return JPM_ERROR_VALIDACION;
    }
    
    /* Buscar en a-modulos/ */
    char ruta_paquete[JPM_MAX_RUTA];
    char ruta_json[JPM_MAX_RUTA];
    
    snprintf(ruta_paquete, sizeof(ruta_paquete), "%s/%s", 
            JPM_DIRECTORIO_DEPENDENCIAS, nombre);
    snprintf(ruta_json, sizeof(ruta_json), "%s/%s", 
            ruta_paquete, JPM_ARCHIVO_METADATOS);
    
    /* Verificar si existe */
    if (!es_directorio(ruta_paquete)) {
        fprintf(stderr, "[ERROR JPM] Paquete '%s' no está instalado\n", nombre);
        fprintf(stderr, "Ejecuta 'jpm list' para ver paquetes instalados.\n");
        return JPM_ERROR_NO_ENCONTRADO;
    }
    
    if (!archivo_existe(ruta_json)) {
        fprintf(stderr, "[ERROR JPM] Paquete '%s' no tiene archivo %s\n", 
                nombre, JPM_ARCHIVO_METADATOS);
        return JPM_ERROR_ARCHIVO;
    }
    
    /* Leer metadatos */
    jpm_metadatos_t *meta = jpm_metadatos_leer(ruta_json);
    if (!meta) {
        fprintf(stderr, "[ERROR JPM] No se pudieron leer metadatos de '%s'\n", nombre);
        return JPM_ERROR_JSON;
    }
    
    /* Mostrar información */
    jpm_log(ctx, JPM_LOG_INFO, "\n");
    jpm_log(ctx, JPM_LOG_INFO, "=== %s ===\n\n", meta->nombre);
    jpm_log(ctx, JPM_LOG_INFO, "Versión:      %s\n", meta->version);
    jpm_log(ctx, JPM_LOG_INFO, "Descripción:  %s\n", meta->descripcion);
    jpm_log(ctx, JPM_LOG_INFO, "Autor:        %s\n", meta->autor);
    jpm_log(ctx, JPM_LOG_INFO, "Licencia:     %s\n", meta->licencia);
    jpm_log(ctx, JPM_LOG_INFO, "Tipo:         %s\n", 
            meta->tipo == JPM_TIPO_BIBLIOTECA ? "biblioteca" :
            meta->tipo == JPM_TIPO_APLICACION ? "aplicación" :
            meta->tipo == JPM_TIPO_PLUGIN ? "plugin" : "herramienta");
    jpm_log(ctx, JPM_LOG_INFO, "Principal:    %s\n", meta->principal);
    jpm_log(ctx, JPM_LOG_INFO, "Jasboot:      %s\n", meta->jasboot_version);
    
    if (meta->repositorio[0] != '\0') {
        jpm_log(ctx, JPM_LOG_INFO, "Repositorio:  %s\n", meta->repositorio);
    }
    
    if (meta->homepage[0] != '\0') {
        jpm_log(ctx, JPM_LOG_INFO, "Homepage:     %s\n", meta->homepage);
    }
    
    /* Dependencias */
    if (meta->num_dependencias > 0) {
        jpm_log(ctx, JPM_LOG_INFO, "\nDependencias:\n");
        for (size_t i = 0; i < meta->num_dependencias; i++) {
            jpm_log(ctx, JPM_LOG_INFO, "  - %s@%s%s\n",
                    meta->dependencias[i].nombre,
                    meta->dependencias[i].operador,
                    meta->dependencias[i].version);
        }
    }
    
    /* Dependencias de desarrollo */
    if (meta->num_dependencias_desarrollo > 0) {
        jpm_log(ctx, JPM_LOG_INFO, "\nDependencias de desarrollo:\n");
        for (size_t i = 0; i < meta->num_dependencias_desarrollo; i++) {
            jpm_log(ctx, JPM_LOG_INFO, "  - %s@%s%s\n",
                    meta->dependencias_desarrollo[i].nombre,
                    meta->dependencias_desarrollo[i].operador,
                    meta->dependencias_desarrollo[i].version);
        }
    }
    
    /* Scripts */
    if (meta->num_scripts > 0) {
        jpm_log(ctx, JPM_LOG_INFO, "\nScripts:\n");
        for (size_t i = 0; i < meta->num_scripts; i++) {
            jpm_log(ctx, JPM_LOG_INFO, "  %s: %s\n",
                    meta->scripts[i].nombre,
                    meta->scripts[i].comando);
        }
    }
    
    /* Palabras clave */
    if (meta->num_palabras_clave > 0) {
        jpm_log(ctx, JPM_LOG_INFO, "\nPalabras clave:\n  ");
        for (size_t i = 0; i < meta->num_palabras_clave; i++) {
            jpm_log(ctx, JPM_LOG_INFO, "%s%s", 
                    meta->palabras_clave[i],
                    i < meta->num_palabras_clave - 1 ? ", " : "");
        }
        jpm_log(ctx, JPM_LOG_INFO, "\n");
    }
    
    /* Información de instalación */
    size_t tamano = calcular_tamano_directorio(ruta_paquete);
    char tamano_str[64];
    formatear_bytes(tamano, tamano_str, sizeof(tamano_str));
    
    jpm_log(ctx, JPM_LOG_INFO, "\nInstalación:\n");
    jpm_log(ctx, JPM_LOG_INFO, "  Ruta:   %s\n", ruta_paquete);
    jpm_log(ctx, JPM_LOG_INFO, "  Tamaño: %s\n", tamano_str);
    
    jpm_log(ctx, JPM_LOG_INFO, "\n");
    
    jpm_metadatos_liberar(meta);
    return JPM_EXITO;
}

/* =============================================================================
 * COMANDO: jpm clean
 * Limpia cache y archivos temporales
 * =============================================================================
 */

int jpm_cmd_clean(jpm_contexto_t *ctx) {
    if (!ctx) {
        fprintf(stderr, "[ERROR JPM] Contexto NULL en cmd_clean\n");
        return JPM_ERROR_VALIDACION;
    }
    
    jpm_log(ctx, JPM_LOG_INFO, "Limpiando cache de JPM...\n\n");
    
    size_t total_liberado = 0;
    int archivos_eliminados = 0;
    
    /* Limpiar .jpm-cache */
    if (es_directorio(JPM_DIRECTORIO_CACHE)) {
        size_t tamano = calcular_tamano_directorio(JPM_DIRECTORIO_CACHE);
        
        jpm_log(ctx, JPM_LOG_INFO, "Limpiando %s...\n", JPM_DIRECTORIO_CACHE);
        
        if (eliminar_directorio_recursivo(JPM_DIRECTORIO_CACHE) == JPM_EXITO) {
            char tamano_str[64];
            formatear_bytes(tamano, tamano_str, sizeof(tamano_str));
            jpm_log(ctx, JPM_LOG_INFO, "  ✓ Eliminado %s\n", tamano_str);
            total_liberado += tamano;
            archivos_eliminados++;
        } else {
            jpm_log(ctx, JPM_LOG_AVISO, "  ✗ No se pudo eliminar completamente\n");
        }
    }
    
    /* Limpiar .jpm-temp */
    if (es_directorio(JPM_DIRECTORIO_TEMP)) {
        size_t tamano = calcular_tamano_directorio(JPM_DIRECTORIO_TEMP);
        
        jpm_log(ctx, JPM_LOG_INFO, "Limpiando %s...\n", JPM_DIRECTORIO_TEMP);
        
        if (eliminar_directorio_recursivo(JPM_DIRECTORIO_TEMP) == JPM_EXITO) {
            char tamano_str[64];
            formatear_bytes(tamano, tamano_str, sizeof(tamano_str));
            jpm_log(ctx, JPM_LOG_INFO, "  ✓ Eliminado %s\n", tamano_str);
            total_liberado += tamano;
            archivos_eliminados++;
        } else {
            jpm_log(ctx, JPM_LOG_AVISO, "  ✗ No se pudo eliminar completamente\n");
        }
    }
    
    /* Resumen */
    jpm_log(ctx, JPM_LOG_INFO, "\n");
    if (total_liberado > 0) {
        char total_str[64];
        formatear_bytes(total_liberado, total_str, sizeof(total_str));
        jpm_log(ctx, JPM_LOG_INFO, "✓ Limpieza completada\n");
        jpm_log(ctx, JPM_LOG_INFO, "  Espacio liberado: %s\n", total_str);
        jpm_log(ctx, JPM_LOG_INFO, "  Elementos eliminados: %d\n", archivos_eliminados);
    } else {
        jpm_log(ctx, JPM_LOG_INFO, "No hay archivos temporales para limpiar.\n");
    }
    
    return JPM_EXITO;
}

/* =============================================================================
 * COMANDO: jpm uninstall
 * Desinstala un paquete de a-modulos/
 * =============================================================================
 */

int jpm_cmd_uninstall(jpm_contexto_t *ctx, const char *nombre) {
    if (!ctx) {
        fprintf(stderr, "[ERROR JPM] Contexto NULL en cmd_uninstall\n");
        return JPM_ERROR_VALIDACION;
    }
    
    if (!nombre || nombre[0] == '\0') {
        fprintf(stderr, "[ERROR JPM] Nombre de paquete requerido\n");
        fprintf(stderr, "Uso: jpm uninstall <paquete>\n");
        return JPM_ERROR_VALIDACION;
    }
    
    jpm_log(ctx, JPM_LOG_INFO, "Desinstalando paquete '%s'...\n", nombre);
    
    /* Construir ruta del paquete */
    char ruta_paquete[JPM_MAX_RUTA];
    snprintf(ruta_paquete, sizeof(ruta_paquete), "%s/%s", 
            JPM_DIRECTORIO_DEPENDENCIAS, nombre);
    
    /* Verificar si existe */
    if (!es_directorio(ruta_paquete)) {
        fprintf(stderr, "[ERROR JPM] Paquete '%s' no está instalado\n", nombre);
        fprintf(stderr, "Ejecuta 'jpm list' para ver paquetes instalados.\n");
        return JPM_ERROR_NO_ENCONTRADO;
    }
    
    /* Verificar si está en jasboot.json como dependencia */
    char ruta_json[JPM_MAX_RUTA];
    snprintf(ruta_json, sizeof(ruta_json), "%s", JPM_ARCHIVO_METADATOS);
    
    bool es_dependencia = false;
    if (archivo_existe(ruta_json)) {
        jpm_metadatos_t *meta_proyecto = jpm_metadatos_leer(ruta_json);
        if (meta_proyecto) {
            /* Verificar en dependencias */
            for (size_t i = 0; i < meta_proyecto->num_dependencias; i++) {
                if (strcmp(meta_proyecto->dependencias[i].nombre, nombre) == 0) {
                    es_dependencia = true;
                    break;
                }
            }
            
            /* Verificar en dependencias de desarrollo */
            if (!es_dependencia) {
                for (size_t i = 0; i < meta_proyecto->num_dependencias_desarrollo; i++) {
                    if (strcmp(meta_proyecto->dependencias_desarrollo[i].nombre, nombre) == 0) {
                        es_dependencia = true;
                        break;
                    }
                }
            }
            
            jpm_metadatos_liberar(meta_proyecto);
        }
    }
    
    /* Advertir si es dependencia listada */
    if (es_dependencia) {
        jpm_log(ctx, JPM_LOG_AVISO, 
                "AVISO: '%s' está listado en %s\n", nombre, JPM_ARCHIVO_METADATOS);
        jpm_log(ctx, JPM_LOG_AVISO, 
                "       Debes eliminarlo manualmente del archivo si ya no lo necesitas.\n");
    }
    
    /* Calcular tamaño antes de eliminar */
    size_t tamano = calcular_tamano_directorio(ruta_paquete);
    char tamano_str[64];
    formatear_bytes(tamano, tamano_str, sizeof(tamano_str));
    
    /* Leer metadatos del paquete antes de eliminarlo */
    char ruta_meta_paquete[JPM_MAX_RUTA];
    snprintf(ruta_meta_paquete, sizeof(ruta_meta_paquete), "%s/%s", 
            ruta_paquete, JPM_ARCHIVO_METADATOS);
    
    char version[JPM_MAX_VERSION] = "desconocida";
    if (archivo_existe(ruta_meta_paquete)) {
        jpm_metadatos_t *meta = jpm_metadatos_leer(ruta_meta_paquete);
        if (meta) {
            snprintf(version, sizeof(version), "%s", meta->version);
            jpm_metadatos_liberar(meta);
        }
    }
    
    /* Eliminar directorio del paquete */
    jpm_log(ctx, JPM_LOG_INFO, "Eliminando %s (%s)...\n", ruta_paquete, tamano_str);
    
    int resultado = eliminar_directorio_recursivo(ruta_paquete);
    if (resultado != JPM_EXITO) {
        fprintf(stderr, "[ERROR JPM] No se pudo desinstalar el paquete '%s'\n", nombre);
        return resultado;
    }
    
    /* Mensaje de éxito */
    jpm_log(ctx, JPM_LOG_INFO, "\n✓ Paquete '%s@%s' desinstalado correctamente\n", 
            nombre, version);
    jpm_log(ctx, JPM_LOG_INFO, "  Espacio liberado: %s\n\n", tamano_str);
    
    return JPM_EXITO;
}