# ESTRUCTURA DE DIRECTORIOS - Gestor de Paquetes Jasboot

## Directorios del Sistema JPM

### 1. Directorio Principal del Usuario
```
$HOME/.jpm/                    # Directorio raíz de JPM
|
+-- paquetes/                   # Paquetes instalados
|   +-- nombre-paquete/
|   |   +-- 1.0.0/
|   |   |   +-- jasboot.json
|   |   |   +-- src/
|   |   |   |   +-- main.jasb
|   |   |   |   +-- modulo1.jasb
|   |   |   |   +-- ...
|   |   |   +-- docs/
|   |   |   |   +-- README.md
|   |   |   |   +-- API.md
|   |   |   +-- tests/
|   |   |   |   +-- test_*.jasb
|   |   |   +-- bin/
|   |   |   |   +-- ejecutable.exe
|   |   |   +-- recursos/
|   |   |   |   +-- config/
|   |   |   |   +-- datos/
|   |   |   |   +-- assets/
|   |   +-- 1.1.0/
|   |   +-- 1.2.0/
|   |   +-- current -> 1.2.0
|
+-- cache/                      # Cache local
|   +-- descargas/
|   |   +-- abc123...          # Paquetes .jpkg descargados
|   |   +-- def456...
|   +-- temporal/
|   |   +-- extract_12345/     # Extracciones temporales
|   |   +-- extract_67890/
|   +-- metadatos/
|   |   +-- registry_cache.json
|   |   +-- package_info.json
|   +-- resolucion/
|   |   +-- dep_graph_123.cache
|   |   +-- version_lock.cache
|
+-- registro/                   # Base de datos local
|   +-- instalados.json        # Paquetes instalados
|   +-- lock.json              # Lockfile del proyecto
|   +-- registry_index.json    # Índice de registry
|   +-- dependencies.db        # Base SQLite de dependencias
|   +-- audit.log              # Log de auditoría
|
+-- bin/                        # Ejecutables vinculados
|   +-- comando1 -> ../paquetes/paquete1/1.0.0/bin/ejecutable1
|   +-- comando2 -> ../paquetes/paquete2/2.1.0/bin/ejecutable2
|   +-- jpm -> /usr/local/bin/jpm
|
+-- config/                     # Configuración
|   +-- config.json            # Configuración principal
|   +-- auth.json              # Tokens de autenticación
|   +-- trust_store.json       # Certificados de confianza
|   +-- profiles/              # Perfiles de configuración
|   |   +-- default.json
|   |   +-- development.json
|   |   +-- production.json
|
+-- logs/                       # Logs del sistema
|   +-- jpm.log                # Log principal
|   +-- install.log            # Log de instalaciones
|   +-- error.log              # Log de errores
|   +-- audit.log              # Log de auditoría
|   +-- performance.log        # Log de rendimiento
|
+-- tmp/                        # Archivos temporales
|   +-- build_12345/           # Builds temporales
|   +-- extract_67890/         # Extracciones temporales
|   +-- download_abcde/        # Descargas parciales
|
+-- keys/                       # Claves criptográficas
|   +-- private/               # Claves privadas
|   |   +-- signing.key
|   |   +-- auth.key
|   +-- public/                # Claves públicas
|   |   +-- signing.pub
|   |   +-- registry.pub
|
+-- backups/                    # Backups automáticos
|   +-- registry_2026-04-17.json
|   +-- config_2026-04-17.json
|   +-- lockfiles/
|   |   +-- project1_2026-04-17.json
|   |   +-- project2_2026-04-17.json
```

