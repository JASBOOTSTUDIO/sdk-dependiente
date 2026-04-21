/*
 * JPM - Jasboot Package Manager
 * Módulo Core: jpm_core.c
 * 
 * Funciones de inicialización, configuración y utilidades básicas
 * Copyright (c) 2024 Jasboot Team
 */

#include "../include/jpm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>

#ifdef _WIN32
    #include <direct.h>
    #include <windows.h>
    #define getcwd _getcwd
    #define mkdir(path, mode) _mkdir(path)
    #define PATH_SEP "\\"
#else
    #include <unistd.h>
    #include <pwd.h>
    #include <dirent.h>
    #define PATH_SEP "/"
#endif

/* =============================================================================
 * FUNCIONES PRIVADAS
 * =============================================================================
 */

/* Obtiene el directorio home del usuario */
static const char* obtener_home(void) {
    static char home[JPM_MAX_RUTA];
    
#ifdef _WIN32
    const char *userprofile = getenv("USERPROFILE");
    if (userprofile) {
        strncpy(home, userprofile, JPM_MAX_RUTA - 1);
        home[JPM_MAX_RUTA - 1] = '\0';
        return home;
    }
    const char *homedrive = getenv("HOMEDRIVE");
    const char *homepath = getenv("HOMEPATH");
    if (homedrive && homepath) {
        snprintf(home, JPM_MAX_RUTA, "%s%s", homedrive, homepath);
        return home;
    }
#else
    const char *home_env = getenv("HOME");
    if (home_env) {
        strncpy(home, home_env, JPM_MAX_RUTA - 1);
        home[JPM_MAX_RUTA - 1] = '\0';
        return home;
    }
    struct passwd *pw = getpwuid(getuid());
    if (pw && pw->pw_dir) {
        strncpy(home, pw->pw_dir, JPM_MAX_RUTA - 1);
        home[JPM_MAX_RUTA - 1] = '\0';
        return home;
    }
#endif
    
    return ".";
}

/* Crea configuración por defecto */
static void crear_configuracion_defecto(jpm_config_t *config) {
    const char *home = obtener_home();
    
    /* Directorios */
    snprintf(config->directorio_cache, JPM_MAX_RUTA, "%s%s.jpm%scache", home, PATH_SEP, PATH_SEP);
    snprintf(config->directorio_global, JPM_MAX_RUTA, "%s%s.jpm%spaquetes", home, PATH_SEP, PATH_SEP);
    
    /* URL del registro */
    strncpy(config->url_registro, JPM_REGISTRO_URL, JPM_MAX_URL - 1);
    config->url_registro[JPM_MAX_URL - 1] = '\0';
    
    /* Opciones por defecto */
    config->modo_verboso = false;
    config->modo_silencioso = false;
    config->forzar = false;
    config->simular = false;
    config->es_global = false;
    config->timeout_segundos = 30;
    config->verificar_ssl = true;
    config->token_autenticacion[0] = '\0';
}

/* =============================================================================
 * FUNCIONES DE INICIALIZACIÓN Y LIMPIEZA
 * =============================================================================
 */

jpm_contexto_t* jpm_inicializar(void) {
    jpm_contexto_t *ctx = (jpm_contexto_t*)calloc(1, sizeof(jpm_contexto_t));
    if (!ctx) {
        fprintf(stderr, "[ERROR] Fallo al asignar memoria para el contexto\n");
        return NULL;
    }
    
    /* Configuración por defecto */
    crear_configuracion_defecto(&ctx->config);
    
    /* Directorio de trabajo actual */
    if (!getcwd(ctx->directorio_trabajo, JPM_MAX_RUTA)) {
        fprintf(stderr, "[ERROR] No se pudo obtener el directorio de trabajo\n");
        free(ctx);
        return NULL;
    }
    
    /* Inicializar punteros */
    ctx->metadatos_proyecto = NULL;
    ctx->archivo_lock = NULL;
    ctx->paquetes_instalados = NULL;
    ctx->num_paquetes_instalados = 0;
    ctx->log_archivo = NULL;
    ctx->nivel_log = JPM_LOG_INFO;
    
    /* Crear directorios necesarios si no existen */
    jpm_crear_directorio_recursivo(ctx->config.directorio_cache);
    jpm_crear_directorio_recursivo(ctx->config.directorio_global);
    
    return ctx;
}

