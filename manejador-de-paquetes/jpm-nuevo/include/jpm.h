#ifndef JPM_H
#define JPM_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

/* =============================================================================
 * JASBOOT PACKAGE MANAGER (JPM)
 * Sistema de gestión de paquetes para Jasboot
 * Todo en español - Carpeta de dependencias: a-modulos
 * =============================================================================
 */

#define JPM_VERSION "0.2.0"
#define JPM_VERSION_MAJOR 0
#define JPM_VERSION_MINOR 2
#define JPM_VERSION_PATCH 0

/* Constantes del sistema */
#define JPM_DIRECTORIO_DEPENDENCIAS "a-modulos"
#define JPM_ARCHIVO_METADATOS "jasboot.json"
#define JPM_ARCHIVO_LOCK "jpm.lock"
#define JPM_ARCHIVO_IGNORAR ".jpmignore"
#define JPM_EXTENSION_PAQUETE ".jpkg"
#define JPM_DIRECTORIO_CACHE ".jpm-cache"
#define JPM_DIRECTORIO_TEMP ".jpm-temp"
#define JPM_REGISTRO_URL "https://registry.jasboot.org"

/* Límites del sistema */
#define JPM_MAX_NOMBRE 256
#define JPM_MAX_VERSION 64
#define JPM_MAX_RUTA 1024
#define JPM_MAX_DESCRIPCION 1024
#define JPM_MAX_URL 2048
#define JPM_MAX_DEPENDENCIAS 256
#define JPM_MAX_SCRIPTS 32
#define JPM_BUFFER_SIZE 8192

/* Códigos de error */
#define JPM_EXITO 0
#define JPM_ERROR_GENERICO -1
#define JPM_ERROR_MEMORIA -2
#define JPM_ERROR_ARCHIVO -3
#define JPM_ERROR_JSON -4
#define JPM_ERROR_RED -5
#define JPM_ERROR_VALIDACION -6
#define JPM_ERROR_DEPENDENCIAS -7
#define JPM_ERROR_PERMISOS -8
#define JPM_ERROR_NO_ENCONTRADO -9
#define JPM_ERROR_YA_EXISTE -10

/* Niveles de log */
typedef enum {
    JPM_LOG_DEBUG = 0,
    JPM_LOG_INFO = 1,
    JPM_LOG_AVISO = 2,
    JPM_LOG_ERROR = 3,
    JPM_LOG_CRITICO = 4
} jpm_nivel_log_t;

/* Tipos de paquete */
typedef enum {
    JPM_TIPO_BIBLIOTECA = 0,
    JPM_TIPO_APLICACION = 1,
    JPM_TIPO_PLUGIN = 2,
    JPM_TIPO_HERRAMIENTA = 3
} jpm_tipo_paquete_t;

/* =============================================================================
 * ESTRUCTURAS DE DATOS
 * =============================================================================
 */

/* Dependencia de un paquete */
typedef struct {
    char nombre[JPM_MAX_NOMBRE];
    char version[JPM_MAX_VERSION];
    char operador[8];  /* ^, ~, >=, <=, =, >, < */
    bool es_desarrollo;
    bool es_opcional;
} jpm_dependencia_t;

/* Script definido en el paquete */
typedef struct {
    char nombre[JPM_MAX_NOMBRE];
    char comando[JPM_MAX_RUTA];
} jpm_script_t;

/* Metadatos del paquete (jasboot.json) */
typedef struct {
    char nombre[JPM_MAX_NOMBRE];
    char version[JPM_MAX_VERSION];
    char descripcion[JPM_MAX_DESCRIPCION];
    char autor[JPM_MAX_NOMBRE];
    char licencia[JPM_MAX_NOMBRE];
    char principal[JPM_MAX_RUTA];
    char jasboot_version[JPM_MAX_VERSION];
    char repositorio[JPM_MAX_URL];
    char homepage[JPM_MAX_URL];
    
    char **palabras_clave;
    size_t num_palabras_clave;
    
    jpm_dependencia_t *dependencias;
    size_t num_dependencias;
    
    jpm_dependencia_t *dependencias_desarrollo;
    size_t num_dependencias_desarrollo;
    
    jpm_script_t *scripts;
    size_t num_scripts;
    
    jpm_tipo_paquete_t tipo;
    
    time_t creado_en;
    time_t actualizado_en;
    
    char hash_sha512[129];
} jpm_metadatos_t;

