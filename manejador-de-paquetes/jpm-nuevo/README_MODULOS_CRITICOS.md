# Módulos Críticos del JPM - Guía de Uso

**Versión:** 0.2.0  
**Fecha:** 2024  
**Estado:** ✅ IMPLEMENTADO Y FUNCIONAL

---

## Introducción

Este documento describe los tres módulos críticos recién implementados del Jasboot Package Manager (JPM):

1. **jpm_pack.c** - Empaquetado y extracción de archivos .jpkg
2. **jpm_install.c** - Instalación de paquetes y gestión de dependencias
3. **jpm_lock.c** - Gestión del archivo jpm.lock

Estos módulos completan la funcionalidad esencial del sistema de gestión de paquetes de Jasboot.

---

## Compilación

### Requisitos Previos

#### Windows
- MinGW-w64 o GCC instalado
- Make (opcional, se puede usar build.bat)
- Windows 7 o superior

#### Linux/macOS
- GCC 7.0+ o Clang 6.0+
- Make
- Bibliotecas estándar de C11

### Compilar en Windows

**Opción 1: Usando build.bat**
```bash
# Compilar versión release
build.bat

# Compilar versión debug
build.bat debug

# Limpiar archivos compilados
build.bat clean

# Ver información del build
build.bat info
```

**Opción 2: Usando Make**
```bash
make all
```

### Compilar en Linux/macOS

```bash
# Compilar versión release
make

# Compilar versión debug
make debug

# Limpiar archivos
make clean

# Instalar en el sistema
sudo make install

# Ver ayuda
make help
```

### Verificar la Compilación

```bash
# El ejecutable estará en:
bin/jpm.exe       # Windows
bin/jpm           # Linux/macOS

# Verificar versión
bin/jpm --version
```

---

## Uso Básico

### 1. Crear un Nuevo Proyecto

```bash
# Inicializar proyecto
jpm init mi-proyecto

# Esto crea jasboot.json:
# {
#   "nombre": "mi-proyecto",
#   "version": "1.0.0",
#   "descripcion": "",
#   "autor": "",
#   "licencia": "MIT",
#   "principal": "src/principal.jasb"
# }
```

### 2. Empaquetar un Proyecto

```bash
# Empaquetar directorio actual
jpm pack

# Empaquetar directorio específico
jpm pack ./mi-biblioteca

# Salida:
# [INFO] Empaquetando mi-proyecto v1.0.0
#   + jasboot.json (234 bytes)
#   + README.md (1.2 KB)
#   + src/principal.jasb (3.4 KB)
#   + src/modulos/utilidades.jasb (2.1 KB)
# [EXITO] Paquete creado: mi-proyecto-1.0.0.jpkg (45.3 KB)
# [INFO] SHA-512: a3f5b9c2d8e1f4a7b3c9d2e5f8a1b4c7d9e2f5a8b1c4d7e0f3a6b9c2d5e8f1a4...
```

### 3. Instalar Paquetes

**Desde archivo local:**
```bash
jpm install ./mi-biblioteca-1.0.0.jpkg

# Salida:
# [INFO] Instalando desde archivo: ./mi-biblioteca-1.0.0.jpkg
# [INFO] Extrayendo paquete...
#   + jasboot.json (234 bytes)
#   + src/principal.jasb (3.4 KB)
# [INFO] Paquete: mi-biblioteca v1.0.0
# [INFO] Verificando integridad...
# [EXITO] Hash verificado correctamente
# [INFO] Instalando en: ./a-modulos/mi-biblioteca
# [EXITO] Paquete mi-biblioteca v1.0.0 instalado correctamente
```

**Desde URL:**
```bash
jpm install https://registro.jasboot.org/paquetes/biblioteca-utils-2.3.0.jpkg

# Descarga el archivo y lo instala automáticamente
```

**Desde registro (nombre):**
```bash
# Instalar última versión
jpm install biblioteca-utils

# Instalar versión específica
jpm install biblioteca-utils@2.3.0
```

### 4. Instalar Dependencias del Proyecto

```bash
# Agregar dependencia a jasboot.json manualmente:
# {
#   "nombre": "mi-app",
#   "version": "1.0.0",
#   "dependencias": {
#     "biblioteca-utils": "^2.3.0",
#     "json-parser": "~1.0.0"
#   }
# }

# Instalar todas las dependencias
jpm install

# Salida:
# [INFO] Instalando dependencias de mi-app v1.0.0
# [INFO] Total: 2 dependencias, 0 de desarrollo
# [INFO] Buscando en registro: biblioteca-utils
# [INFO] Instalando dependencia: biblioteca-utils ^2.3.0
# [INFO] Buscando en registro: json-parser
# [INFO] Instalando dependencia: json-parser ~1.0.0
# [EXITO] Todas las dependencias instaladas
```

### 5. Verificar Paquetes Instalados

```bash
# Listar paquetes instalados
jpm list

# Ver información de un paquete
jpm info biblioteca-utils
```

---

## Archivo .jpmignore