void jpm_liberar(jpm_contexto_t *ctx) {
    if (!ctx) return;
    
    /* Liberar metadatos del proyecto */
    if (ctx->metadatos_proyecto) {
        jpm_metadatos_liberar(ctx->metadatos_proyecto);
    }
    
    /* Liberar archivo lock */
    if (ctx->archivo_lock) {
        jpm_lock_liberar(ctx->archivo_lock);
    }
    
    /* Liberar paquetes instalados */
    if (ctx->paquetes_instalados) {
        free(ctx->paquetes_instalados);
    }
    
    /* Cerrar archivo de log */
    if (ctx->log_archivo) {
        fclose(ctx->log_archivo);
    }
    
    /* Liberar contexto */
    free(ctx);
}

int jpm_cargar_configuracion(jpm_contexto_t *ctx, const char *ruta_config) {
    if (!ctx) return JPM_ERROR_VALIDACION;
    
    /* Si no se especifica ruta, usar configuración por defecto */
    if (!ruta_config) {
        crear_configuracion_defecto(&ctx->config);
        return JPM_EXITO;
    }
    
    /* TODO: Implementar carga desde archivo JSON */
    /* Por ahora, usar configuración por defecto */
    crear_configuracion_defecto(&ctx->config);
    
    return JPM_EXITO;
}

int jpm_guardar_configuracion(jpm_contexto_t *ctx, const char *ruta_config) {
    if (!ctx || !ruta_config) return JPM_ERROR_VALIDACION;
    
    /* TODO: Implementar guardado a archivo JSON */
    
    return JPM_EXITO;
}

/* =============================================================================
 * SISTEMA DE LOGGING
 * =============================================================================
 */

void jpm_log(jpm_contexto_t *ctx, jpm_nivel_log_t nivel, const char *formato, ...) {
    if (!ctx) return;
    
    /* Verificar nivel de log */
    if (nivel < ctx->nivel_log) return;
    if (ctx->config.modo_silencioso && nivel < JPM_LOG_ERROR) return;
    
    /* Prefijos por nivel */
    const char *prefijos[] = {
        "[DEBUG]",
        "[INFO]",
        "[AVISO]",
        "[ERROR]",
        "[CRITICO]"
    };
    
    /* Colores ANSI (solo en terminales compatibles) */
    const char *colores[] = {
        "\033[0;36m",  /* DEBUG - Cyan */
        "\033[0;32m",  /* INFO - Verde */
        "\033[0;33m",  /* AVISO - Amarillo */
        "\033[0;31m",  /* ERROR - Rojo */
        "\033[0;35m"   /* CRITICO - Magenta */
    };
    const char *reset = "\033[0m";
    
    /* Imprimir en consola */
    FILE *salida = (nivel >= JPM_LOG_ERROR) ? stderr : stdout;
    
    /* Usar colores solo si no estamos en modo verboso o si es terminal */
    bool usar_colores = false;
#ifndef _WIN32
    usar_colores = isatty(fileno(salida));
#endif
    
    if (usar_colores) {
        fprintf(salida, "%s%s%s ", colores[nivel], prefijos[nivel], reset);
    } else {
        fprintf(salida, "%s ", prefijos[nivel]);
    }
    
    /* Imprimir mensaje */
    va_list args;
    va_start(args, formato);
    vfprintf(salida, formato, args);
    va_end(args);
    
    fprintf(salida, "\n");
    
    /* Escribir a archivo de log si está abierto */
    if (ctx->log_archivo) {
        time_t ahora = time(NULL);
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&ahora));
        
        fprintf(ctx->log_archivo, "[%s] %s ", timestamp, prefijos[nivel]);
        
        va_start(args, formato);
        vfprintf(ctx->log_archivo, formato, args);
        va_end(args);
        
        fprintf(ctx->log_archivo, "\n");
        fflush(ctx->log_archivo);
    }
}

