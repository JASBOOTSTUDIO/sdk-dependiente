/*
 * jpm_json.c
 * Parser JSON simple para Jasboot Package Manager
 * 
 * Soporte para objetos {}, arrays [], strings "", números, booleanos y null
 */

#include "../include/jpm.h"
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>

/* =============================================================================
 * TIPOS DE DATOS INTERNOS
 * =============================================================================
 */

typedef enum {
    JSON_OBJETO,
    JSON_ARRAY,
    JSON_STRING,
    JSON_NUMERO,
    JSON_BOOL,
    JSON_NULL
} json_tipo_t;

/* Par clave-valor para objetos */
typedef struct json_par_s {
    char *clave;
    struct json_valor_s *valor;
    struct json_par_s *siguiente;
} json_par_t;

/* Elemento de array */
typedef struct json_elemento_s {
    struct json_valor_s *valor;
    struct json_elemento_s *siguiente;
} json_elemento_t;

/* Valor JSON */
typedef struct json_valor_s {
    json_tipo_t tipo;
    union {
        json_par_t *pares;        /* Para objetos */
        json_elemento_t *elementos; /* Para arrays */
        char *cadena;              /* Para strings */
        double numero;             /* Para números */
        bool booleano;            /* Para booleanos */
    } datos;
    size_t longitud;  /* Número de pares/elementos */
} json_valor_t;

/* Contexto del parser */
typedef struct {
    const char *entrada;
    size_t posicion;
    size_t longitud;
    char error[512];
    int linea;
    int columna;
} json_parser_t;

/* =============================================================================
 * PROTOTIPOS INTERNOS
 * =============================================================================
 */

static json_valor_t* json_parsear_valor(json_parser_t *p);
static json_valor_t* json_parsear_objeto(json_parser_t *p);
static json_valor_t* json_parsear_array(json_parser_t *p);
static json_valor_t* json_parsear_string(json_parser_t *p);
static json_valor_t* json_parsear_numero(json_parser_t *p);
static json_valor_t* json_parsear_literal(json_parser_t *p);

static void json_saltar_espacios(json_parser_t *p);
static bool json_esperado(json_parser_t *p, char c);
static char json_mirar(json_parser_t *p);
static char json_avanzar(json_parser_t *p);

static int json_serializar_valor(json_valor_t *val, FILE *f, int indent);
static void json_escribir_indent(FILE *f, int nivel);

static void json_liberar_valor(json_valor_t *val);

/* =============================================================================
 * FUNCIONES DE PARSER
 * =============================================================================
 */

static void json_saltar_espacios(json_parser_t *p) {
    while (p->posicion < p->longitud) {
        char c = p->entrada[p->posicion];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            if (c == '\n') {
                p->linea++;
                p->columna = 1;
            } else {
                p->columna++;
            }
            p->posicion++;
        } else {
            break;
        }
    }
}

static char json_mirar(json_parser_t *p) {
    json_saltar_espacios(p);
    if (p->posicion >= p->longitud) {
        return '\0';
    }
    return p->entrada[p->posicion];
}

static char json_avanzar(json_parser_t *p) {
    if (p->posicion >= p->longitud) {
        return '\0';
    }
    char c = p->entrada[p->posicion++];
    p->columna++;
    return c;
}

static bool json_esperado(json_parser_t *p, char c) {
    json_saltar_espacios(p);
    if (p->posicion >= p->longitud || p->entrada[p->posicion] != c) {
        snprintf(p->error, sizeof(p->error), 
                "Se esperaba '%c' en línea %d, columna %d", c, p->linea, p->columna);
        return false;
    }
    p->posicion++;
    p->columna++;
    return true;
}