/* Información de paquete instalado */
typedef struct {
    jpm_metadatos_t metadatos;
    char ruta_instalacion[JPM_MAX_RUTA];
    time_t fecha_instalacion;
    bool es_global;
    char instalado_desde[JPM_MAX_URL];  /* URL o ruta local */
} jpm_paquete_instalado_t;

/* Entrada del archivo jpm.lock */
typedef struct {
    char nombre[JPM_MAX_NOMBRE];
    char version[JPM_MAX_VERSION];
    char resuelto[JPM_MAX_URL];
    char hash[129];
    jpm_dependencia_t *dependencias;
    size_t num_dependencias;
} jpm_lock_entrada_t;

/* Archivo jpm.lock completo */
typedef struct {
    jpm_lock_entrada_t *entradas;
    size_t num_entradas;
    time_t generado_en;
} jpm_lock_t;

/* Opciones de configuración de JPM */
typedef struct {
    char directorio_cache[JPM_MAX_RUTA];
    char directorio_global[JPM_MAX_RUTA];
    char url_registro[JPM_MAX_URL];
    bool modo_verboso;
    bool modo_silencioso;
    bool forzar;
    bool simular;  /* dry-run */
    bool es_global;
    int timeout_segundos;
    bool verificar_ssl;
    char token_autenticacion[512];
} jpm_config_t;

/* Resultado de una operación */
typedef struct {
    int codigo;
    char mensaje[JPM_MAX_DESCRIPCION];
    void *datos;
} jpm_resultado_t;

/* Contexto principal de JPM */
typedef struct {
    jpm_config_t config;
    char directorio_trabajo[JPM_MAX_RUTA];
    jpm_metadatos_t *metadatos_proyecto;
    jpm_lock_t *archivo_lock;
    jpm_paquete_instalado_t *paquetes_instalados;
    size_t num_paquetes_instalados;
    FILE *log_archivo;
    jpm_nivel_log_t nivel_log;
} jpm_contexto_t;

/* Callback para progreso de descarga */
typedef void (*jpm_callback_progreso_t)(size_t descargado, size_t total, void *datos_usuario);

/* =============================================================================
 * FUNCIONES PRINCIPALES - INICIALIZACIÓN Y LIMPIEZA
 * =============================================================================
 */

/* Inicializa el contexto de JPM */
jpm_contexto_t* jpm_inicializar(void);

/* Libera el contexto de JPM */
void jpm_liberar(jpm_contexto_t *ctx);

/* Carga configuración desde archivo o valores por defecto */
int jpm_cargar_configuracion(jpm_contexto_t *ctx, const char *ruta_config);

/* Guarda configuración actual */
int jpm_guardar_configuracion(jpm_contexto_t *ctx, const char *ruta_config);

/* =============================================================================
 * FUNCIONES DE METADATOS
 * =============================================================================
 */

/* Crea nuevos metadatos con valores por defecto */
jpm_metadatos_t* jpm_metadatos_crear(const char *nombre, const char *version);

/* Libera memoria de metadatos */
void jpm_metadatos_liberar(jpm_metadatos_t *meta);

/* Lee metadatos desde archivo jasboot.json */
jpm_metadatos_t* jpm_metadatos_leer(const char *ruta_archivo);

/* Escribe metadatos a archivo jasboot.json */
int jpm_metadatos_escribir(const jpm_metadatos_t *meta, const char *ruta_archivo);

/* Valida metadatos */
bool jpm_metadatos_validar(const jpm_metadatos_t *meta, char *error, size_t error_len);

/* Agrega dependencia a metadatos */
int jpm_metadatos_agregar_dependencia(jpm_metadatos_t *meta, const char *nombre, 
                                      const char *version, bool es_desarrollo);

/* =============================================================================
 * FUNCIONES DE COMANDOS
 * =============================================================================
 */

/* jpm init <nombre> - Inicializa nuevo paquete */
int jpm_cmd_init(jpm_contexto_t *ctx, const char *nombre);

/* jpm pack - Empaqueta el proyecto actual */
int jpm_cmd_pack(jpm_contexto_t *ctx, const char *directorio);

/* jpm install <fuente> - Instala paquete desde archivo, URL o nombre */
int jpm_cmd_install(jpm_contexto_t *ctx, const char *fuente);

/* jpm install (sin args) - Instala todas las dependencias */
int jpm_cmd_install_todas(jpm_contexto_t *ctx);

/* jpm uninstall <paquete> - Desinstala paquete */
int jpm_cmd_uninstall(jpm_contexto_t *ctx, const char *nombre);

/* jpm list - Lista paquetes instalados */
int jpm_cmd_list(jpm_contexto_t *ctx);