/* =============================================================================
 * FUNCIONES DE UTILIDADES
 * =============================================================================
 */

const char* jpm_obtener_directorio_home(void) {
    return obtener_home();
}

const char* jpm_obtener_directorio_global(void) {
    static char global[JPM_MAX_RUTA];
    const char *home = obtener_home();
    snprintf(global, JPM_MAX_RUTA, "%s%s.jpm%spaquetes", home, PATH_SEP, PATH_SEP);
    return global;
}

int jpm_crear_directorio_recursivo(const char *ruta) {
    if (!ruta || ruta[0] == '\0') return JPM_ERROR_VALIDACION;
    
    /* Copiar ruta para modificarla */
    char tmp[JPM_MAX_RUTA];
    strncpy(tmp, ruta, JPM_MAX_RUTA - 1);
    tmp[JPM_MAX_RUTA - 1] = '\0';
    
    /* Normalizar separadores a PATH_SEP */
    for (char *p = tmp; *p; p++) {
        if (*p == '/') *p = PATH_SEP[0];
    }
    
    /* Si la ruta ya existe, es un éxito */
    struct stat st;
    if (stat(tmp, &st) == 0 && S_ISDIR(st.st_mode)) {
        return JPM_EXITO;
    }
    
#ifdef _WIN32
    /* En Windows, usar CreateDirectoryA para cada nivel */
    char ruta_actual[JPM_MAX_RUTA] = "";
    const char *p = tmp;
    
    /* Si empieza con letra y dos puntos (ej: C:), copiar eso primero */
    if (p[0] != '\0' && p[1] == ':') {
        ruta_actual[0] = p[0];
        ruta_actual[1] = ':';
        ruta_actual[2] = '\0';
        p += 2;
    }
    
    while (*p) {
        if (*p == PATH_SEP[0]) {
            p++;
            continue;
        }
        
        /* Encontrar el siguiente separador */
        const char *siguiente_sep = strchr(p, PATH_SEP[0]);
        if (siguiente_sep) {
            size_t len = siguiente_sep - p;
            strncat(ruta_actual, p, len);
            p = siguiente_sep;
        } else {
            strcat(ruta_actual, p);
            p = p + strlen(p);
        }
        
        /* Crear directorio si no existe */
        if (stat(ruta_actual, &st) != 0) {
            if (_mkdir(ruta_actual) != 0) {
                /* Verificar si otro proceso lo creó */
                if (stat(ruta_actual, &st) != 0 || !(st.st_mode & _S_IFDIR)) {
                    fprintf(stderr, "[ERROR] No se pudo crear directorio: %s\n", ruta_actual);
                    return JPM_ERROR_ARCHIVO;
                }
            }
        }
    }
    
    return JPM_EXITO;
#else
    /* En Unix/Linux/macOS, usar enfoque tradicional */
    for (char *p = tmp + 1; *p; p++) {
        if (*p == PATH_SEP[0]) {
            *p = '\0';
            
            /* Intentar crear directorio */
            struct stat st2;
            if (stat(tmp, &st2) != 0) {
                if (mkdir(tmp, 0755) != 0) {
                    /* Ignorar error si el directorio ya existe */
                    if (stat(tmp, &st2) != 0 || !S_ISDIR(st2.st_mode)) {
                        fprintf(stderr, "[ERROR] No se pudo crear directorio: %s\n", tmp);
                        return JPM_ERROR_ARCHIVO;
                    }
                }
            }
            
            *p = PATH_SEP[0];
        }
    }
    
    /* Crear el directorio final */
    if (stat(tmp, &st) != 0) {
        if (mkdir(tmp, 0755) != 0) {
            struct stat st2;
            if (stat(tmp, &st2) != 0 || !S_ISDIR(st2.st_mode)) {
                fprintf(stderr, "[ERROR] No se pudo crear directorio: %s\n", tmp);
                return JPM_ERROR_ARCHIVO;
            }
        }
    }
    
    return JPM_EXITO;
#endif
}

