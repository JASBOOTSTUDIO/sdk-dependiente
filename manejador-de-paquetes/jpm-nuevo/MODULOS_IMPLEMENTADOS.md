# Módulos Implementados - Jasboot Package Manager (JPM)

## Descripción General

Este documento describe los módulos implementados para el Jasboot Package Manager (JPM).
Los módulos están escritos en C11 y siguen las convenciones del proyecto Jasboot.

**Fecha de Implementación:** 2024  
**Versión JPM:** 0.2.0  
**Ubicación:** `jasboot/sdk-dependiente/manejador-de-paquetes/jpm-nuevo/src/`

---

## Módulos Implementados

### 1. jpm_json.c - Parser JSON Simple

Parser JSON completo implementado desde cero sin dependencias externas.

#### Características
- Soporte para objetos `{}`
- Soporte para arrays `[]`
- Soporte para strings `""`
- Soporte para números (enteros y decimales)
- Soporte para booleanos (`true`, `false`)
- Soporte para `null`
- Escapado de caracteres especiales en strings
- Serialización con formato indentado (pretty-print)

#### Funciones Implementadas

```c
void* jpm_json_leer_archivo(const char *ruta);
int jpm_json_escribir_archivo(void *objeto, const char *ruta);
const char* jpm_json_obtener_string(void *objeto, const char *clave);
int64_t jpm_json_obtener_numero(void *objeto, const char *clave);
bool jpm_json_obtener_bool(void *objeto, const char *clave);
void* jpm_json_obtener_array(void *objeto, const char *clave);
void* jpm_json_obtener_objeto(void *objeto, const char *clave);
void jpm_json_liberar(void *objeto);
```

#### Ejemplo de Uso

```c
// Leer archivo JSON
void *json = jpm_json_leer_archivo("jasboot.json");
if (!json) {
    fprintf(stderr, "Error al leer JSON\n");
    return -1;
}

// Obtener valores
const char *nombre = jpm_json_obtener_string(json, "nombre");
const char *version = jpm_json_obtener_string(json, "version");
bool es_privado = jpm_json_obtener_bool(json, "privado");

// Obtener objeto anidado
void *deps = jpm_json_obtener_objeto(json, "dependencias");
if (deps) {
    // Procesar dependencias...
}

// Liberar memoria
jpm_json_liberar(json);
```

#### Estructuras Internas

```c
typedef enum {
    JSON_OBJETO,
    JSON_ARRAY,
    JSON_STRING,
    JSON_NUMERO,
    JSON_BOOL,
    JSON_NULL
} json_tipo_t;

typedef struct json_valor_s {
    json_tipo_t tipo;
    union {
        json_par_t *pares;        // Para objetos
        json_elemento_t *elementos; // Para arrays
        char *cadena;              // Para strings
        double numero;             // Para números
        bool booleano;            // Para booleanos
    } datos;
    size_t longitud;
} json_valor_t;
```

---

### 2. jpm_metadata.c - Gestión de Metadatos

Gestiona la creación, lectura, escritura y validación de archivos `jasboot.json`.

#### Funciones Implementadas

```c
jpm_metadatos_t* jpm_metadatos_crear(const char *nombre, const char *version);
void jpm_metadatos_liberar(jpm_metadatos_t *meta);
jpm_metadatos_t* jpm_metadatos_leer(const char *ruta_archivo);
int jpm_metadatos_escribir(const jpm_metadatos_t *meta, const char *ruta_archivo);
bool jpm_metadatos_validar(const jpm_metadatos_t *meta, char *error, size_t error_len);
int jpm_metadatos_agregar_dependencia(jpm_metadatos_t *meta, const char *nombre, 
                                      const char *version, bool es_desarrollo);
```

#### Campos del Archivo jasboot.json

**Obligatorios:**
- `nombre` - Nombre del paquete (validado)
- `version` - Versión semántica (X.Y.Z)

**Opcionales:**
- `descripcion` - Descripción del paquete
- `autor` - Nombre del autor
- `licencia` - Tipo de licencia (default: MIT)
- `principal` - Archivo principal (default: src/main.jasb)
- `jasboot` - Versión de Jasboot requerida (default: ^0.1.0)
- `tipo` - Tipo de paquete: biblioteca, aplicacion, plugin, herramienta
- `repositorio` - URL del repositorio
- `homepage` - URL de la página web
- `palabrasClave` - Array de palabras clave
- `dependencias` - Objeto con dependencias de producción
- `dependenciasDev` - Objeto con dependencias de desarrollo
- `scripts` - Objeto con scripts personalizados

