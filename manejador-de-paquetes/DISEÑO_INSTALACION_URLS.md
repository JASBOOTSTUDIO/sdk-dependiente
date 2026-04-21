# 🌐 DISEÑO: INSTALACIÓN DESDE URLs - JPM

**Documento de diseño técnico para implementar instalación de paquetes desde URLs**

Versión: 1.0  
Fecha: 20 de Abril 2024  
Autor: Equipo Jasboot

---

## 📋 ÍNDICE

1. [Introducción](#introducción)
2. [Opciones de Implementación](#opciones-de-implementación)
3. [Opción 1: URLs Directas a .jpkg](#opción-1-urls-directas-a-jpkg)
4. [Opción 2: GitHub Releases](#opción-2-github-releases)
5. [Opción 3: GitHub Repositorios](#opción-3-github-repositorios)
6. [Opción 4: Registro Central](#opción-4-registro-central)
7. [Comparación y Recomendación](#comparación-y-recomendación)
8. [Plan de Implementación](#plan-de-implementación)

---

## 🎯 INTRODUCCIÓN

### Objetivo

Permitir a los usuarios instalar paquetes Jasboot desde internet sin necesidad de descargar manualmente los archivos.

### Estado Actual

✅ **FUNCIONANDO:**
- Instalación desde archivos locales: `jpm install ./paquete.jpkg`
- Instalación desde rutas relativas: `jpm install ../otro-proyecto/paquete.jpkg`

❌ **NO IMPLEMENTADO:**
- Instalación desde URLs HTTP/HTTPS
- Instalación desde repositorios Git
- Registro central de paquetes

### ¿Por qué es importante?

```bash
# Actualmente (tedioso):
1. Ir a GitHub
2. Descargar archivo .jpkg
3. jpm install C:\Downloads\paquete.jpkg

# Con URLs (simple):
jpm install https://github.com/user/repo/releases/download/v1.0.0/paquete.jpkg
```

---

## 🔍 OPCIONES DE IMPLEMENTACIÓN

### Resumen de Opciones

| # | Tipo | Ejemplo | Complejidad | Infraestructura |
|---|------|---------|-------------|-----------------|
| 1 | URL Directa a .jpkg | `https://ejemplo.com/paquete.jpkg` | ⭐ Baja | Ninguna |
| 2 | GitHub Releases | `github:usuario/repo@v1.0.0` | ⭐⭐ Media | Ninguna |
| 3 | GitHub Repo | `git+https://github.com/user/repo` | ⭐⭐⭐ Media-Alta | Ninguna |
| 4 | Registro Central | `jpm install calculadora` | ⭐⭐⭐⭐⭐ Alta | Servidor propio |

---

## 📦 OPCIÓN 1: URLs DIRECTAS A .jpkg

### Descripción

Descargar archivos `.jpkg` desde cualquier URL HTTP/HTTPS.

### Casos de Uso

```bash
# Desde un servidor web
jpm install https://mi-servidor.com/paquetes/calculadora-1.0.0.jpkg

# Desde GitHub Releases
jpm install https://github.com/jasboot/calculadora/releases/download/v1.0.0/calculadora-1.0.0.jpkg

# Desde Dropbox (link directo)
jpm install https://dl.dropboxusercontent.com/s/xxxxx/paquete.jpkg

# Desde Google Drive (link directo)
jpm install https://drive.google.com/uc?export=download&id=xxxxx

# Desde cualquier CDN
jpm install https://cdn.jasboot.org/packages/paquete-1.0.0.jpkg
```

### Flujo de Trabajo

```
Usuario: jpm install https://ejemplo.com/paquete.jpkg

    ↓
┌─────────────────────────────────────┐
│ 1. Parsear URL                      │
│    - Validar formato                │
│    - Extraer nombre de archivo      │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 2. Descargar archivo                │
│    - HTTP GET request               │
│    - Mostrar progreso               │
│    - Guardar en cache               │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 3. Verificar archivo                │
│    - Es ZIP válido?                 │
│    - Contiene jasboot.json?         │
│    - Hash correcto? (si existe)     │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 4. Instalar                         │
│    - Extraer a a-modulos/           │
│    - Actualizar jpm.lock            │
│    - Instalar dependencias          │
└─────────────────────────────────────┘
    ↓
✅ Instalación completa
```

### Implementación Técnica

#### Estructura de Código

```c
// jpm_http.c - Cliente HTTP simple

typedef struct {
    char *url;
    char *archivo_destino;
    size_t bytes_descargados;
    size_t bytes_totales;
    jpm_callback_progreso_t callback;
    void *datos_usuario;
} jpm_descarga_t;

// Función principal de descarga
int jpm_descargar_archivo(const char *url, const char *destino,
                          jpm_callback_progreso_t callback, void *datos) {
    
    // 1. Parsear URL
    jpm_url_t url_info;
    if (jpm_parsear_url(url, &url_info) != JPM_EXITO) {
        return JPM_ERROR_VALIDACION;
    }
    
    // 2. Conectar al servidor
    int socket = jpm_http_conectar(url_info.host, url_info.puerto);
    if (socket < 0) {
        return JPM_ERROR_RED;
    }
    
    // 3. Enviar petición HTTP GET
    char request[2048];
    snprintf(request, sizeof(request),
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-Agent: jpm/%s\r\n"
        "Connection: close\r\n"
        "\r\n",
        url_info.ruta, url_info.host, JPM_VERSION);
    
    if (send(socket, request, strlen(request), 0) < 0) {
        close(socket);
        return JPM_ERROR_RED;
    }
    
    // 4. Leer respuesta
    FILE *archivo = fopen(destino, "wb");
    if (!archivo) {
        close(socket);
        return JPM_ERROR_ARCHIVO;
    }
    
    char buffer[8192];
    size_t bytes_leidos;
    size_t total = 0;
    
    // Leer headers primero
    int headers_completos = 0;
    while (!headers_completos) {
        bytes_leidos = recv(socket, buffer, sizeof(buffer), 0);
        if (bytes_leidos <= 0) break;
        
        // Buscar fin de headers (\r\n\r\n)
        if (strstr(buffer, "\r\n\r\n")) {
            headers_completos = 1;
            // Escribir solo el cuerpo
            char *cuerpo = strstr(buffer, "\r\n\r\n") + 4;
            fwrite(cuerpo, 1, bytes_leidos - (cuerpo - buffer), archivo);
        }
    }
    
    // Leer cuerpo
    while ((bytes_leidos = recv(socket, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, bytes_leidos, archivo);
        total += bytes_leidos;
        
        // Callback de progreso
        if (callback) {
            callback(total, 0, datos); // 0 = tamaño desconocido
        }
    }
    
    fclose(archivo);
    close(socket);
    
    return JPM_EXITO;
}
```

#### Alternativa con libcurl (Más robusto)

```c
// Si se usa libcurl (biblioteca externa)
#include <curl/curl.h>

int jpm_descargar_archivo_curl(const char *url, const char *destino,
                                jpm_callback_progreso_t callback, void *datos) {
    
    CURL *curl = curl_easy_init();
    if (!curl) {
        return JPM_ERROR_GENERICO;
    }
    
    FILE *archivo = fopen(destino, "wb");
    if (!archivo) {
        curl_easy_cleanup(curl);
        return JPM_ERROR_ARCHIVO;
    }
    
    // Configurar curl
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, archivo);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Seguir redirects
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L); // Verificar SSL
    
    // Callback de progreso
    if (callback) {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, callback);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, datos);
    }
    
    // Realizar descarga
    CURLcode res = curl_easy_perform(curl);
    
    fclose(archivo);
    curl_easy_cleanup(curl);
    
    return (res == CURLE_OK) ? JPM_EXITO : JPM_ERROR_RED;
}
```

#### Uso en jpm_install.c

```c
int jpm_cmd_install(jpm_contexto_t *ctx, const char *fuente) {
    
    // Detectar si es URL
    if (strncmp(fuente, "http://", 7) == 0 || 
        strncmp(fuente, "https://", 8) == 0) {
        
        JPM_LOG_INFO(ctx, "Descargando desde: %s", fuente);
        
        // Generar nombre de archivo temporal
        char temp_file[JPM_MAX_RUTA];
        snprintf(temp_file, sizeof(temp_file), "%s/temp_%ld.jpkg",
                ctx->config.directorio_cache, (long)time(NULL));
        
        // Descargar
        int resultado = jpm_descargar_archivo(fuente, temp_file,
                                              mostrar_progreso, NULL);
        
        if (resultado != JPM_EXITO) {
            JPM_LOG_ERROR(ctx, "Error al descargar archivo");
            return resultado;
        }
        
        JPM_LOG_INFO(ctx, "Descarga completa");
        
        // Instalar desde archivo temporal
        resultado = jpm_instalar_desde_archivo(ctx, temp_file);
        
        // Limpiar archivo temporal si la instalación fue exitosa
        if (resultado == JPM_EXITO) {
            remove(temp_file);
        }
        
        return resultado;
    }
    
    // Si no es URL, continuar con lógica normal (archivo local)
    return jpm_instalar_desde_archivo(ctx, fuente);
}
```

### Callback de Progreso

```c
void mostrar_progreso(size_t descargado, size_t total, void *datos) {
    (void)datos;
    
    if (total > 0) {
        // Tamaño conocido
        int porcentaje = (int)((descargado * 100) / total);
        
        char desc_str[32], total_str[32];
        jpm_formatear_bytes(descargado, desc_str, sizeof(desc_str));
        jpm_formatear_bytes(total, total_str, sizeof(total_str));
        
        printf("\r[%3d%%] %s / %s", porcentaje, desc_str, total_str);
        fflush(stdout);
        
        if (descargado >= total) {
            printf("\n");
        }
    } else {
        // Tamaño desconocido
        char desc_str[32];
        jpm_formatear_bytes(descargado, desc_str, sizeof(desc_str));
        printf("\r%s descargados...", desc_str);
        fflush(stdout);
    }
}
```

### Ventajas

✅ **Súper simple** de implementar  
✅ **Funciona con cualquier hosting** (GitHub, Dropbox, servidores propios)  
✅ **No requiere infraestructura** adicional  
✅ **Control total** del usuario sobre dónde hospedar  
✅ **Compatible con CDNs** para distribución global

### Desventajas

❌ URLs largas y difíciles de recordar  
❌ Sin gestión automática de versiones  
❌ Sin búsqueda de paquetes  
❌ El usuario debe conocer la URL exacta

---

## 🎨 OPCIÓN 2: GITHUB RELEASES

### Descripción

Usar GitHub Releases como hosting de paquetes con sintaxis especial.

### Sintaxis Propuesta

```bash
# Formato corto
jpm install github:usuario/repositorio@version

# Ejemplos
jpm install github:jasboot/calculadora@v1.0.0
jpm install github:jasboot/calculadora@latest
jpm install github:jasboot/calculadora@main
```

### Cómo Funciona GitHub Releases

GitHub permite adjuntar archivos binarios a las releases:

```
Repositorio: https://github.com/jasboot/calculadora

Release v1.0.0:
- Tag: v1.0.0
- Archivos adjuntos:
  ├── calculadora-1.0.0.jpkg (el paquete)
  ├── source code (zip)
  └── source code (tar.gz)

URL del .jpkg:
https://github.com/jasboot/calculadora/releases/download/v1.0.0/calculadora-1.0.0.jpkg
```

### Flujo de Trabajo

```
Usuario: jpm install github:jasboot/calculadora@v1.0.0

    ↓
┌─────────────────────────────────────┐
│ 1. Parsear sintaxis                 │
│    usuario = "jasboot"              │
│    repo = "calculadora"             │
│    version = "v1.0.0"               │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 2. Consultar GitHub API             │
│    GET /repos/jasboot/calculadora/  │
│        releases/tags/v1.0.0         │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 3. Buscar archivo .jpkg             │
│    En la lista de assets            │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 4. Construir URL de descarga        │
│    https://github.com/.../download/ │
│    v1.0.0/calculadora-1.0.0.jpkg    │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 5. Descargar e instalar             │
│    (igual que Opción 1)             │
└─────────────────────────────────────┘
    ↓
✅ Instalación completa
```

### Implementación Técnica

```c
// jpm_github.c

typedef struct {
    char usuario[128];
    char repo[128];
    char version[64];
} jpm_github_ref_t;

// Parsear referencia github:
int jpm_parsear_github_ref(const char *ref, jpm_github_ref_t *info) {
    // Formato: github:usuario/repo@version
    
    if (strncmp(ref, "github:", 7) != 0) {
        return JPM_ERROR_VALIDACION;
    }
    
    const char *resto = ref + 7; // Saltar "github:"
    
    // Buscar '/'
    const char *slash = strchr(resto, '/');
    if (!slash) {
        return JPM_ERROR_VALIDACION;
    }
    
    // Extraer usuario
    size_t user_len = slash - resto;
    if (user_len >= sizeof(info->usuario)) {
        return JPM_ERROR_VALIDACION;
    }
    strncpy(info->usuario, resto, user_len);
    info->usuario[user_len] = '\0';
    
    // Buscar '@'
    const char *arroba = strchr(slash + 1, '@');
    
    if (arroba) {
        // Extraer repo
        size_t repo_len = arroba - (slash + 1);
        if (repo_len >= sizeof(info->repo)) {
            return JPM_ERROR_VALIDACION;
        }
        strncpy(info->repo, slash + 1, repo_len);
        info->repo[repo_len] = '\0';
        
        // Extraer versión
        strncpy(info->version, arroba + 1, sizeof(info->version) - 1);
        info->version[sizeof(info->version) - 1] = '\0';
    } else {
        // Sin versión, usar "latest"
        strncpy(info->repo, slash + 1, sizeof(info->repo) - 1);
        info->repo[sizeof(info->repo) - 1] = '\0';
        strcpy(info->version, "latest");
    }
    
    return JPM_EXITO;
}

// Obtener URL de descarga desde GitHub API
char* jpm_github_obtener_url_jpkg(jpm_contexto_t *ctx, 
                                   const jpm_github_ref_t *ref) {
    
    // Construir URL de API
    char api_url[JPM_MAX_URL];
    
    if (strcmp(ref->version, "latest") == 0) {
        snprintf(api_url, sizeof(api_url),
            "https://api.github.com/repos/%s/%s/releases/latest",
            ref->usuario, ref->repo);
    } else {
        snprintf(api_url, sizeof(api_url),
            "https://api.github.com/repos/%s/%s/releases/tags/%s",
            ref->usuario, ref->repo, ref->version);
    }
    
    JPM_LOG_DEBUG(ctx, "Consultando GitHub API: %s", api_url);
    
    // Hacer petición HTTP GET
    int codigo_estado;
    char *respuesta_json = jpm_http_get(api_url, &codigo_estado);
    
    if (!respuesta_json || codigo_estado != 200) {
        JPM_LOG_ERROR(ctx, "Error al consultar GitHub API (código %d)", 
                     codigo_estado);
        if (respuesta_json) free(respuesta_json);
        return NULL;
    }
    
    // Parsear JSON
    void *json = jpm_json_parsear(respuesta_json);
    if (!json) {
        JPM_LOG_ERROR(ctx, "Error al parsear respuesta de GitHub");
        free(respuesta_json);
        return NULL;
    }
    
    // Obtener array de assets
    void *assets = jpm_json_obtener_array(json, "assets");
    if (!assets) {
        JPM_LOG_ERROR(ctx, "No se encontraron assets en la release");
        jpm_json_liberar(json);
        free(respuesta_json);
        return NULL;
    }
    
    // Buscar archivo .jpkg
    size_t num_assets = jpm_json_array_tamano(assets);
    char *download_url = NULL;
    
    for (size_t i = 0; i < num_assets; i++) {
        void *asset = jpm_json_array_obtener(assets, i);
        const char *nombre = jpm_json_obtener_string(asset, "name");
        
        if (nombre && strstr(nombre, ".jpkg")) {
            download_url = strdup(jpm_json_obtener_string(asset, 
                                  "browser_download_url"));
            break;
        }
    }
    
    jpm_json_liberar(json);
    free(respuesta_json);
    
    if (!download_url) {
        JPM_LOG_ERROR(ctx, "No se encontró archivo .jpkg en la release");
    }
    
    return download_url;
}

// Instalar desde GitHub
int jpm_instalar_desde_github(jpm_contexto_t *ctx, const char *ref_str) {
    
    // Parsear referencia
    jpm_github_ref_t ref;
    if (jpm_parsear_github_ref(ref_str, &ref) != JPM_EXITO) {
        JPM_LOG_ERROR(ctx, "Formato inválido. Usa: github:usuario/repo@version");
        return JPM_ERROR_VALIDACION;
    }
    
    JPM_LOG_INFO(ctx, "Instalando desde GitHub: %s/%s @ %s",
                ref.usuario, ref.repo, ref.version);
    
    // Obtener URL de descarga
    char *download_url = jpm_github_obtener_url_jpkg(ctx, &ref);
    if (!download_url) {
        return JPM_ERROR_RED;
    }
    
    JPM_LOG_INFO(ctx, "URL encontrada: %s", download_url);
    
    // Descargar e instalar (reusar código de Opción 1)
    int resultado = jpm_cmd_install(ctx, download_url);
    
    free(download_url);
    return resultado;
}
```

### Uso

```bash
# Instalar última versión
jpm install github:jasboot/calculadora@latest

# Instalar versión específica
jpm install github:jasboot/calculadora@v1.2.0

# Instalar desde branch
jpm install github:jasboot/calculadora@main
```

### Ventajas

✅ **Sintaxis corta** y fácil de recordar  
✅ **Aprovecha infraestructura de GitHub** (gratis)  
✅ **Control de versiones** integrado con Git tags  
✅ **Sin servidor propio** necesario  
✅ **Estadísticas** de descargas en GitHub

### Desventajas

❌ Dependencia de GitHub (si cae, no funciona)  
❌ Necesita configurar releases correctamente  
❌ Límites de tasa de API (60 requests/hora sin autenticación)  
❌ Sin búsqueda avanzada

---

## 🔗 OPCIÓN 3: GITHUB REPOSITORIOS

### Descripción

Clonar repositorio Git completo y empaquetarlo automáticamente.

### Sintaxis

```bash
# Desde Git HTTPS
jpm install git+https://github.com/jasboot/calculadora.git

# Desde Git con tag
jpm install git+https://github.com/jasboot/calculadora.git#v1.0.0

# Desde Git con branch
jpm install git+https://github.com/jasboot/calculadora.git#main
```

### Flujo de Trabajo

```
Usuario: jpm install git+https://github.com/jasboot/calculadora.git

    ↓
┌─────────────────────────────────────┐
│ 1. Clonar repositorio               │
│    git clone URL temp_dir           │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 2. Checkout versión (si se indica)  │
│    git checkout v1.0.0              │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 3. Leer jasboot.json                │
│    Desde el repositorio clonado     │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 4. Empaquetar automáticamente       │
│    jpm pack temp_dir                │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 5. Instalar paquete generado        │
│    jpm install temp.jpkg            │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 6. Limpiar temporales               │
│    rm -rf temp_dir                  │
└─────────────────────────────────────┘
    ↓
✅ Instalación completa
```

### Implementación

```c
// jpm_git.c

int jpm_instalar_desde_git(jpm_contexto_t *ctx, const char *url_git) {
    
    // Crear directorio temporal
    char temp_dir[JPM_MAX_RUTA];
    snprintf(temp_dir, sizeof(temp_dir), "%s/git_%ld",
            ctx->config.directorio_cache, (long)time(NULL));
    
    JPM_LOG_INFO(ctx, "Clonando repositorio Git...");
    
    // Ejecutar git clone
    char comando[JPM_MAX_RUTA * 2];
    snprintf(comando, sizeof(comando), "git clone \"%s\" \"%s\"",
            url_git, temp_dir);
    
    int resultado = system(comando);
    if (resultado != 0) {
        JPM_LOG_ERROR(ctx, "Error al clonar repositorio");
        return JPM_ERROR_GENERICO;
    }
    
    JPM_LOG_INFO(ctx, "Repositorio clonado exitosamente");
    
    // Empaquetar
    JPM_LOG_INFO(ctx, "Empaquetando repositorio...");
    
    char paquete_temp[JPM_MAX_RUTA];
    snprintf(paquete_temp, sizeof(paquete_temp), "%s/temp.jpkg",
            ctx->config.directorio_cache);
    
    resultado = jpm_empaquetar(temp_dir, paquete_temp);
    if (resultado != JPM_EXITO) {
        JPM_LOG_ERROR(ctx, "Error al empaquetar repositorio");
        jpm_eliminar_directorio(temp_dir);
        return resultado;
    }
    
    // Instalar
    resultado = jpm_instalar_desde_archivo(ctx, paquete_temp);
    
    // Limpiar
    jpm_eliminar_directorio(temp_dir);
    remove(paquete_temp);
    
    return resultado;
}
```

### Ventajas

✅ **Acceso al código fuente** completo  
✅ **Soporte para cualquier hosting Git** (GitHub, GitLab, Bitbucket)  
✅ **Historial completo** disponible  
✅ **Fácil para desarrollo** (git submodules)

### Desventajas

❌ **Requiere Git instalado** en el sistema  
❌ **Descarga completa** del repositorio (puede ser pesado)  
❌ **Más lento** que descargar .jpkg directo  
❌ **Complejidad adicional** en el código

---

## 🏢 OPCIÓN 4: REGISTRO CENTRAL

### Descripción

Un servidor central que mantiene un índice de todos los paquetes Jasboot.

### Sintaxis

```bash
# Instalación simple por nombre
jpm install calculadora

# Con versión específica
jpm install calculadora@1.2.0

# Última versión
jpm install calculadora@latest
```

### Arquitectura del Registro

```
registry.jasboot.org
│
├── API REST
│   ├── GET  /api/packages          # Listar todos
│   ├── GET  /api/packages/:nombre  # Info de paquete
│   ├── GET  /api/search?q=...      # Buscar
│   ├── POST /api/publish           # Publicar (autenticado)
│   └── GET  /api/download/:nombre/:version
│
├── Base de Datos (PostgreSQL)
│   ├── Tabla: paquetes
│   ├── Tabla: versiones
│   ├── Tabla: usuarios
│   └── Tabla: descargas (estadísticas)
│
├── Almacenamiento de Archivos
│   ├── S3 / Bucket / Filesystem
│   └── CDN (CloudFlare, AWS CloudFront)
│
└── Frontend Web
    ├── Página de inicio
    ├── Búsqueda de paquetes
    ├── Página de cada paquete
    └── Dashboard de usuario
```

### Base de Datos - Esquema

```sql
-- Tabla de paquetes
CREATE TABLE paquetes (
    id SERIAL PRIMARY KEY,
    nombre VARCHAR(100) UNIQUE NOT NULL,
    descripcion TEXT,
    autor_id INT REFERENCES usuarios(id),
    homepage VARCHAR(500),
    repositorio VARCHAR(500),
    licencia VARCHAR(50),
    palabras_clave TEXT[],
    descargas_totales BIGINT DEFAULT 0,
    fecha_creacion TIMESTAMP DEFAULT NOW(),
    fecha_actualizacion TIMESTAMP DEFAULT NOW()
);

-- Tabla de versiones
CREATE TABLE versiones (
    id SERIAL PRIMARY KEY,
    paquete_id INT REFERENCES paquetes(id),
    version VARCHAR(20) NOT NULL,
    archivo_url VARCHAR(500) NOT NULL,
    archivo_hash VARCHAR(129) NOT NULL,
    tamano_bytes BIGINT,
    dependencias JSONB,
    fecha_publicacion TIMESTAMP DEFAULT NOW(),
    UNIQUE(paquete_id, version)
);

-- Tabla de usuarios
CREATE TABLE usuarios (
    id SERIAL PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    email VARCHAR(100) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    api_token VARCHAR(100) UNIQUE,
    fecha_registro TIMESTAMP DEFAULT NOW()
);

-- Tabla de estadísticas
CREATE TABLE descargas (
    id