bool jpm_archivo_existe(const char *ruta) {
    if (!ruta) return false;
    
    struct stat st;
    return (stat(ruta, &st) == 0);
}

bool jpm_es_directorio(const char *ruta) {
    if (!ruta) return false;
    
    struct stat st;
    if (stat(ruta, &st) != 0) return false;
    
#ifdef _WIN32
    return (st.st_mode & _S_IFDIR) != 0;
#else
    return S_ISDIR(st.st_mode);
#endif
}

size_t jpm_obtener_tamano_archivo(const char *ruta) {
    if (!ruta) return 0;
    
    struct stat st;
    if (stat(ruta, &st) != 0) return 0;
    
    return (size_t)st.st_size;
}

void jpm_formatear_bytes(size_t bytes, char *buffer, size_t buffer_len) {
    if (!buffer || buffer_len == 0) return;
    
    const char *unidades[] = {"B", "KB", "MB", "GB", "TB"};
    double tamano = (double)bytes;
    int idx = 0;
    
    while (tamano >= 1024.0 && idx < 4) {
        tamano /= 1024.0;
        idx++;
    }
    
    if (idx == 0) {
        snprintf(buffer, buffer_len, "%zu %s", bytes, unidades[idx]);
    } else {
        snprintf(buffer, buffer_len, "%.2f %s", tamano, unidades[idx]);
    }
}

/* =============================================================================
 * FUNCIONES DE VALIDACIÓN
 * =============================================================================
 */

bool jpm_validar_nombre(const char *nombre) {
    if (!nombre || nombre[0] == '\0') return false;
    
    /* El nombre no puede empezar con punto o guión */
    if (nombre[0] == '.' || nombre[0] == '-') return false;
    
    /* Verificar caracteres válidos */
    for (const char *p = nombre; *p; p++) {
        char c = *p;
        bool valido = (c >= 'a' && c <= 'z') ||
                      (c >= '0' && c <= '9') ||
                      c == '-' || c == '_';
        
        if (!valido) return false;
    }
    
    /* Longitud razonable */
    size_t len = strlen(nombre);
    if (len < 2 || len > 100) return false;
    
    return true;
}

bool jpm_validar_version(const char *version) {
    if (!version || version[0] == '\0') return false;
    
    /* Formato básico: X.Y.Z */
    int mayor, menor, parche;
    int num = sscanf(version, "%d.%d.%d", &mayor, &menor, &parche);
    
    if (num != 3) return false;
    if (mayor < 0 || menor < 0 || parche < 0) return false;
    
    return true;
}

int jpm_comparar_versiones(const char *v1, const char *v2) {
    if (!v1 || !v2) return 0;
    
    int v1_mayor, v1_menor, v1_parche;
    int v2_mayor, v2_menor, v2_parche;
    
    sscanf(v1, "%d.%d.%d", &v1_mayor, &v1_menor, &v1_parche);
    sscanf(v2, "%d.%d.%d", &v2_mayor, &v2_menor, &v2_parche);
    
    /* Comparar mayor */
    if (v1_mayor != v2_mayor) {
        return (v1_mayor > v2_mayor) ? 1 : -1;
    }
    
    /* Comparar menor */
    if (v1_menor != v2_menor) {
        return (v1_menor > v2_menor) ? 1 : -1;
    }
    
    /* Comparar parche */
    if (v1_parche != v2_parche) {
        return (v1_parche > v2_parche) ? 1 : -1;
    }
    
    return 0;
}