### 2. Directorios de Proyecto
```
mi-proyecto/                   # Raíz del proyecto
|
+-- jasboot.json              # Metadatos del paquete
+-- README.md                  # Documentación
+-- LICENSE                    # Licencia
+-- .gitignore                 # Ignorados por git
+-- .jpmignore                 # Ignorados por jpm
|
+-- src/                       # Código fuente
|   +-- main.jasb             # Punto de entrada
|   +-- lib/
|   |   +-- utils.jasb
|   |   +-- config.jasb
|   |   +-- ...
|   +-- modulos/
|   |   +-- modulo1.jasb
|   |   +-- modulo2.jasb
|
+-- tests/                     # Pruebas
|   +-- test_main.jasb
|   +-- test_utils.jasb
|   +-- integration/
|   |   +-- test_api.jasb
|
+-- docs/                      # Documentación
|   +-- API.md
|   +-- GUIDE.md
|   +-- examples/
|   |   +-- basic_usage.jasb
|   |   +-- advanced.jasb
|
+-- bin/                       # Ejecutables compilados
|   +-- main.exe
|   +-- main.jbo
|
+-- build/                     # Build artifacts
|   +-- *.jbo                  # Bytecode Jasboot
|   +-- *.exe                  # Ejecutables
|
+-- recursos/                  # Recursos adicionales
|   +-- config/
|   |   +-- default.json
|   |   +-- development.json
|   +-- datos/
|   |   +-- sample.dat
|   +-- assets/
|   |   +-- images/
|   |   +-- templates/
|
+-- node_modules/              # Dependencias instaladas (si aplica)
|   +-- paquete-dependiente/
|   |   +-- 1.2.3/
|   |   |   +-- jasboot.json
|   |   |   +-- src/
|   |   +-- current -> 1.2.3
|
+-- .jpm/                      # Metadatos locales del proyecto
|   +-- lock.json              # Lockfile de dependencias
|   +-- cache.json             # Cache local del proyecto
|   +-- build.log              # Log de builds
|   +-- test.log               # Log de pruebas
```

---

## Estructura de Registry (Servidor)

### 1. Directorios del Servidor
```
/opt/jpm-registry/              # Raíz del registry
|
+-- packages/                  # Almacenamiento de paquetes
|   +-- nombre-paquete/
|   |   +-- 1.0.0.jpkg
|   |   +-- 1.1.0.jpkg
|   |   +-- 1.2.0.jpkg
|   |   +-- latest -> 1.2.0.jpkg
|
+-- metadata/                   # Metadatos de paquetes
|   +-- nombre-paquete/
|   |   +-- 1.0.0.json
|   |   +-- 1.1.0.json
|   |   +-- 1.2.0.json
|   |   +-- latest.json
|
+-- search/                     # Índices de búsqueda
|   +-- index.json             # Índice principal
|   +-- keywords.json          # Índice por palabras clave
|   +-- authors.json           # Índice por autores
|   +-- tags.json              # Índice por etiquetas
|
+-- users/                      # Gestión de usuarios
|   +-- usuarios.json          # Base de usuarios
|   +-- tokens/                # Tokens activos
|   |   +-- abc123.json
|   |   +-- def456.json
|
+-- stats/                      # Estadísticas
|   +-- downloads.json        # Contador de descargas
|   |   +-- 2026-04.json
|   |   +-- 2026-04-17.json
|   +-- popularity.json       # Popularidad
|   +-- trends.json           # Tendencias
|
+-- logs/                       # Logs del servidor
|   +-- access.log             # Log de accesos
|   +-- error.log              # Log de errores
|   +-- audit.log              # Log de auditoría
|
+-- config/                     # Configuración del servidor
|   +-- server.json            # Config principal
|   +-- database.json          # Config DB
|   +-- security.json          # Config seguridad
|
+-- backups/                    # Backups del servidor
|   +-- daily/
|   |   +-- 2026-04-17/
|   |   |   +-- packages.tar.gz
|   |   |   +-- metadata.tar.gz
|   |   |   +-- database.sql
|   +-- weekly/
|   +-- monthly/
```

---

## Convenciones de Nomenclatura

### 1. Nombres de Paquetes
- **Formato**: kebab-case (ej: `mi-paquete-awsome`)
- **Caracteres permitidos**: letras minúsculas, números, guiones
- **Longitud**: 3-50 caracteres
- **Reservados**: `jpm`, `jasboot`, `core`, `stdlib`

### 2. Versiones
- **Formato**: SemVer (X.Y.Z)
- **Pre-releases**: `alpha.1`, `beta.2`, `rc.1`
- **Build metadata**: `+build.123`

### 3. Directorios
- **Paquetes**: `$HOME/.jpm/paquetes/{nombre}/{version}/`
- **Cache**: `$HOME/.jpm/cache/{tipo}/`
- **Config**: `$HOME/.jpm/config/`
- **Logs**: `$HOME/.jpm/logs/`

### 4. Archivos
- **Metadatos**: `jasboot.json`
- **Lockfile**: `jpm.lock` o `lock.json`
- **Config**: `config.json`
- **Ignore**: `.jpmignore`

---

## Permisos y Seguridad