#### Ejemplo de jasboot.json

```json
{
  "nombre": "mi-paquete",
  "version": "1.0.0",
  "descripcion": "Un paquete de ejemplo",
  "autor": "Tu Nombre",
  "licencia": "MIT",
  "principal": "src/main.jasb",
  "jasboot": "^0.1.0",
  "tipo": "biblioteca",
  "repositorio": "https://github.com/usuario/mi-paquete",
  "homepage": "https://mi-paquete.com",
  "palabrasClave": ["jasboot", "ejemplo"],
  "dependencias": {
    "otro-paquete": "^1.2.3"
  },
  "dependenciasDev": {
    "test-framework": "~2.0.0"
  },
  "scripts": {
    "test": "jasboot tests/test.jasb",
    "build": "jpm pack"
  }
}
```

#### Ejemplo de Uso

```c
// Crear metadatos nuevos
jpm_metadatos_t *meta = jpm_metadatos_crear("mi-paquete", "1.0.0");
if (!meta) {
    fprintf(stderr, "Error al crear metadatos\n");
    return -1;
}

// Configurar metadatos
strcpy(meta->descripcion, "Mi paquete de ejemplo");
strcpy(meta->autor, "Mi Nombre");
meta->tipo = JPM_TIPO_BIBLIOTECA;

// Agregar dependencia
jpm_metadatos_agregar_dependencia(meta, "otro-paquete", "^1.2.3", false);

// Validar
char error[512];
if (!jpm_metadatos_validar(meta, error, sizeof(error))) {
    fprintf(stderr, "Metadatos inválidos: %s\n", error);
    jpm_metadatos_liberar(meta);
    return -1;
}

// Escribir a archivo
int resultado = jpm_metadatos_escribir(meta, "jasboot.json");
if (resultado != JPM_EXITO) {
    fprintf(stderr, "Error al escribir archivo\n");
}

// Liberar
jpm_metadatos_liberar(meta);
```

#### Validaciones Implementadas

- ✅ Nombre de paquete válido (solo letras minúsculas, números, guiones y guiones bajos)
- ✅ Versión semántica válida (formato X.Y.Z)
- ✅ Versión Jasboot válida (con operadores ^, ~, >=, etc)
- ✅ Longitud de strings dentro de límites
- ✅ Dependencias con nombres válidos

---

### 3. jpm_commands.c - Comandos CLI

Implementa los comandos de la interfaz de línea de comandos de JPM.

#### Comandos Implementados

##### 1. `jpm init <nombre>`

Inicializa un nuevo paquete Jasboot con estructura completa.

**Crea:**
- `jasboot.json` - Metadatos del paquete
- `README.md` - Documentación básica
- `.gitignore` - Ignorar dependencias y cache
- `.jpmignore` - Patrones para empaquetar
- `src/main.jasb` - Archivo principal con código de ejemplo
- `tests/` - Directorio para tests
- `docs/` - Directorio para documentación

**Ejemplo:**
```bash
jpm init mi-paquete
```

**Salida:**
```
[INFO] Inicializando paquete 'mi-paquete'...
[INFO] Creando estructura de directorios...
[INFO] Creando archivo principal...
[INFO] Creando README.md...
[INFO] Creando .gitignore...
[INFO] Creando .jpmignore...
[INFO] Creando jasboot.json...

✓ Paquete 'mi-paquete' inicializado correctamente

Estructura creada:
  jasboot.json
  README.md
  .gitignore
  .jpmignore
  src/main.jasb
  tests/
  docs/

Próximos pasos:
  1. Edita src/main.jasb con tu código
  2. Actualiza README.md con documentación
  3. Ejecuta 'jpm install' para instalar dependencias
  4. Ejecuta 'jpm pack' para empaquetar tu código
```

##### 2. `jpm list`

Lista todos los paquetes instalados en `a-modulos/`.

**Ejemplo:**
```bash
jpm list
```