bool jpm_version_cumple_requisito(const char *version, const char *requisito) {
    if (!version || !requisito) return false;
    
    /* Parsear versión */
    int v_mayor, v_menor, v_parche;
    if (sscanf(version, "%d.%d.%d", &v_mayor, &v_menor, &v_parche) != 3) {
        return false;
    }
    
    /* Determinar operador */
    const char *op = requisito;
    const char *ver = requisito;
    
    if (requisito[0] == '^') {
        /* Compatible: ^1.2.3 = >=1.2.3 <2.0.0 */
        ver = requisito + 1;
        int r_mayor, r_menor, r_parche;
        if (sscanf(ver, "%d.%d.%d", &r_mayor, &r_menor, &r_parche) != 3) {
            return false;
        }
        
        if (v_mayor != r_mayor) return false;
        if (v_menor < r_menor) return false;
        if (v_menor == r_menor && v_parche < r_parche) return false;
        
        return true;
        
    } else if (requisito[0] == '~') {
        /* Compatible en parche: ~1.2.3 = >=1.2.3 <1.3.0 */
        ver = requisito + 1;
        int r_mayor, r_menor, r_parche;
        if (sscanf(ver, "%d.%d.%d", &r_mayor, &r_menor, &r_parche) != 3) {
            return false;
        }
        
        if (v_mayor != r_mayor || v_menor != r_menor) return false;
        if (v_parche < r_parche) return false;
        
        return true;
        
    } else if (requisito[0] == '>' && requisito[1] == '=') {
        /* Mayor o igual */
        ver = requisito + 2;
        return jpm_comparar_versiones(version, ver) >= 0;
        
    } else if (requisito[0] == '<' && requisito[1] == '=') {
        /* Menor o igual */
        ver = requisito + 2;
        return jpm_comparar_versiones(version, ver) <= 0;
        
    } else if (requisito[0] == '>') {
        /* Mayor */
        ver = requisito + 1;
        return jpm_comparar_versiones(version, ver) > 0;
        
    } else if (requisito[0] == '<') {
        /* Menor */
        ver = requisito + 1;
        return jpm_comparar_versiones(version, ver) < 0;
        
    } else {
        /* Versión exacta */
        return jpm_comparar_versiones(version, requisito) == 0;
    }
}

int jpm_copiar_directorio(const char *origen, const char *destino) {
    if (!origen || !destino) {
        return JPM_ERROR_VALIDACION;
    }

    /* Crear directorio destino */
    if (jpm_crear_directorio_recursivo(destino) != JPM_EXITO) {
        return JPM_ERROR_ARCHIVO;
    }

#ifdef _WIN32
    WIN32_FIND_DATAA find_data;
    char patron[JPM_MAX_RUTA];
    snprintf(patron, sizeof(patron), "%s\\*", origen);
    
    HANDLE hFind = FindFirstFileA(patron, &find_data);
    if (hFind == INVALID_HANDLE_VALUE) {
        return JPM_ERROR_ARCHIVO;
    }

    do {
        if (strcmp(find_data.cFileName, ".") == 0 || 
            strcmp(find_data.cFileName, "..") == 0) {
            continue;
        }

        char ruta_origen[JPM_MAX_RUTA];
        char ruta_destino[JPM_MAX_RUTA];
        snprintf(ruta_origen, sizeof(ruta_origen), "%s\\%s", origen, find_data.cFileName);
        snprintf(ruta_destino, sizeof(ruta_destino), "%s\\%s", destino, find_data.cFileName);

        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            /* Copiar subdirectorio recursivamente */
            if (jpm_copiar_directorio(ruta_origen, ruta_destino) != JPM_EXITO) {
                FindClose(hFind);
                return JPM_ERROR_ARCHIVO;
            }
        } else {
            /* Copiar archivo */
            if (!CopyFileA(ruta_origen, ruta_destino, FALSE)) {
                FindClose(hFind);
                return JPM_ERROR_ARCHIVO;
            }
        }
    } while (FindNextFileA(hFind, &find_data));

    FindClose(hFind);