/* jpm info <paquete> - Muestra información de paquete */
int jpm_cmd_info(jpm_contexto_t *ctx, const char *nombre);

/* jpm search <termino> - Busca paquetes en registro */
int jpm_cmd_search(jpm_contexto_t *ctx, const char *termino);

/* jpm publish - Publica paquete al registro */
int jpm_cmd_publish(jpm_contexto_t *ctx);

/* jpm update [paquete] - Actualiza paquete(s) */
int jpm_cmd_update(jpm_contexto_t *ctx, const char *nombre);

/* jpm clean - Limpia cache y archivos temporales */
int jpm_cmd_clean(jpm_contexto_t *ctx);

/* jpm run <script> - Ejecuta script definido */
int jpm_cmd_run(jpm_contexto_t *ctx, const char *script, int argc, char **argv);

/* =============================================================================
 * FUNCIONES DE INSTALACIÓN
 * =============================================================================
 */

/* Instala desde archivo local .jpkg */
int jpm_instalar_desde_archivo(jpm_contexto_t *ctx, const char *ruta_archivo);

/* Instala desde URL remota */
int jpm_instalar_desde_url(jpm_contexto_t *ctx, const char *url, 
                           jpm_callback_progreso_t callback, void *datos_usuario);

/* Instala desde registro por nombre */
int jpm_instalar_desde_registro(jpm_contexto_t *ctx, const char *nombre, const char *version);

/* Desinstala paquete */
int jpm_desinstalar_paquete(jpm_contexto_t *ctx, const char *nombre);

/* Resuelve árbol de dependencias */
int jpm_resolver_dependencias(jpm_contexto_t *ctx, jpm_metadatos_t *meta, 
                              jpm_lock_entrada_t **entradas, size_t *num_entradas);

/* =============================================================================
 * FUNCIONES DE EMPAQUETADO
 * =============================================================================
 */

/* Crea paquete .jpkg desde directorio */
int jpm_empaquetar(const char *directorio, const char *archivo_salida);

/* Extrae paquete .jpkg a directorio */
int jpm_extraer(const char *archivo_jpkg, const char *directorio_destino);

/* Calcula hash SHA-512 de archivo */
int jpm_calcular_hash(const char *archivo, char *hash_salida, size_t hash_len);

/* Verifica integridad de paquete */
bool jpm_verificar_integridad(const char *archivo, const char *hash_esperado);

/* =============================================================================
 * FUNCIONES DE RED (HTTP/HTTPS)
 * =============================================================================
 */

/* Descarga archivo desde URL */
int jpm_descargar_archivo(const char *url, const char *ruta_destino, 
                          jpm_callback_progreso_t callback, void *datos_usuario);

/* Realiza petición HTTP GET y retorna respuesta */
char* jpm_http_get(const char *url, int *codigo_estado);

/* Realiza petición HTTP POST */
char* jpm_http_post(const char *url, const char *datos, int *codigo_estado);

/* =============================================================================
 * FUNCIONES DE REGISTRO CENTRAL
 * =============================================================================
 */

/* Busca paquetes en registro */
jpm_metadatos_t** jpm_registro_buscar(jpm_contexto_t *ctx, const char *termino, 
                                      size_t *num_resultados);

/* Obtiene información de paquete del registro */
jpm_metadatos_t* jpm_registro_obtener_info(jpm_contexto_t *ctx, const char *nombre);

/* Publica paquete al registro */
int jpm_registro_publicar(jpm_contexto_t *ctx, const char *archivo_jpkg);

/* Autentica con el registro */
int jpm_registro_autenticar(jpm_contexto_t *ctx, const char *usuario, const char *password);

/* =============================================================================
 * FUNCIONES DE ARCHIVO LOCK
 * =============================================================================
 */

/* Crea nuevo archivo lock */
jpm_lock_t* jpm_lock_crear(void);

/* Libera memoria de lock */
void jpm_lock_liberar(jpm_lock_t *lock);

/* Lee archivo jpm.lock */
jpm_lock_t* jpm_lock_leer(const char *ruta_archivo);

/* Escribe archivo jpm.lock */
int jpm_lock_escribir(const jpm_lock_t *lock, const char *ruta_archivo);

/* Agrega entrada al lock */
int jpm_lock_agregar_entrada(jpm_lock_t *lock, const jpm_lock_entrada_t *entrada);

/* =============================================================================
 * FUNCIONES DE UTILIDADES
 * =============================================================================
 */

/* Sistema de logging */
void jpm_log(jpm_contexto_t *ctx, jpm_nivel_log_t nivel, const char *formato, ...);