static json_valor_t* json_parsear_string(json_parser_t *p) {
    if (!json_esperado(p, '"')) {
        return NULL;
    }

    size_t inicio = p->posicion;
    size_t capacidad = 256;
    char *buffer = (char*)malloc(capacidad);
    if (!buffer) {
        snprintf(p->error, sizeof(p->error), "Error de memoria al parsear string");
        return NULL;
    }
    
    size_t len = 0;
    while (p->posicion < p->longitud && p->entrada[p->posicion] != '"') {
        char c = p->entrada[p->posicion];
        
        if (c == '\\') {
            p->posicion++;
            if (p->posicion >= p->longitud) {
                free(buffer);
                snprintf(p->error, sizeof(p->error), "String sin terminar");
                return NULL;
            }
            char escape = p->entrada[p->posicion];
            switch (escape) {
                case '"': c = '"'; break;
                case '\\': c = '\\'; break;
                case '/': c = '/'; break;
                case 'b': c = '\b'; break;
                case 'f': c = '\f'; break;
                case 'n': c = '\n'; break;
                case 'r': c = '\r'; break;
                case 't': c = '\t'; break;
                default:
                    c = escape;
            }
        }
        
        if (len >= capacidad - 1) {
            capacidad *= 2;
            char *nuevo = (char*)realloc(buffer, capacidad);
            if (!nuevo) {
                free(buffer);
                snprintf(p->error, sizeof(p->error), "Error de memoria al parsear string");
                return NULL;
            }
            buffer = nuevo;
        }
        
        buffer[len++] = c;
        p->posicion++;
        p->columna++;
    }
    
    if (!json_esperado(p, '"')) {
        free(buffer);
        return NULL;
    }
    
    buffer[len] = '\0';
    
    json_valor_t *val = (json_valor_t*)calloc(1, sizeof(json_valor_t));
    if (!val) {
        free(buffer);
        return NULL;
    }
    
    val->tipo = JSON_STRING;
    val->datos.cadena = buffer;
    return val;
}

static json_valor_t* json_parsear_numero(json_parser_t *p) {
    json_saltar_espacios(p);
    
    size_t inicio = p->posicion;
    
    if (p->entrada[p->posicion] == '-') {
        p->posicion++;
    }
    
    while (p->posicion < p->longitud && isdigit(p->entrada[p->posicion])) {
        p->posicion++;
    }
    
    if (p->posicion < p->longitud && p->entrada[p->posicion] == '.') {
        p->posicion++;
        while (p->posicion < p->longitud && isdigit(p->entrada[p->posicion])) {
            p->posicion++;
        }
    }
    
    if (p->posicion < p->longitud && 
        (p->entrada[p->posicion] == 'e' || p->entrada[p->posicion] == 'E')) {
        p->posicion++;
        if (p->posicion < p->longitud && 
            (p->entrada[p->posicion] == '+' || p->entrada[p->posicion] == '-')) {
            p->posicion++;
        }
        while (p->posicion < p->longitud && isdigit(p->entrada[p->posicion])) {
            p->posicion++;
        }
    }
    
    size_t longitud = p->posicion - inicio;
    char *num_str = (char*)malloc(longitud + 1);
    if (!num_str) {
        snprintf(p->error, sizeof(p->error), "Error de memoria al parsear número");
        return NULL;
    }
    
    memcpy(num_str, p->entrada + inicio, longitud);
    num_str[longitud] = '\0';
    
    double numero = atof(num_str);
    free(num_str);
    
    json_valor_t *val = (json_valor_t*)calloc(1, sizeof(json_valor_t));
    if (!val) {
        return NULL;
    }
    
    val->tipo = JSON_NUMERO;
    val->datos.numero = numero;
    return val;
}