#else
    DIR *d = opendir(origen);
    if (!d) {
        return JPM_ERROR_ARCHIVO;
    }

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char ruta_origen[JPM_MAX_RUTA];
        char ruta_destino[JPM_MAX_RUTA];
        snprintf(ruta_origen, sizeof(ruta_origen), "%s/%s", origen, entry->d_name);
        snprintf(ruta_destino, sizeof(ruta_destino), "%s/%s", destino, entry->d_name);

        struct stat st;
        if (stat(ruta_origen, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                /* Copiar subdirectorio recursivamente */
                if (jpm_copiar_directorio(ruta_origen, ruta_destino) != JPM_EXITO) {
                    closedir(d);
                    return JPM_ERROR_ARCHIVO;
                }
            } else {
                /* Copiar archivo */
                FILE *f_in = fopen(ruta_origen, "rb");
                if (!f_in) {
                    closedir(d);
                    return JPM_ERROR_ARCHIVO;
                }

                FILE *f_out = fopen(ruta_destino, "wb");
                if (!f_out) {
                    fclose(f_in);
                    closedir(d);
                    return JPM_ERROR_ARCHIVO;
                }

                char buffer[8192];
                size_t bytes;
                while ((bytes = fread(buffer, 1, sizeof(buffer), f_in)) > 0) {
                    if (fwrite(buffer, 1, bytes, f_out) != bytes) {
                        fclose(f_in);
                        fclose(f_out);
                        closedir(d);
                        return JPM_ERROR_ARCHIVO;
                    }
                }

                fclose(f_in);
                fclose(f_out);

                /* Preservar permisos */
                chmod(ruta_destino, st.st_mode);
            }
        }
    }
    closedir(d);
#endif

    return JPM_EXITO;
}

int jpm_eliminar_directorio(const char *ruta) {
    if (!ruta || strlen(ruta) == 0) {
        return JPM_ERROR_VALIDACION;
    }

    /* Verificar que existe */
    struct stat st;
    if (stat(ruta, &st) != 0) {
        return JPM_EXITO; /* Ya no existe */
    }

#ifdef _WIN32
    WIN32_FIND_DATAA find_data;
    char patron[JPM_MAX_RUTA];
    snprintf(patron, sizeof(patron), "%s\\*", ruta);
    
    HANDLE hFind = FindFirstFileA(patron, &find_data);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(find_data.cFileName, ".") == 0 || 
                strcmp(find_data.cFileName, "..") == 0) {
                continue;
            }

            char ruta_item[JPM_MAX_RUTA];
            snprintf(ruta_item, sizeof(ruta_item), "%s\\%s", ruta, find_data.cFileName);

            if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                jpm_eliminar_directorio(ruta_item);
            } else {
                DeleteFileA(ruta_item);
            }
        } while (FindNextFileA(hFind, &find_data));

        FindClose(hFind);
    }

    if (!RemoveDirectoryA(ruta)) {
        return JPM_ERROR_ARCHIVO;
    }
#else
    DIR *d = opendir(ruta);
    if (d) {
        struct dirent *entry;
        while ((entry = readdir(d)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            char ruta_item[JPM_MAX_RUTA];
            snprintf(ruta_item, sizeof(ruta_item), "%s/%s", ruta, entry->d_name);

            struct stat st_item;
            if (stat(ruta_item, &st_item) == 0) {
                if (S_ISDIR(st_item.st_mode)) {
                    jpm_eliminar_directorio(ruta_item);
                } else {
                    unlink(ruta_item);
                }
            }
        }
        closedir(d);
    }

    if (rmdir(ruta) != 0) {
        return JPM_ERROR_ARCHIVO;
    }
#endif

    return JPM_EXITO;
}