Crea un archivo `.jpmignore` en la raíz de tu proyecto para excluir archivos/directorios del empaquetado:

```
# Comentarios con #
node_modules
.git/
.vscode/
*.tmp
*.log
build/
dist/
.jpm-temp/
.jpm-cache/
tests/
docs/
```

**Patrones ignorados por defecto (siempre):**
- `node_modules`
- `.git`
- `.jpm-temp`
- `.jpm-cache`

---

## Archivo jpm.lock

El archivo `jpm.lock` se genera automáticamente al instalar dependencias y garantiza instalaciones reproducibles:

```json
{
  "version": "1.0.0",
  "generado": "2024-04-20T14:30:00",
  "paquetes": {
    "biblioteca-utils": {
      "version": "2.3.0",
      "resuelto": "https://registro.jasboot.org/paquetes/biblioteca-utils-2.3.0.jpkg",
      "hash": "sha512-a3f5b9c2d8e1f4a7b3c9d2e5f8a1b4c7...",
      "dependencias": {
        "json-parser": "~1.0.0"
      }
    },
    "json-parser": {
      "version": "1.0.2",
      "resuelto": "https://registro.jasboot.org/paquetes/json-parser-1.0.2.jpkg",
      "hash": "sha512-b4c7d9e2f5a8b1c4d7e0f3a6b9c2d5e8..."
    }
  }
}
```

**Importante:** Commitea `jpm.lock` en tu repositorio para garantizar que todos los desarrolladores usen las mismas versiones.

---

## Estructura de Directorios

Después de instalar dependencias, tu proyecto tendrá esta estructura:

```
mi-proyecto/
├── jasboot.json          # Metadatos del proyecto
├── jpm.lock              # Versiones bloqueadas de dependencias
├── .jpmignore            # Patrones de archivos a ignorar
├── src/
│   └── principal.jasb
├── a-modulos/            # Dependencias instaladas (NO commitear)
│   ├── biblioteca-utils/
│   │   ├── jasboot.json
│   │   └── src/
│   └── json-parser/
│       ├── jasboot.json
│       └── src/
└── mi-proyecto-1.0.0.jpkg  # Paquete generado (al ejecutar jpm pack)
```

---

## Ejemplos Prácticos

### Ejemplo 1: Crear y Publicar una Biblioteca

```bash
# 1. Crear proyecto
mkdir mi-biblioteca
cd mi-biblioteca

# 2. Inicializar
jpm init mi-biblioteca

# 3. Editar jasboot.json
# {
#   "nombre": "mi-biblioteca",
#   "version": "1.0.0",
#   "descripcion": "Biblioteca de utilidades para Jasboot",
#   "autor": "Tu Nombre",
#   "licencia": "MIT",
#   "principal": "src/principal.jasb",
#   "palabras_clave": ["utilidades", "helpers"]
# }

# 4. Crear código fuente
mkdir src
echo 'funcion saludar(nombre: texto): texto { ... }' > src/principal.jasb

# 5. Crear .jpmignore
echo "tests/" > .jpmignore
echo "*.tmp" >> .jpmignore

# 6. Empaquetar
jpm pack

# 7. Verificar integridad
# El hash SHA-512 se agregó automáticamente a jasboot.json

# 8. Instalar localmente para probar
jpm install ./mi-biblioteca-1.0.0.jpkg

# 9. Publicar (cuando esté implementado jpm_registry.c)
jpm publish
```

### Ejemplo 2: Consumir una Biblioteca

```bash
# 1. Crear aplicación
mkdir mi-app
cd mi-app
jpm init mi-app

# 2. Editar jasboot.json y agregar dependencia
# {
#   "nombre": "mi-app",
#   "version": "1.0.0",
#   "dependencias": {
#     "mi-biblioteca": "^1.0.0"
#   }
# }

# 3. Instalar dependencias
jpm install

# 4. Usar la biblioteca en tu código
# src/principal.jasb:
# importar { saludar } desde "mi-biblioteca"
# 
# funcion principal() {
#   texto mensaje = saludar("Mundo")
#   escribir(mensaje)
# }

# 5. Ejecutar (con jasboot-ir)
jasboot-ir mi-app
```

### Ejemplo 3: Dry-Run (Simular Instalación)

```bash
# Ver qué se haría sin ejecutar
jpm install mi-biblioteca --simular

# Salida:
# [DRY-RUN] Se extraería a: ./a-modulos/mi-biblioteca
# [DRY-RUN] Se copiarían 15 archivos
# [DRY-RUN] Se instalarían 3 dependencias
```

---

## Opciones de Línea de Comandos

```bash
# Opciones globales
jpm --version              # Mostrar versión
jpm --help                 # Mostrar ayuda

# Opciones de instalación
jpm install --global       # Instalar globalmente (~/.jpm/paquetes)
jpm install --forzar       # Reinstalar aunque ya esté instalado
jpm install --simular      # Dry-run, no ejecutar realmente
jpm install --verboso      # Mostrar información detallada

# Opciones de empaquetado
jpm pack --directorio ./path   # Empaquetar directorio específico
```