static json_valor_t* json_parsear_literal(json_parser_t *p) {
    json_saltar_espacios(p);
    
    if (p->posicion + 4 <= p->longitud && 
        strncmp(p->entrada + p->posicion, "true", 4) == 0) {
        p->posicion += 4;
        json_valor_t *val = (json_valor_t*)calloc(1, sizeof(json_valor_t));
        if (val) {
            val->tipo = JSON_BOOL;
            val->datos.booleano = true;
        }
        return val;
    }
    
    if (p->posicion + 5 <= p->longitud && 
        strncmp(p->entrada + p->posicion, "false", 5) == 0) {
        p->posicion += 5;
        json_valor_t *val = (json_valor_t*)calloc(1, sizeof(json_valor_t));
        if (val) {
            val->tipo = JSON_BOOL;
            val->datos.booleano = false;
        }
        return val;
    }
    
    if (p->posicion + 4 <= p->longitud && 
        strncmp(p->entrada + p->posicion, "null", 4) == 0) {
        p->posicion += 4;
        json_valor_t *val = (json_valor_t*)calloc(1, sizeof(json_valor_t));
        if (val) {
            val->tipo = JSON_NULL;
        }
        return val;
    }
    
    snprintf(p->error, sizeof(p->error), "Literal desconocido en línea %d", p->linea);
    return NULL;
}

static json_valor_t* json_parsear_array(json_parser_t *p) {
    if (!json_esperado(p, '[')) {
        return NULL;
    }
    
    json_valor_t *val = (json_valor_t*)calloc(1, sizeof(json_valor_t));
    if (!val) {
        snprintf(p->error, sizeof(p->error), "Error de memoria al parsear array");
        return NULL;
    }
    
    val->tipo = JSON_ARRAY;
    val->datos.elementos = NULL;
    val->longitud = 0;
    
    json_elemento_t **actual = &val->datos.elementos;
    
    if (json_mirar(p) == ']') {
        json_avanzar(p);
        return val;
    }
    
    while (true) {
        json_valor_t *elemento_val = json_parsear_valor(p);
        if (!elemento_val) {
            json_liberar_valor(val);
            return NULL;
        }
        
        json_elemento_t *elem = (json_elemento_t*)calloc(1, sizeof(json_elemento_t));
        if (!elem) {
            json_liberar_valor(elemento_val);
            json_liberar_valor(val);
            snprintf(p->error, sizeof(p->error), "Error de memoria al parsear array");
            return NULL;
        }
        
        elem->valor = elemento_val;
        elem->siguiente = NULL;
        *actual = elem;
        actual = &elem->siguiente;
        val->longitud++;
        
        char siguiente = json_mirar(p);
        if (siguiente == ']') {
            json_avanzar(p);
            break;
        } else if (siguiente == ',') {
            json_avanzar(p);
        } else {
            json_liberar_valor(val);
            snprintf(p->error, sizeof(p->error), 
                    "Se esperaba ',' o ']' en línea %d", p->linea);
            return NULL;
        }
    }
    
    return val;
}

static json_valor_t* json_parsear_objeto(json_parser_t *p) {
    if (!json_esperado(p, '{')) {
        return NULL;
    }
    
    json_valor_t *val = (json_valor_t*)calloc(1, sizeof(json_valor_t));
    if (!val) {
        snprintf(p->error, sizeof(p->error), "Error de memoria al parsear objeto");
        return NULL;
    }
    
    val->tipo = JSON_OBJETO;
    val->datos.pares = NULL;
    val->longitud = 0;
    
    json_par_t **actual = &val->datos.pares;
    
    if (json_mirar(p) == '}') {
        json_avanzar(p);
        return val;
    }
    
    while (true) {
        json_valor_t *clave_val = json_parsear_string(p);
        if (!clave_val) {
            json_liberar_valor(val);
            return NULL;
        }
        
        if (!json_esperado(p, ':')) {
            json_liberar_valor(clave_val);
            json_liberar_valor(val);
            return NULL;
        }
        
        json_valor_t *valor_val = json_parsear_valor(p);
        if (!valor_val) {
            json_liberar_valor(clave_val);
            json_liberar_valor(val);
            return NULL;
        }
        
        json_par_t *par = (json_par_t*)calloc(1, sizeof(json_par_t));
        if (!par) {
            json_liberar_valor(clave_val);
            json_liberar_valor(valor_val);
            json_liberar_valor(val);
            snprintf(p->error, sizeof(p->error), "Error de memoria al parsear objeto");
            return NULL;
        }
        
        par->clave = clave_val->datos.cadena;
        clave_val->datos.cadena = NULL;  /* Transferir propiedad */
        json_liberar_valor(clave_val);
        
        par->valor = valor_val;
        par->siguiente = NULL;
        *actual = par;
        actual = &par->siguiente;
        val->longitud++;
        
        char siguiente = json_mirar(p);
        if (siguiente == '}') {
            json_avanzar(p);
            break;
        } else if (siguiente == ',') {
            json_avanzar(p);
        } else {
            json_liberar_valor(val);
            snprintf(p->error, sizeof(p->error), 
                    "Se esperaba ',' o '}' en línea %d", p->linea);
            return NULL;
        }
    }
    
    return val;
}