char** jpm_leer_archivo_ignorar(const char *ruta, size_t *num_patrones) {
    if (!num_patrones) {
        return NULL;
    }

    *num_patrones = 0;

    FILE *f = fopen(ruta, "r");
    if (!f) {
        /* Si no existe el archivo, usar patrones por defecto */
        char **patrones = (char**)malloc(sizeof(char*) * 3);
        if (!patrones) return NULL;

        patrones[0] = strdup("node_modules");
        patrones[1] = strdup(".git");
        patrones[2] = strdup(".jpm-temp");
        *num_patrones = 3;
        return patrones;
    }

    /* Contar líneas */
    char linea[512];
    size_t count = 0;
    while (fgets(linea, sizeof(linea), f)) {
        /* Ignorar líneas vacías y comentarios */
        char *p = linea;
        while (*p == ' ' || *p == '\t') p++;
        if (*p != '\0' && *p != '\n' && *p != '#') {
            count++;
        }
    }

    if (count == 0) {
        fclose(f);
        *num_patrones = 0;
        return NULL;
    }

    /* Asignar memoria para patrones */
    char **patrones = (char**)malloc(sizeof(char*) * count);
    if (!patrones) {
        fclose(f);
        return NULL;
    }

    /* Leer patrones */
    fseek(f, 0, SEEK_SET);
    size_t idx = 0;
    while (fgets(linea, sizeof(linea), f) && idx < count) {
        /* Eliminar espacios iniciales */
        char *p = linea;
        while (*p == ' ' || *p == '\t') p++;

        /* Ignorar líneas vacías y comentarios */
        if (*p == '\0' || *p == '\n' || *p == '#') {
            continue;
        }

        /* Eliminar salto de línea final */
        char *nl = strchr(p, '\n');
        if (nl) *nl = '\0';
        nl = strchr(p, '\r');
        if (nl) *nl = '\0';

        /* Copiar patrón */
        patrones[idx] = strdup(p);
        if (!patrones[idx]) {
            /* Error: liberar memoria ya asignada */
            for (size_t i = 0; i < idx; i++) {
                free(patrones[i]);
            }
            free(patrones);
            fclose(f);
            return NULL;
        }
        idx++;
    }

    fclose(f);
    *num_patrones = idx;
    return patrones;
}

bool jpm_debe_ignorar(const char *ruta, char **patrones, size_t num_patrones) {
    if (!ruta) {
        return false;
    }

    /* Siempre ignorar ciertos archivos/directorios */
    if (strcmp(ruta, ".") == 0 || strcmp(ruta, "..") == 0) {
        return true;
    }

    if (strstr(ruta, ".jpm-temp") != NULL || 
        strstr(ruta, ".jpm-cache") != NULL ||
        strstr(ruta, "node_modules") != NULL ||
        strstr(ruta, ".git") != NULL) {
        return true;
    }

    /* Verificar contra patrones del archivo .jpmignore */
    for (size_t i = 0; i < num_patrones; i++) {
        if (!patrones[i]) continue;

        /* Coincidencia exacta */
        if (strcmp(ruta, patrones[i]) == 0) {
            return true;
        }

        /* Coincidencia con comodines básicos */
        if (strstr(ruta, patrones[i]) != NULL) {
            return true;
        }

        /* Si el patrón termina con /, solo directorios */
        size_t len = strlen(patrones[i]);
        if (len > 0 && patrones[i][len-1] == '/') {
            char patron_sin_barra[512];
            strncpy(patron_sin_barra, patrones[i], sizeof(patron_sin_barra) - 1);
            patron_sin_barra[len-1] = '\0';
            if (strcmp(ruta, patron_sin_barra) == 0) {
                return true;
            }
        }
    }

    return false;
}