**Salida:**
```
[INFO] Paquetes instalados en a-modulos:

  - libreria-math@1.2.3 - Funciones matemáticas avanzadas
  - utilidades-texto@2.0.1 - Utilidades para manipulación de texto
  - http-client@0.5.0 - Cliente HTTP simple

Total: 3 paquetes instalados
```

##### 3. `jpm info <paquete>`

Muestra información detallada de un paquete instalado.

**Ejemplo:**
```bash
jpm info libreria-math
```

**Salida:**
```
=== libreria-math ===

Versión:      1.2.3
Descripción:  Funciones matemáticas avanzadas
Autor:        Equipo Jasboot
Licencia:     MIT
Tipo:         biblioteca
Principal:    src/main.jasb
Jasboot:      ^0.1.0
Repositorio:  https://github.com/jasboot/libreria-math
Homepage:     https://jasboot.org/paquetes/libreria-math

Dependencias:
  - utilidades-comunes@^1.0.0

Palabras clave:
  matematicas, algebra, geometria

Instalación:
  Ruta:   a-modulos/libreria-math
  Tamaño: 245.67 KB
```

##### 4. `jpm clean`

Limpia el cache y archivos temporales de JPM.

**Elimina:**
- `.jpm-cache/` - Cache de descargas
- `.jpm-temp/` - Archivos temporales

**Ejemplo:**
```bash
jpm clean
```

**Salida:**
```
[INFO] Limpiando cache de JPM...

[INFO] Limpiando .jpm-cache...
  ✓ Eliminado 15.34 MB
[INFO] Limpiando .jpm-temp...
  ✓ Eliminado 2.45 MB

✓ Limpieza completada
  Espacio liberado: 17.79 MB
  Elementos eliminados: 2
```

##### 5. `jpm uninstall <paquete>`

Desinstala un paquete de `a-modulos/`.

**Características:**
- Elimina el directorio del paquete recursivamente
- Muestra advertencia si el paquete está en `jasboot.json`
- Calcula espacio liberado
- Muestra versión del paquete desinstalado

**Ejemplo:**
```bash
jpm uninstall libreria-math
```

**Salida:**
```
[INFO] Desinstalando paquete 'libreria-math'...
[AVISO] AVISO: 'libreria-math' está listado en jasboot.json
[AVISO]        Debes eliminarlo manualmente del archivo si ya no lo necesitas.
[INFO] Eliminando a-modulos/libreria-math (245.67 KB)...

✓ Paquete 'libreria-math@1.2.3' desinstalado correctamente
  Espacio liberado: 245.67 KB
```

#### Funciones Implementadas

```c
int jpm_cmd_init(jpm_contexto_t *ctx, const char *nombre);
int jpm_cmd_list(jpm_contexto_t *ctx);
int jpm_cmd_info(jpm_contexto_t *ctx, const char *nombre);
int jpm_cmd_clean(jpm_contexto_t *ctx);
int jpm_cmd_uninstall(jpm_contexto_t *ctx, const char *nombre);
```

#### Funciones Auxiliares Internas

```c
static int crear_archivo_con_contenido(const char *ruta, const char *contenido);
static bool archivo_existe(const char *ruta);
static bool es_directorio(const char *ruta);
static size_t obtener_tamano_archivo(const char *ruta);
static void formatear_bytes(size_t bytes, char *buffer, size_t buffer_len);
static int eliminar_archivo(const char *ruta);
static int eliminar_directorio_recursivo(const char *ruta);
static size_t calcular_tamano_directorio(const char *ruta);
```

---

## Integración con jpm_core.c

Los módulos implementados utilizan funciones de `jpm_core.c`:

### Funciones del Core Utilizadas

```c
// Logging
void jpm_log(jpm_contexto_t *ctx, jpm_nivel_log_t nivel, const char *formato, ...);

// Validación
bool jpm_validar_nombre(const char *nombre);
bool jpm_validar_version(const char *version);
int jpm_comparar_versiones(const char *v1, const char *v2);
bool jpm_version_cumple_requisito(const char *version, const char *requisito);

// Sistema de archivos
bool jpm_archivo_existe(const char *ruta);
bool jpm_es_directorio(const char *ruta);
size_t jpm_obtener_tamano_archivo(const char *ruta);
void jpm_formatear_bytes(size_t bytes, char *buffer, size_t buffer_len);
int jpm_crear_directorio_recursivo(const char *ruta);

// Directorios del sistema
const char* jpm_obtener_directorio_home(void);
const char* jpm_obtener_directorio_global(void);

// Contexto
jpm_contexto_t* jpm_inicializar(void);
void jpm_liberar(jpm_contexto_t *ctx);
```