static json_valor_t* json_parsear_valor(json_parser_t *p) {
    char c = json_mirar(p);
    
    if (c == '{') {
        return json_parsear_objeto(p);
    } else if (c == '[') {
        return json_parsear_array(p);
    } else if (c == '"') {
        return json_parsear_string(p);
    } else if (c == '-' || isdigit(c)) {
        return json_parsear_numero(p);
    } else if (c == 't' || c == 'f' || c == 'n') {
        return json_parsear_literal(p);
    } else {
        snprintf(p->error, sizeof(p->error), 
                "Carácter inesperado '%c' en línea %d", c, p->linea);
        return NULL;
    }
}

/* =============================================================================
 * FUNCIONES DE SERIALIZACIÓN
 * =============================================================================
 */

static void json_escribir_indent(FILE *f, int nivel) {
    for (int i = 0; i < nivel; i++) {
        fprintf(f, "  ");
    }
}

static void json_escapar_string(const char *str, FILE *f) {
    fputc('"', f);
    while (*str) {
        switch (*str) {
            case '"': fprintf(f, "\\\""); break;
            case '\\': fprintf(f, "\\\\"); break;
            case '\b': fprintf(f, "\\b"); break;
            case '\f': fprintf(f, "\\f"); break;
            case '\n': fprintf(f, "\\n"); break;
            case '\r': fprintf(f, "\\r"); break;
            case '\t': fprintf(f, "\\t"); break;
            default:
                if ((unsigned char)*str < 32) {
                    fprintf(f, "\\u%04x", (unsigned char)*str);
                } else {
                    fputc(*str, f);
                }
        }
        str++;
    }
    fputc('"', f);
}

static int json_serializar_valor(json_valor_t *val, FILE *f, int indent) {
    if (!val) {
        fprintf(f, "null");
        return 0;
    }
    
    switch (val->tipo) {
        case JSON_NULL:
            fprintf(f, "null");
            break;
            
        case JSON_BOOL:
            fprintf(f, val->datos.booleano ? "true" : "false");
            break;
            
        case JSON_NUMERO:
            if (val->datos.numero == (int64_t)val->datos.numero) {
                fprintf(f, "%lld", (int64_t)val->datos.numero);
            } else {
                fprintf(f, "%.17g", val->datos.numero);
            }
            break;
            
        case JSON_STRING:
            json_escapar_string(val->datos.cadena ? val->datos.cadena : "", f);
            break;
            
        case JSON_ARRAY:
            fprintf(f, "[");
            if (val->longitud > 0) {
                fprintf(f, "\n");
                json_elemento_t *elem = val->datos.elementos;
                bool primero = true;
                while (elem) {
                    if (!primero) {
                        fprintf(f, ",\n");
                    }
                    json_escribir_indent(f, indent + 1);
                    json_serializar_valor(elem->valor, f, indent + 1);
                    elem = elem->siguiente;
                    primero = false;
                }
                fprintf(f, "\n");
                json_escribir_indent(f, indent);
            }
            fprintf(f, "]");
            break;
            
        case JSON_OBJETO:
            fprintf(f, "{");
            if (val->longitud > 0) {
                fprintf(f, "\n");
                json_par_t *par = val->datos.pares;
                bool primero = true;
                while (par) {
                    if (!primero) {
                        fprintf(f, ",\n");
                    }
                    json_escribir_indent(f, indent + 1);
                    json_escapar_string(par->clave, f);
                    fprintf(f, ": ");
                    json_serializar_valor(par->valor, f, indent + 1);
                    par = par->siguiente;
                    primero = false;
                }
                fprintf(f, "\n");
                json_escribir_indent(f, indent);
            }
            fprintf(f, "}");
            break;
    }
    
    return 0;
}