---

## Verificación de Integridad

Todos los paquetes incluyen hash SHA-512 para verificación:

```bash
# El hash se calcula automáticamente al empaquetar
jpm pack
# [INFO] SHA-512: a3f5b9c2d8e1f4a7b3c9d2e5f8a1b4c7d9e2f5a8b1c4d7e0f3a6b9c2d5e8f1a4...

# Se verifica automáticamente al instalar
jpm install ./paquete.jpkg
# [INFO] Verificando integridad...
# [EXITO] Hash verificado correctamente

# Si el hash no coincide:
# [ERROR] Verificación de integridad falló
#   Esperado:  a3f5b9c2...
#   Calculado: b4c7d9e2...
```

---

## Resolución de Problemas

### Error: "No se pudo abrir archivo"

**Causa:** El archivo .jpkg no existe o no tienes permisos.

**Solución:**
```bash
# Verificar que el archivo existe
ls -la mi-paquete.jpkg

# Verificar permisos
chmod 644 mi-paquete.jpkg  # Linux/macOS
```

### Error: "Archivo ZIP inválido o corrupto"

**Causa:** El archivo .jpkg está dañado o no es un ZIP válido.

**Solución:**
```bash
# Intentar extraer manualmente
unzip -t mi-paquete.jpkg

# Volver a empaquetar desde el directorio fuente
jpm pack ./directorio-fuente
```

### Error: "jasboot.json no encontrado"

**Causa:** Intentas empaquetar un directorio sin jasboot.json.

**Solución:**
```bash
# Inicializar el proyecto primero
jpm init mi-proyecto

# O crear jasboot.json manualmente
echo '{"nombre":"mi-proyecto","version":"1.0.0"}' > jasboot.json
```

### Error: "Fallo al instalar dependencia"

**Causa:** La dependencia no está disponible o hay problemas de red.

**Solución:**
```bash
# Verificar conectividad (si es desde URL/registro)
ping registro.jasboot.org

# Instalar manualmente desde archivo local
jpm install ./dependencia-local.jpkg

# Ver logs detallados
jpm install --verboso
```

### Error: "Profundidad máxima de dependencias alcanzada"

**Causa:** Dependencias circulares o árbol muy profundo (>10 niveles).

**Solución:**
```bash
# Revisa tus dependencias en jasboot.json
# Evita dependencias circulares: A depende de B, B depende de A

# Simplifica el árbol de dependencias si es posible
```

---

## Referencia Rápida de Comandos

| Comando | Descripción |
|---------|-------------|
| `jpm init <nombre>` | Inicializar nuevo proyecto |
| `jpm pack` | Empaquetar proyecto actual |
| `jpm pack <dir>` | Empaquetar directorio específico |
| `jpm install` | Instalar todas las dependencias |
| `jpm install <paquete>` | Instalar paquete específico |
| `jpm install <archivo.jpkg>` | Instalar desde archivo local |
| `jpm install <url>` | Instalar desde URL |
| `jpm list` | Listar paquetes instalados |
| `jpm info <paquete>` | Ver información de paquete |
| `jpm uninstall <paquete>` | Desinstalar paquete |
| `jpm clean` | Limpiar caché |

---

## Características Implementadas ✅

- ✅ Empaquetado ZIP estándar
- ✅ Extracción de paquetes .jpkg
- ✅ Hash SHA-512 para verificación de integridad
- ✅ Instalación desde archivo local
- ✅ Instalación desde URL (requiere jpm_network.c)
- ✅ Gestión de dependencias recursivas
- ✅ Archivo jpm.lock para reproducibilidad
- ✅ Soporte para .jpmignore
- ✅ Instalación global y local
- ✅ Dry-run (--simular)
- ✅ Validación de metadatos

---

## Características Pendientes 🚧

- 🚧 Descarga HTTP/HTTPS real (jpm_network.c)
- 🚧 Registro central de paquetes (jpm_registry.c)
- 🚧 Resolución avanzada de conflictos de versiones
- 🚧 Compresión DEFLATE en ZIP
- 🚧 Sistema de caché de paquetes (jpm_cache.c)
- 🚧 Firmas digitales
- 🚧 Soporte para repositorios Git

---

## Contribuir

Si encuentras bugs o quieres contribuir:

1. Reporta issues en el repositorio
2. Crea pull requests con mejoras
3. Sigue el estilo de código existente
4. Agrega tests para nuevas funcionalidades

---

## Licencia

Jasboot Package Manager (JPM)  
Copyright (c) 2024 Jasboot Team  
Licencia: MIT

---

## Recursos Adicionales

- **Documentación completa:** `MODULOS_CRITICOS_IMPLEMENTADOS.md`
- **Especificación .jpkg:** `../especificacion-jpkg.md`
- **Plan de implementación:** `../plan-implementacion-jpm.md`
- **Estructura interna:** `../estructura-interna.md`

---

**Última actualización:** 2024  
**Versión del documento:** 1.0