### 1. Permisos Recomendados
```bash
# Directorio principal .jpm
chmod 700 $HOME/.jpm/

# Paquetes instalados
chmod 755 $HOME/.jpm/paquetes/
chmod 644 $HOME/.jpm/paquetes/*/*.json
chmod 755 $HOME/.jpm/paquetes/*/current

# Cache
chmod 700 $HOME/.jpm/cache/
chmod 600 $HOME/.jpm/cache/*/*

# Configuración
chmod 700 $HOME/.jpm/config/
chmod 600 $HOME/.jpm/config/*.json
chmod 600 $HOME/.jpm/config/auth.json

# Logs
chmod 755 $HOME/.jpm/logs/
chmod 644 $HOME/.jpm/logs/*.log

# Claves
chmod 700 $HOME/.jpm/keys/
chmod 600 $HOME/.jpm/keys/*/*
```

### 2. Aislamiento de Usuario
- Cada usuario tiene su propio `.jpm/`
- No compartir directorios entre usuarios
- Permisos restrictivos en archivos sensibles

### 3. Sandbox de Instalación
- Instalaciones en directorios aislados
- Verificación de permisos antes de instalar
- Aislamiento de paquetes globales vs locales

---

## Gestión de Espacio

### 1. Cuotas por Defecto
```json
{
  "cache": {
    "max_size": "1GB",
    "max_files": 1000,
    "ttl": "7d"
  },
  "logs": {
    "max_size": "100MB",
    "max_files": 10,
    "rotation": "daily"
  },
  "temp": {
    "max_size": "500MB",
    "cleanup_interval": "1h"
  },
  "packages": {
    "max_versions_per_package": 5,
    "auto_cleanup": true
  }
}
```

### 2. Limpieza Automática
- **Cache**: LRU con TTL de 7 días
- **Logs**: Rotación diaria, mantener 10 archivos
- **Temp**: Limpieza cada hora
- **Paquetes**: Mantener últimas 5 versiones

### 3. Comandos de Limpieza
```bash
# Limpiar cache
jpm cache clean

# Limpiar logs antiguos
jpm logs clean --older-than 30d

# Limpiar paquetes viejos
jpm cleanup --keep-versions 3

# Limpiar todo
jpm cleanup --all
```

---

## Backups y Recuperación

### 1. Estrategia de Backup
```bash
# Backup completo
jpm backup create --full

# Backup incremental
jpm backup create --incremental

# Backup de configuración
jpm backup create --config-only

# Listar backups
jpm backup list

# Restaurar backup
jpm backup restore 2026-04-17_full
```

### 2. Archivos Críticos
- `config/config.json` - Configuración principal
- `registro/instalados.json` - Paquetes instalados
- `keys/private/signing.key` - Clave de firma
- `registry/lock.json` - Lockfile del proyecto

### 3. Recuperación de Desastres
```bash
# Recuperar desde backup
jpm recovery restore --backup 2026-04-17_full

# Recuperar paquetes corruptos
jpm recovery verify --fix

# Recuperar configuración
jpm recovery config --restore

# Reconstruir índices
jpm recovery rebuild-index
```

---

## Integración con Sistema

### 1. Variables de Entorno
```bash
export JPM_HOME="$HOME/.jpm"
export JPM_CONFIG="$JPM_HOME/config/config.json"
export JPM_CACHE="$JPM_HOME/cache"
export JPM_LOGS="$JPM_HOME/logs"
export JPM_REGISTRY="https://registry.jasboot.org"
export JPM_TIMEOUT="30s"
export JPM_MAX_CONCURRENT="5"
```

### 2. Integración con Shell
```bash
# Bash completion
source <(jpm completion bash)

# Zsh completion
source <(jpm completion zsh)

# Fish completion
jpm completion fish > ~/.config/fish/completions/jpm.fish
```

### 3. Integración con Editores
```json
// VS Code settings.json
{
  "jasboot.packageManager": "jpm",
  "jasboot.jpmPath": "~/.jpm",
  "jasboot.autoInstall": true
}
```

---

## Monitoreo y Mantenimiento

### 1. Estado del Sistema
```bash
# Verificar estado
jpm status

# Verificar integridad
jpm verify --all

# Estadísticas de uso
jpm stats

# Health check
jpm health
```

### 2. Métricas de Directorios
```bash
# Uso de espacio
jpm disk-usage

# Tamaño de cache
jpm cache stats

# Estadísticas de paquetes
jpm package stats

# Logs de errores recientes
jpm logs --level error --since 1h
```

### 3. Mantenimiento Programado
```bash
# Mantenimiento automático (cron)
0 2 * * * jpm maintenance --auto

# Limpieza semanal
0 3 * * 0 jpm cleanup --all

# Backup diario
0 4 * * * jpm backup create --incremental
```

---

*Esta estructura de directorios proporciona una base sólida y organizada para el gestor de paquetes Jasboot, asegurando consistencia, seguridad y facilidad de mantenimiento.*