/* =============================================================================
 * FUNCIONES DE LIBERACIÓN DE MEMORIA
 * =============================================================================
 */

static void json_liberar_valor(json_valor_t *val) {
    if (!val) {
        return;
    }
    
    switch (val->tipo) {
        case JSON_STRING:
            free(val->datos.cadena);
            break;
            
        case JSON_ARRAY: {
            json_elemento_t *elem = val->datos.elementos;
            while (elem) {
                json_elemento_t *siguiente = elem->siguiente;
                json_liberar_valor(elem->valor);
                free(elem);
                elem = siguiente;
            }
            break;
        }
        
        case JSON_OBJETO: {
            json_par_t *par = val->datos.pares;
            while (par) {
                json_par_t *siguiente = par->siguiente;
                free(par->clave);
                json_liberar_valor(par->valor);
                free(par);
                par = siguiente;
            }
            break;
        }
        
        default:
            break;
    }
    
    free(val);
}

/* =============================================================================
 * API PÚBLICA
 * =============================================================================
 */

void* jpm_json_leer_archivo(const char *ruta) {
    if (!ruta) {
        fprintf(stderr, "[ERROR JPM JSON] Ruta de archivo NULL\n");
        return NULL;
    }
    
    FILE *f = fopen(ruta, "rb");
    if (!f) {
        fprintf(stderr, "[ERROR JPM JSON] No se pudo abrir archivo: %s - %s\n", 
                ruta, strerror(errno));
        return NULL;
    }
    
    /* Obtener tamaño del archivo */
    fseek(f, 0, SEEK_END);
    long tamano = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if (tamano <= 0) {
        fclose(f);
        fprintf(stderr, "[ERROR JPM JSON] Archivo vacío o inválido: %s\n", ruta);
        return NULL;
    }
    
    /* Leer contenido */
    char *contenido = (char*)malloc(tamano + 1);
    if (!contenido) {
        fclose(f);
        fprintf(stderr, "[ERROR JPM JSON] Error de memoria al leer archivo\n");
        return NULL;
    }
    
    size_t leidos = fread(contenido, 1, tamano, f);
    fclose(f);
    
    if (leidos != (size_t)tamano) {
        free(contenido);
        fprintf(stderr, "[ERROR JPM JSON] Error al leer archivo: %s\n", ruta);
        return NULL;
    }
    
    contenido[tamano] = '\0';
    
    /* Parsear JSON */
    json_parser_t parser = {
        .entrada = contenido,
        .posicion = 0,
        .longitud = tamano,
        .linea = 1,
        .columna = 1
    };
    parser.error[0] = '\0';
    
    json_valor_t *raiz = json_parsear_valor(&parser);
    
    free(contenido);
    
    if (!raiz) {
        fprintf(stderr, "[ERROR JPM JSON] Error al parsear JSON: %s\n", 
                parser.error[0] ? parser.error : "Error desconocido");
        return NULL;
    }
    
    return (void*)raiz;
}