---

## Estructura de Directorios

```
jpm-nuevo/
├── include/
│   └── jpm.h                 # Header principal con todas las definiciones
├── src/
│   ├── jpm_core.c            # Funciones core (ya existente)
│   ├── jpm_json.c            # ✅ Parser JSON (NUEVO)
│   ├── jpm_metadata.c        # ✅ Gestión de metadatos (NUEVO)
│   ├── jpm_commands.c        # ✅ Comandos CLI (NUEVO)
│   └── jpm_main.c            # Punto de entrada (ya existente)
└── MODULOS_IMPLEMENTADOS.md  # Este archivo
```

---

## Códigos de Error

Los módulos utilizan los siguientes códigos de error definidos en `jpm.h`:

```c
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
```

---

## Compatibilidad

### Plataformas Soportadas
- ✅ Windows (con MinGW/MSVC)
- ✅ Linux
- ✅ macOS

### Dependencias
- C11 estándar
- `dirent.h` para manejo de directorios
- `sys/stat.h` para información de archivos
- Sin dependencias externas (JSON implementado desde cero)

---

## Próximos Pasos (TODO)

### Módulos Pendientes

1. **jpm_install.c** - Instalación de paquetes
   - `jpm_cmd_install()`
   - `jpm_cmd_install_todas()`
   - `jpm_instalar_desde_archivo()`
   - `jpm_instalar_desde_url()`
   - `jpm_instalar_desde_registro()`

2. **jpm_pack.c** - Empaquetado
   - `jpm_cmd_pack()`
   - `jpm_empaquetar()`
   - `jpm_extraer()`

3. **jpm_registry.c** - Interacción con registro
   - `jpm_cmd_search()`
   - `jpm_cmd_publish()`
   - `jpm_registro_buscar()`
   - `jpm_registro_obtener_info()`

4. **jpm_http.c** - Cliente HTTP/HTTPS
   - `jpm_descargar_archivo()`
   - `jpm_http_get()`
   - `jpm_http_post()`

5. **jpm_lock.c** - Gestión de jpm.lock
   - `jpm_lock_crear()`
   - `jpm_lock_leer()`
   - `jpm_lock_escribir()`

### Mejoras Futuras

- Soporte completo para arrays de strings en JSON
- Parsing de dependencias desde objetos JSON
- Validación de integridad con SHA-512
- Compresión de paquetes (.jpkg)
- Resolución de dependencias transitivas
- Actualización de paquetes
- Ejecución de scripts personalizados

---

## Testing

### Compilación

```bash
cd sdk-dependiente/manejador-de-paquetes/jpm-nuevo
gcc -o jpm \
    src/jpm_main.c \
    src/jpm_core.c \
    src/jpm_json.c \
    src/jpm_metadata.c \
    src/jpm_commands.c \
    -I include \
    -std=c11 \
    -Wall -Wextra
```

### Pruebas Básicas

```bash
# Inicializar paquete
./jpm init test-package

# Ver estructura creada
ls -la

# Verificar jasboot.json
cat jasboot.json

# Limpiar cache
./jpm clean

# Listar paquetes (debería estar vacío)
./jpm list
```

---

## Contribuciones

Al modificar o extender estos módulos, asegúrate de:

1. ✅ Seguir el estilo de código C11
2. ✅ Usar las constantes definidas en `jpm.h`
3. ✅ Manejar errores con códigos `JPM_ERROR_*`
4. ✅ Usar `jpm_log()` para mensajes
5. ✅ Validar todos los parámetros de entrada
6. ✅ Liberar toda la memoria asignada
7. ✅ Documentar funciones con comentarios
8. ✅ Usar macros `JPM_VERIFICAR_MEMORIA` para validación
9. ✅ Mantener compatibilidad multiplataforma (Windows/Linux/macOS)
10. ✅ Escribir tests para nuevas funcionalidades

---

## Licencia

Este código es parte del proyecto Jasboot y está bajo la misma licencia del proyecto principal.

---

## Contacto

Para reportar bugs o sugerir mejoras en estos módulos, contacta al equipo de desarrollo de Jasboot.

**Última actualización:** 2024