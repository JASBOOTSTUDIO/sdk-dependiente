# 📦 JPM - Jasboot Package Manager

**Sistema de gestión de paquetes para el lenguaje Jasboot**

![Version](https://img.shields.io/badge/version-0.2.0-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)
![Language](https://img.shields.io/badge/language-C11-orange.svg)

---

## 📋 Tabla de Contenidos

- [Descripción](#-descripción)
- [Estado del Proyecto](#-estado-del-proyecto)
- [Características](#-características)
- [Compilación](#-compilación)
- [Instalación](#-instalación)
- [Uso Rápido](#-uso-rápido)
- [Comandos](#-comandos)
- [Formato .jpkg](#-formato-jpkg)
- [Arquitectura](#-arquitectura)
- [Ejemplos](#-ejemplos)
- [Roadmap](#-roadmap)
- [Contribuir](#-contribuir)

---

## 🎯 Descripción

**JPM (Jasboot Package Manager)** es el gestor de paquetes oficial para el lenguaje de programación Jasboot. Permite crear, distribuir, instalar y gestionar paquetes de código Jasboot de forma sencilla y eficiente.

### ¿Qué es JPM?

JPM es similar a `npm` (Node.js), `pip` (Python) o `cargo` (Rust), pero diseñado específicamente para Jasboot con soporte nativo para:

- 📦 **Empaquetado estándar** - Formato `.jpkg` basado en ZIP
- 🔒 **Integridad verificada** - Hash SHA-512 automático
- 📝 **Metadatos JSON** - Configuración con `jasboot.json`
- 🔗 **Dependencias** - Gestión automática de dependencias
- 🌐 **Multiplataforma** - Windows, Linux, macOS
- 🧠 **Soporte JMN** - Integración con Jasboot Memory Neural

---

## 📊 Estado del Proyecto

### ✅ Implementado (Fase 1 - 100%)

| Componente | Estado | Descripción |
|------------|--------|-------------|
| Core Engine | ✅ | Inicialización, configuración, logging |
| Parser JSON | ✅ | Parser completo sin dependencias externas |
| Metadatos | ✅ | Lectura/escritura de jasboot.json |
| Comandos Básicos | ✅ | init, list, info, clean, uninstall |
| Empaquetado | ✅ | Crear/extraer archivos .jpkg (ZIP) |
| Instalación Local | ✅ | Instalar desde archivos .jpkg |
| Lock File | ✅ | Gestión de jpm.lock |
| Hash SHA-512 | ✅ | Verificación de integridad |
| Versionado Semántico | ✅ | Comparación y validación (^, ~, >=) |

### ⏳ Pendiente (Fases 2 y 3)

| Componente | Estado | Descripción |
|------------|--------|-------------|
| Cliente HTTP | ⏳ | Descarga de paquetes remotos |
| Registro Central | ⏳ | https://registry.jasboot.org |
| Publicación | ⏳ | Subir paquetes al registro |
| Búsqueda | ⏳ | Buscar en el registro |
| Actualización | ⏳ | Actualizar paquetes instalados |
| Scripts | ⏳ | Ejecutar scripts de jasboot.json |

---

## ✨ Características

### Funcionalidades Actuales

#### 1. **Inicialización de Paquetes**
```bash
jpm init mi-paquete
```
Crea automáticamente:
- `jasboot.json` - Metadatos del paquete
- `src/main.jasb` - Archivo principal
- `README.md` - Documentación
- `.gitignore` y `.jpmignore` - Archivos de configuración
- Estructura de directorios: `tests/`, `docs/`

#### 2. **Empaquetado (.jpkg)**
```bash
jpm pack
```
- Crea archivo ZIP estándar con extensión `.jpkg`
- Calcula hash SHA-512 para verificación de integridad
- Respeta patrones de `.jpmignore`
- Nomenclatura: `nombre-version.jpkg`

#### 3. **Instalación Local**
```bash
jpm install paquete.jpkg
jpm install ./ruta/al/paquete.jpkg
```
- Extrae paquete a `a-modulos/`
- Verifica integridad con SHA-512
- Instala dependencias recursivamente
- Actualiza `jpm.lock` automáticamente

#### 4. **Gestión de Paquetes**
```bash
jpm list              # Lista paquetes instalados
jpm info mi-paquete   # Información detallada
jpm uninstall paquete # Desinstala paquete
jpm clean             # Limpia caché
```

#### 5. **Directorio de Dependencias**
JPM usa `a-modulos/` en lugar de `node_modules/`:
```
mi-proyecto/
├── a-modulos/          # Dependencias instaladas
│   ├── paquete-1/
│   └── paquete-2/
├── jasboot.json        # Metadatos del proyecto
├── jpm.lock            # Lock file
└── src/
    └── main.jasb
```

---

## 🔧 Compilación

### Requisitos

- **Compilador C:** GCC 7+ o Clang 8+ con soporte C11
- **Sistema Operativo:** Windows, Linux, macOS
- **Make** (opcional, para Unix/Linux/macOS)

### Opción 1: Makefile (Unix/Linux/macOS)

```bash
cd sdk-dependiente/manejador-de-paquetes/jpm-nuevo

# Compilar versión release
make

# Compilar versión debug
make debug

# Limpiar archivos de compilación
make clean

# Instalar en el sistema (requiere permisos)
sudo make install
```

### Opción 2: Script batch (Windows)

```cmd
cd sdk-dependiente\manejador-de-paquetes\jpm-nuevo

REM Compilar versión release
build.bat

REM Compilar versión debug
build.bat debug

REM Limpiar
build.bat clean
```

### Opción 3: Compilación manual

```bash
# Crear directorios
mkdir -p build bin

# Compilar todos los archivos
gcc -std=c11 -Wall -Wextra -Iinclude -O2 \
    src/jpm_main.c \
    src/jpm_core.c \
    src/jpm_json.c \
    src/jpm_metadata.c \
    src/jpm_commands.c \
    src/jpm_pack.c \
    src/jpm_install.c \
    src/jpm_lock.c \
    src/jpm_stubs.c \
    -o bin/jpm

# Windows
gcc -std=c11 -Wall -Wextra -Iinclude -O2 src/jpm_*.c -o bin/jpm.exe
```

### Resultado

Ejecutable generado: `bin/jpm.exe` (Windows) o `bin/jpm` (Unix/Linux/macOS)

Tamaño aproximado: **147 KB**

---

## 📥 Instalación

### Instalación en el Sistema

#### Unix/Linux/macOS

```bash
# Compilar e instalar
make && sudo make install

# O manualmente
sudo cp bin/jpm /usr/local/bin/jpm
sudo chmod +x /usr/local/bin/jpm
```

#### Windows

1. Compilar con `build.bat`
2. Copiar `bin/jpm.exe` a un directorio en tu `PATH`
3. O agregar `bin/` a tu variable de entorno `PATH`

### Verificar Instalación

```bash
jpm --version
# Output: jpm version 0.2.0
```

---

## 🚀 Uso Rápido

### 1. Crear un Nuevo Paquete

```bash
# Crear paquete
jpm init mi-libreria

# Navegar al directorio
cd mi-libreria

# Ver estructura creada
tree
```

### 2. Editar jasboot.json

```json
{
  "nombre": "mi-libreria",
  "version": "1.0.0",
  "descripcion": "Una librería increíble para Jasboot",
  "autor": "Tu Nombre",
  "licencia": "MIT",
  "principal": "src/main.jasb",
  "jasboot": "^1.0.0",
  "tipo": "biblioteca",
  "dependencias": {
    "otra-libreria": "^2.1.0"
  }
}
```

### 3. Empaquetar

```bash
# Crear paquete .jpkg
jpm pack

# Resultado: mi-libreria-1.0.0.jpkg
```

### 4. Instalar en Otro Proyecto

```bash
# En otro proyecto
jpm install ruta/a/mi-libreria-1.0.0.jpkg

# Verificar instalación
jpm list
```

---

## 📚 Comandos

### Comandos Principales

#### `jpm init <nombre>`
Inicializa un nuevo paquete.

```bash
jpm init mi-paquete
```

**Opciones:**
- Crea estructura completa de directorios
- Genera `jasboot.json` con valores por defecto
- Crea archivos iniciales

---

#### `jpm pack [directorio]`
Empaqueta el proyecto actual en formato `.jpkg`.

```bash
jpm pack              # Empaqueta directorio actual
jpm pack ./mi-lib     # Empaqueta directorio específico
```

**Salida:**
- Archivo `nombre-version.jpkg`
- Hash SHA-512 para verificación

---

#### `jpm install [fuente]`
Instala paquete(s).

```bash
# Instalar desde archivo local
jpm install paquete.jpkg

# Instalar todas las dependencias de jasboot.json
jpm install

# Con opciones
jpm install paquete.jpkg --global
jpm install paquete.jpkg --force
```

**Opciones:**
- `-g, --global` - Instalación global (~/.jpm/paquetes/)
- `-f, --force` - Forzar reinstalación
- `--dry-run` - Simular sin instalar

---

#### `jpm uninstall <paquete>`
Desinstala un paquete.

```bash
jpm uninstall mi-paquete

# Con opciones
jpm uninstall mi-paquete --global
```

---

#### `jpm list`
Lista paquetes instalados.

```bash
jpm list           # Paquetes locales
jpm list --global  # Paquetes globales
```

**Salida:**
```
Paquetes instalados en: ./a-modulos

  mi-libreria@1.0.0
    Descripción: Una librería increíble

  otra-libreria@2.1.5
    Descripción: Otra librería útil

Total: 2 paquetes
```

---

#### `jpm info <paquete>`
Muestra información detallada de un paquete.

```bash
jpm info mi-libreria
```

**Salida:**
```
Nombre:       mi-libreria
Versión:      1.0.0
Descripción:  Una librería increíble
Autor:        Tu Nombre
Licencia:     MIT
Principal:    src/main.jasb
Jasboot:      ^1.0.0
Tipo:         biblioteca
Tamaño:       15.3 KB

Dependencias:
  otra-libreria: ^2.1.0

Scripts:
  test: jbc tests/*.jasb
  build: jbc src/main.jasb -o build/main.jbo
```

---

#### `jpm clean`
Limpia caché y archivos temporales.

```bash
jpm clean
```

Elimina:
- `.jpm-cache/`
- `.jpm-temp/`

---

### Comandos de Información

#### `jpm version`
Muestra la versión de JPM.

```bash
jpm version
jpm --version
jpm -v
```

---

#### `jpm help`
Muestra ayuda general.

```bash
jpm help
jpm --help
jpm -h
```

---

### Comandos Futuros (No Implementados)

Estos comandos están planificados para versiones futuras:

```bash
jpm search <término>     # Buscar en registro central
jpm publish              # Publicar paquete
jpm update [paquete]     # Actualizar paquete(s)
jpm run <script>         # Ejecutar script
jpm login                # Autenticarse
```

---

## 📦 Formato .jpkg

### Estructura del Paquete

Un archivo `.jpkg` es un archivo ZIP estándar con la siguiente estructura:

```
mi-paquete-1.0.0.jpkg (archivo ZIP)
├── jasboot.json          # Metadatos obligatorios
├── src/                  # Código fuente
│   └── main.jasb        # Punto de entrada
├── tests/                # Tests (opcional)
├── docs/                 # Documentación (opcional)
└── README.md            # README (opcional)
```

### jasboot.json - Campos

#### Campos Obligatorios

```json
{
  "nombre": "mi-paquete",
  "version": "1.0.0"
}
```

#### Campos Recomendados

```json
{
  "nombre": "mi-paquete",
  "version": "1.0.0",
  "descripcion": "Descripción breve del paquete",
  "autor": "Nombre del Autor",
  "licencia": "MIT",
  "principal": "src/main.jasb",
  "jasboot": "^1.0.0",
  "tipo": "biblioteca"
}
```

#### Campos Completos

```json
{
  "nombre": "mi-paquete",
  "version": "1.0.0",
  "descripcion": "Un paquete completo para Jasboot",
  "autor": "Nombre del Autor",
  "licencia": "MIT",
  "principal": "src/main.jasb",
  "jasboot": "^1.0.0",
  "tipo": "biblioteca",
  "repositorio": "https://github.com/usuario/mi-paquete",
  "homepage": "https://mi-paquete.org",
  "palabrasClave": ["ia", "neuronal", "jmn"],
  
  "dependencias": {
    "libreria-1": "^2.0.0",
    "libreria-2": "~1.5.3"
  },
  
  "dependenciasDev": {
    "test-framework": "^0.5.0"
  },
  
  "scripts": {
    "test": "jbc tests/*.jasb",
    "build": "jbc src/main.jasb -o build/main.jbo",
    "start": "jasboot-ir-vm build/main.jbo"
  }
}
```

### Tipos de Paquete

- `"biblioteca"` - Librería reutilizable
- `"aplicacion"` - Aplicación ejecutable
- `"plugin"` - Plugin o extensión
- `"herramienta"` - Herramienta de desarrollo

### Versionado Semántico

JPM usa versionado semántico (SemVer):

```
MAJOR.MINOR.PATCH

Ejemplo: 1.2.3
```

#### Rangos de Versiones

```json
{
  "dependencias": {
    "paquete1": "1.2.3",      // Exactamente 1.2.3
    "paquete2": "^1.2.3",     // >=1.2.3 <2.0.0
    "paquete3": "~1.2.3",     // >=1.2.3 <1.3.0
    "paquete4": ">=1.2.3",    // 1.2.3 o superior
    "paquete5": "<2.0.0"      // Menor a 2.0.0
  }
}
```

---

## 🏗️ Arquitectura

### Módulos Principales

```
jpm-nuevo/
├── include/
│   └── jpm.h              # API pública completa
│
├── src/
│   ├── jpm_main.c         # Punto de entrada, parseo CLI
│   ├── jpm_core.c         # Core: init, config, logging
│   ├── jpm_json.c         # Parser JSON sin dependencias
│   ├── jpm_metadata.c     # Gestión de jasboot.json
│   ├── jpm_commands.c     # Comandos: init, list, info, etc
│   ├── jpm_pack.c         # Empaquetado/extracción .jpkg
│   ├── jpm_install.c      # Instalación de paquetes
│   ├── jpm_lock.c         # Gestión de jpm.lock
│   └── jpm_stubs.c        # Stubs de funciones futuras
│
├── Makefile               # Build system Unix/Linux/macOS
├── build.bat              # Build system Windows
└── README.md              # Esta documentación
```

### Flujo de Instalación

```
1. Usuario ejecuta: jpm install paquete.jpkg

2. jpm_install.c:
   - Verifica que archivo existe
   - Lee metadatos internos
   - Calcula hash SHA-512
   
3. jpm_pack.c:
   - Extrae .jpkg a a-modulos/paquete/
   - Verifica integridad
   
4. jpm_metadata.c:
   - Lee dependencias de jasboot.json
   
5. jpm_install.c (recursivo):
   - Instala dependencias faltantes
   
6. jpm_lock.c:
   - Actualiza jpm.lock
   
7. Completo ✅
```

### Sistema de Logging

JPM incluye un sistema de logging con 5 niveles:

```c
JPM_LOG_DEBUG    // Información de debug
JPM_LOG_INFO     // Información general
JPM_LOG_AVISO    // Advertencias
JPM_LOG_ERROR    // Errores
JPM_LOG_CRITICO  // Errores críticos
```

Uso:
```bash
jpm install paquete.jpkg --verbose  # Modo detallado
jpm install paquete.jpkg --silent   # Solo errores
```

---

## 💡 Ejemplos

### Ejemplo 1: Crear una Librería Simple

```bash
# 1. Crear paquete
jpm init calculadora

cd calculadora

# 2. Editar src/main.jasb
cat > src/main.jasb << 'EOF'
funcion sumar entero a, entero b retorna entero {
    retorna a + b
}

funcion restar entero a, entero b retorna entero {
    retorna a - b
}

funcion multiplicar entero a, entero b retorna entero {
    retorna a * b
}
EOF

# 3. Actualizar jasboot.json
# (editar manualmente con tu editor favorito)

# 4. Empaquetar
jpm pack

# Resultado: calculadora-0.1.0.jpkg
```

### Ejemplo 2: Usar una Librería

```bash
# 1. Crear proyecto
jpm init mi-app

cd mi-app

# 2. Instalar dependencia
jpm install ../calculadora/calculadora-0.1.0.jpkg

# 3. Verificar instalación
jpm list

# 4. Usar en tu código (src/main.jasb)
importar calculadora

funcion principal {
    entero resultado = calculadora.sumar(5, 3)
    imprimir("5 + 3 = ", resultado)
}
```

### Ejemplo 3: Paquete con Dependencias

```json
{
  "nombre": "app-compleja",
  "version": "2.0.0",
  "descripcion": "Aplicación con múltiples dependencias",
  "principal": "src/app.jasb",
  "tipo": "aplicacion",
  
  "dependencias": {
    "calculadora": "^0.1.0",
    "memoria-jmn": "^1.5.0",
    "analitica-neuronal": "~2.0.0"
  },
  
  "scripts": {
    "compilar": "jbc src/app.jasb -o build/app.jbo",
    "ejecutar": "jasboot-ir-vm build/app.jbo",
    "test": "jbc tests/*.jasb && jasboot-ir-vm tests/*.jbo"
  }
}
```

---

## 🗺️ Roadmap

### ✅ Fase 1: Prototipo Funcional (COMPLETADO)

- [x] Core engine y configuración
- [x] Parser JSON sin dependencias
- [x] Gestión de metadatos
- [x] Comandos básicos (init, list, info, clean, uninstall)
- [x] Empaquetado .jpkg (ZIP)
- [x] Instalación local
- [x] Sistema de lock (jpm.lock)
- [x] Hash SHA-512
- [x] Versionado semántico

### ⏳ Fase 2: Sistema Completo (EN PROGRESO)

- [ ] Cliente HTTP/HTTPS para descargas
- [ ] Instalación desde URL
- [ ] Registro central (https://registry.jasboot.org)
- [ ] Comando `jpm search`
- [ ] Comando `jpm publish`
- [ ] Comando `jpm update`
- [ ] Sistema de caché inteligente
- [ ] Portal web para explorar paquetes

### 🔮 Fase 3: Ecosistema Maduro (PLANIFICADO)

- [ ] Firma digital de paquetes (GPG)
- [ ] Scanner de seguridad
- [ ] Comando `jpm audit`
- [ ] Workspaces para monorepos
- [ ] Comando `jpm run` para scripts
- [ ] Sistema social (calificaciones, reseñas)
- [ ] CDN global para distribución
- [ ] Integración con VS Code
- [ ] CI/CD automatizado

---

## 🤝 Contribuir

### Reportar Bugs

Si encuentras un bug, por favor abre un issue con:

1. Descripción del problema
2. Pasos para reproducir
3. Comportamiento esperado vs actual
4. Versión de JPM (`jpm --version`)
5. Sistema operativo

### Solicitar Funcionalidades

¿Tienes una idea para mejorar JPM? ¡Genial!

1. Verifica que no exista un issue similar
2. Crea un nuevo issue describiendo la funcionalidad
3. Explica el caso de uso
4. Discute con la comunidad

### Contribuir Código

1. Fork el repositorio
2. Crea una rama para tu feature: `git checkout -b feature/mi-feature`
3. Implementa tus cambios
4. Agrega tests si es posible
5. Asegúrate de que compila sin warnings
6. Commit: `git commit -m "Agrega mi-feature"`
7. Push: `git push origin feature/mi-feature`
8. Abre un Pull Request

### Guía de Estilo

- **Lenguaje:** C11 estándar
- **Estilo:** snake_case para funciones y variables
- **Prefijos:** `jpm_` para funciones públicas
- **Comentarios:** En español
- **Documentación:** Completa y clara
- **Sin warnings:** Compilar con `-Wall -Wextra -Wpedantic`

---

## 📄 Licencia

MIT License - Ver archivo `LICENSE` para más detalles.

---

## 🙏 Agradecimientos

- Equipo Jasboot por el lenguaje increíble
- Comunidad de código abierto
- Todos los contribuidores

---

## 📞 Contacto

- **Proyecto Jasboot:** https://jasboot.org
- **Documentación:** https://jasboot.org/docs/jpm
- **Issues:** https://github.com/jasboot/jpm/issues
- **Discusiones:** https://github.com/jasboot/jpm/discussions

---

## 📈 Estadísticas

- **Líneas de código C:** ~6,500
- **Funciones implementadas:** 80+
- **Módulos:** 9
- **Tamaño ejecutable:** ~147 KB
- **Sin dependencias externas:** ✅
- **Multiplataforma:** ✅
- **Versión actual:** 0.2.0

---

**Hecho con ❤️ para la comunidad Jasboot**