int jpm_json_escribir_archivo(void *objeto, const char *ruta) {
    if (!objeto || !ruta) {
        fprintf(stderr, "[ERROR JPM JSON] Parámetros NULL en escribir_archivo\n");
        return JPM_ERROR_VALIDACION;
    }
    
    FILE *f = fopen(ruta, "w");
    if (!f) {
        fprintf(stderr, "[ERROR JPM JSON] No se pudo crear archivo: %s - %s\n", 
                ruta, strerror(errno));
        return JPM_ERROR_ARCHIVO;
    }
    
    json_valor_t *val = (json_valor_t*)objeto;
    int resultado = json_serializar_valor(val, f, 0);
    fprintf(f, "\n");
    
    fclose(f);
    return resultado == 0 ? JPM_EXITO : JPM_ERROR_JSON;
}

const char* jpm_json_obtener_string(void *objeto, const char *clave) {
    if (!objeto || !clave) {
        return NULL;
    }
    
    json_valor_t *val = (json_valor_t*)objeto;
    if (val->tipo != JSON_OBJETO) {
        return NULL;
    }
    
    json_par_t *par = val->datos.pares;
    while (par) {
        if (strcmp(par->clave, clave) == 0) {
            if (par->valor && par->valor->tipo == JSON_STRING) {
                return par->valor->datos.cadena;
            }
            return NULL;
        }
        par = par->siguiente;
    }
    
    return NULL;
}

int64_t jpm_json_obtener_numero(void *objeto, const char *clave) {
    if (!objeto || !clave) {
        return 0;
    }
    
    json_valor_t *val = (json_valor_t*)objeto;
    if (val->tipo != JSON_OBJETO) {
        return 0;
    }
    
    json_par_t *par = val->datos.pares;
    while (par) {
        if (strcmp(par->clave, clave) == 0) {
            if (par->valor && par->valor->tipo == JSON_NUMERO) {
                return (int64_t)par->valor->datos.numero;
            }
            return 0;
        }
        par = par->siguiente;
    }
    
    return 0;
}

bool jpm_json_obtener_bool(void *objeto, const char *clave) {
    if (!objeto || !clave) {
        return false;
    }
    
    json_valor_t *val = (json_valor_t*)objeto;
    if (val->tipo != JSON_OBJETO) {
        return false;
    }
    
    json_par_t *par = val->datos.pares;
    while (par) {
        if (strcmp(par->clave, clave) == 0) {
            if (par->valor && par->valor->tipo == JSON_BOOL) {
                return par->valor->datos.booleano;
            }
            return false;
        }
        par = par->siguiente;
    }
    
    return false;
}

void* jpm_json_obtener_array(void *objeto, const char *clave) {
    if (!objeto || !clave) {
        return NULL;
    }
    
    json_valor_t *val = (json_valor_t*)objeto;
    if (val->tipo != JSON_OBJETO) {
        return NULL;
    }
    
    json_par_t *par = val->datos.pares;
    while (par) {
        if (strcmp(par->clave, clave) == 0) {
            if (par->valor && par->valor->tipo == JSON_ARRAY) {
                return (void*)par->valor;
            }
            return NULL;
        }
        par = par->siguiente;
    }
    
    return NULL;
}

void* jpm_json_obtener_objeto(void *objeto, const char *clave) {
    if (!objeto || !clave) {
        return NULL;
    }
    
    json_valor_t *val = (json_valor_t*)objeto;
    if (val->tipo != JSON_OBJETO) {
        return NULL;
    }
    
    json_par_t *par = val->datos.pares;
    while (par) {
        if (strcmp(par->clave, clave) == 0) {
            if (par->valor && par->valor->tipo == JSON_OBJETO) {
                return (void*)par->valor;
            }
            return NULL;
        }
        par = par->siguiente;
    }
    
    return NULL;
}

void jpm_json_liberar(void *objeto) {
    if (!objeto) {
        return;
    }
    json_liberar_valor((json_valor_t*)objeto);
}