/* Valida nombre de paquete */
bool jpm_validar_nombre(const char *nombre);

/* Valida versión semántica */
bool jpm_validar_version(const char *version);

/* Compara versiones semánticas (-1: v1<v2, 0: v1==v2, 1: v1>v2) */
int jpm_comparar_versiones(const char *v1, const char *v2);

/* Verifica si versión cumple con requisito (ej: "^1.2.3") */
bool jpm_version_cumple_requisito(const char *version, const char *requisito);

/* Obtiene directorio home del usuario */
const char* jpm_obtener_directorio_home(void);

/* Obtiene directorio de instalación global */
const char* jpm_obtener_directorio_global(void);

/* Crea directorio recursivamente */
int jpm_crear_directorio_recursivo(const char *ruta);

/* Copia directorio recursivamente */
int jpm_copiar_directorio(const char *origen, const char *destino);

/* Elimina directorio recursivamente */
int jpm_eliminar_directorio(const char *ruta);

/* Verifica si archivo existe */
bool jpm_archivo_existe(const char *ruta);

/* Verifica si es directorio */
bool jpm_es_directorio(const char *ruta);

/* Obtiene tamaño de archivo */
size_t jpm_obtener_tamano_archivo(const char *ruta);

/* Formatea bytes a string legible (KB, MB, etc) */
void jpm_formatear_bytes(size_t bytes, char *buffer, size_t buffer_len);

/* Lee archivo de ignorar (.jpmignore) */
char** jpm_leer_archivo_ignorar(const char *ruta, size_t *num_patrones);

/* Verifica si ruta debe ser ignorada */
bool jpm_debe_ignorar(const char *ruta, char **patrones, size_t num_patrones);

/* =============================================================================
 * FUNCIONES DE JSON
 * =============================================================================
 */

/* Lee archivo JSON completo */
void* jpm_json_leer_archivo(const char *ruta);

/* Escribe objeto JSON a archivo */
int jpm_json_escribir_archivo(void *objeto, const char *ruta);

/* Obtiene string de objeto JSON */
const char* jpm_json_obtener_string(void *objeto, const char *clave);

/* Obtiene número de objeto JSON */
int64_t jpm_json_obtener_numero(void *objeto, const char *clave);

/* Obtiene booleano de objeto JSON */
bool jpm_json_obtener_bool(void *objeto, const char *clave);

/* Obtiene array de objeto JSON */
void* jpm_json_obtener_array(void *objeto, const char *clave);

/* Obtiene objeto de objeto JSON */
void* jpm_json_obtener_objeto(void *objeto, const char *clave);

/* Libera objeto JSON */
void jpm_json_liberar(void *objeto);

/* =============================================================================
 * FUNCIONES DE VERSIONADO SEMÁNTICO
 * =============================================================================
 */

typedef struct {
    int mayor;
    int menor;
    int parche;
    char pre_release[64];
    char build[64];
} jpm_version_sem_t;

/* Parsea versión semántica */
int jpm_parsear_version_semantica(const char *version, jpm_version_sem_t *v);

/* Verifica si cumple con operador (^, ~, >=, etc) */
bool jpm_version_cumple_operador(const jpm_version_sem_t *version, 
                                 const char *operador, 
                                 const jpm_version_sem_t *requisito);

/* =============================================================================
 * MACROS DE UTILIDAD
 * =============================================================================
 */

#define JPM_LOG_INFO(ctx, ...) jpm_log(ctx, JPM_LOG_INFO, __VA_ARGS__)
#define JPM_LOG_AVISO(ctx, ...) jpm_log(ctx, JPM_LOG_AVISO, __VA_ARGS__)
#define JPM_LOG_ERROR(ctx, ...) jpm_log(ctx, JPM_LOG_ERROR, __VA_ARGS__)
#define JPM_LOG_DEBUG(ctx, ...) jpm_log(ctx, JPM_LOG_DEBUG, __VA_ARGS__)

#define JPM_VERIFICAR_MEMORIA(ptr) \
    do { \
        if (!(ptr)) { \
            fprintf(stderr, "[ERROR] Fallo de asignación de memoria en %s:%d\n", __FILE__, __LINE__); \
            return NULL; \
        } \
    } while(0)

#define JPM_VERIFICAR_MEMORIA_RET(ptr, retval) \
    do { \
        if (!(ptr)) { \
            fprintf(stderr, "[ERROR] Fallo de asignación de memoria en %s:%d\n", __FILE__, __LINE__); \
            return retval; \
        } \
    } while(0)

#endif /* JPM_H */