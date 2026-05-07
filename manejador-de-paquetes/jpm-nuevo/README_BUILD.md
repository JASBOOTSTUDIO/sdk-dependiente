# Guía de Compilación - Jasboot Package Manager (JPM)

## Descripción

Este documento proporciona instrucciones detalladas para compilar el Jasboot Package Manager (JPM) desde el código fuente en diferentes plataformas.

**Versión:** 0.2.0  
**Última actualización:** 2024

---

## Requisitos del Sistema

### Requisitos Generales

- **Compilador C11:** GCC 7.0+, Clang 6.0+, o MSVC 2019+
- **Make:** GNU Make 3.81+ (opcional, recomendado para Linux/macOS)
- **Sistema Operativo:** Windows 7+, Linux (kernel 3.0+), macOS 10.12+

### Requisitos por Plataforma

#### Windows
- **MinGW-w64** (recomendado) o **MSVC**
- Git for Windows (opcional)
- Mínimo 50 MB de espacio en disco

#### Linux
- GCC instalado (`build-essential` en Debian/Ubuntu)
- Make
- Bibliotecas estándar de desarrollo

```bash
# Debian/Ubuntu
sudo apt-get install build-essential

# Fedora/RHEL
sudo dnf install gcc make

# Arch Linux
sudo pacman -S base-devel
```

#### macOS
- Xcode Command Line Tools

```bash
xcode-select --install
```

---

## Estructura del Proyecto

```
jpm-nuevo/
├── include/
│   └── jpm.h                 # Header principal
├── src/
│   ├── jpm_main.c            # Punto de entrada
│   ├── jpm_core.c            # Funciones core
│   ├── jpm_json.c            # Parser JSON
│   ├── jpm_metadata.c        # Gestión de metadatos
│   └── jpm_commands.c        # Comandos CLI
├── build/                    # Directorio de objetos (generado)
├── bin/                      # Directorio de ejecutables (generado)
├── Makefile                  # Makefile para Unix/Linux/macOS
├── build.bat                 # Script de build para Windows
└── README_BUILD.md           # Este archivo
```

---

## Compilación

### Método 1: Usando Make (Linux/macOS/MinGW en Windows)

#### Compilación Básica

```bash
# Clonar o navegar al directorio
cd jpm-nuevo

# Compilar versión release (optimizada)
make

# El ejecutable estará en bin/jpm (o bin/jpm.exe en Windows)
```

#### Compilación en Modo Debug

```bash
# Compilar con símbolos de depuración
make debug

# Verificar el ejecutable
./bin/jpm --version
```

#### Otras Opciones de Make

```bash
# Ver información de compilación
make info

# Limpiar archivos compilados
make clean

# Limpiar todo (incluye ejecutables)
make distclean

# Ejecutar tests básicos
make test

# Instalar en el sistema (requiere permisos)
sudo make install

# Desinstalar del sistema
sudo make uninstall

# Ver ayuda
make help
```

#### Compilación Manual (sin Make)

```bash
# Crear directorios
mkdir -p build bin

# Compilar cada archivo fuente
gcc -std=c11 -Wall -Wextra -Iinclude -O2 -c src/jpm_main.c -o build/jpm_main.o
gcc -std=c11 -Wall -Wextra -Iinclude -O2 -c src/jpm_core.c -o build/jpm_core.o
gcc -std=c11 -Wall -Wextra -Iinclude -O2 -c src/jpm_json.c -o build/jpm_json.o
gcc -std=c11 -Wall -Wextra -Iinclude -O2 -c src/jpm_metadata.c -o build/jpm_metadata.o
gcc -std=c11 -Wall -Wextra -Iinclude -O2 -c src/jpm_commands.c -o build/jpm_commands.o

# Enlazar
gcc build/*.o -o bin/jpm

# En Linux puede necesitar -lm para matemáticas
gcc build/*.o -lm -o bin/jpm
```

### Método 2: Usando build.bat (Windows)

#### Compilación Básica

```cmd
REM Abrir CMD o PowerShell en el directorio jpm-nuevo

REM Compilar versión release
build.bat

REM El ejecutable estará en bin\jpm.exe
```

#### Otras Opciones

```cmd
REM Compilar en modo debug
build.bat debug

REM Ver información de compilación
build.bat info

REM Limpiar archivos compilados
build.bat clean

REM Limpiar todo
build.bat distclean

REM Ejecutar tests
build.bat test

REM Ver ayuda
build.bat help
```

### Método 3: Usando Visual Studio (Windows)

1. Abrir **Visual Studio 2019+**
2. Crear nuevo proyecto **Console App (C)**
3. Agregar todos los archivos `.c` de `src/` al proyecto
4. Configurar **Include Directories** para apuntar a `include/`
5. Configurar **C/C++ → Language → C Language Standard** a **ISO C11**
6. Compilar (Ctrl+Shift+B)

---

## Verificación de la Compilación

Una vez compilado, verifica que el ejecutable funciona correctamente:

```bash
# Verificar que el ejecutable existe
ls -lh bin/jpm        # Linux/macOS
dir bin\jpm.exe       # Windows

# Intentar ejecutar (puede mostrar ayuda o error si no hay argumentos)
./bin/jpm             # Linux/macOS
bin\jpm.exe           # Windows
```

---

## Flags de Compilación

### Flags Estándar (incluidos en Makefile/build.bat)

- `-std=c11` - Usar estándar C11
- `-Wall` - Activar todos los warnings comunes
- `-Wextra` - Activar warnings adicionales
- `-Wpedantic` - Warnings sobre conformidad con el estándar
- `-Iinclude` - Directorio de headers

### Flags de Optimización

**Release (por defecto):**
- `-O2` - Optimización nivel 2 (buen balance)
- `-DNDEBUG` - Desactivar asserts

**Debug:**
- `-g` - Incluir símbolos de depuración
- `-O0` - Sin optimización
- `-DDEBUG` - Activar modo debug

### Flags Opcionales

```bash
# Optimización agresiva (puede causar bugs en código incorrecto)
-O3

# Optimizar para tamaño
-Os

# Warnings como errores
-Werror

# Información de stack para debugging
-fno-omit-frame-pointer

# Sanitizers (para detectar bugs)
-fsanitize=address          # Detecta errores de memoria
-fsanitize=undefined        # Detecta comportamiento indefinido
-fsanitize=leak             # Detecta memory leaks
```

---

## Compilación con Diferentes Compiladores

### GCC

```bash
gcc -std=c11 -Wall -Wextra -Iinclude -O2 \
    src/*.c -o bin/jpm
```

### Clang

```bash
clang -std=c11 -Wall -Wextra -Iinclude -O2 \
    src/*.c -o bin/jpm
```

### MSVC (Visual Studio Command Prompt)

```cmd
cl /std:c11 /W4 /O2 /I include ^
    src\jpm_main.c ^
    src\jpm_core.c ^
    src\jpm_json.c ^
    src\jpm_metadata.c ^
    src\jpm_commands.c ^
    /Fe:bin\jpm.exe
```

---

## Solución de Problemas

### Error: "gcc: command not found"

**Solución:** Instala GCC/MinGW según tu sistema operativo.

### Error: "fatal error: jpm.h: No such file or directory"

**Solución:** Verifica que estés compilando desde el directorio correcto y que `include/jpm.h` existe.

```bash
# Verifica la estructura
ls -la include/jpm.h
ls -la src/*.c
```

### Error: "undefined reference to 'mkdir'"

**En Windows con MinGW:** Este error puede ocurrir si no estás usando las bibliotecas correctas.

**Solución:**
```bash
# Asegúrate de compilar en Windows con -D_WIN32
gcc -std=c11 -D_WIN32 -Iinclude src/*.c -o bin/jpm.exe
```

### Error: "Permission denied" al ejecutar make install

**Solución:** Necesitas permisos de administrador.

```bash
sudo make install
```

### Warnings sobre "implicit declaration of function"

**Solución:** Asegúrate de incluir los headers necesarios. Esto es normal si las funciones aún no están implementadas.

### Error: Compilación muy lenta

**Solución:** Usa compilación paralela con Make:

```bash
make -j4    # Compila con 4 procesos en paralelo
make -j$(nproc)  # Usa todos los cores disponibles
```

---

## Pruebas Después de Compilar

### Pruebas Básicas

```bash
# 1. Verificar que el ejecutable existe y es ejecutable
./bin/jpm 2>&1

# 2. Probar comando init
mkdir test-package
cd test-package
../bin/jpm init mi-prueba

# 3. Verificar que se creó jasboot.json
cat jasboot.json

# 4. Probar comando list (debería estar vacío)
../bin/jpm list

# 5. Probar comando clean
../bin/jpm clean
```

### Pruebas con Valgrind (Linux/macOS)

```bash
# Detectar memory leaks
valgrind --leak-check=full ./bin/jpm init test-leak

# Verificar sin leaks
# Debe mostrar: "All heap blocks were freed -- no leaks are possible"
```

### Pruebas con AddressSanitizer

```bash
# Compilar con sanitizer
make clean
gcc -std=c11 -Wall -Iinclude -g -fsanitize=address src/*.c -o bin/jpm

# Ejecutar
./bin/jpm init test-asan
```

---

## Instalación en el Sistema

### Linux/macOS

```bash
# Compilar
make

# Instalar (requiere sudo)
sudo make install

# Verificar instalación
which jpm
jpm --version  # (si está implementado)

# Desinstalar
sudo make uninstall
```

Por defecto, se instala en `/usr/local/bin/jpm`. Para cambiar el prefijo:

```bash
make install PREFIX=/opt/jasboot
```

### Windows

En Windows, la instalación manual es más común:

1. Compila el ejecutable: `build.bat`
2. Copia `bin\jpm.exe` a un directorio en tu PATH
3. Opción recomendada: `C:\Program Files\Jasboot\bin\`
4. Agrega ese directorio a tu PATH en Variables de Entorno

---

## Compilación Cruzada

### Compilar para Windows desde Linux

```bash
# Instalar MinGW cross-compiler
sudo apt-get install mingw-w64

# Compilar
x86_64-w64-mingw32-gcc -std=c11 -Iinclude src/*.c -o jpm.exe
```

### Compilar para Linux desde macOS

```bash
# Generalmente funciona directamente
gcc -std=c11 -Iinclude src/*.c -o jpm
```

---

## Depuración

### GDB (Linux/macOS)

```bash
# Compilar con símbolos de debug
make debug

# Ejecutar en GDB
gdb bin/jpm

# Dentro de GDB:
(gdb) run init test-debug
(gdb) backtrace
(gdb) list
(gdb) quit
```

### LLDB (macOS)

```bash
lldb bin/jpm
(lldb) run init test-debug
(lldb) bt
(lldb) quit
```

### Visual Studio Debugger (Windows)

1. Compila en modo Debug
2. Abre Visual Studio
3. File → Open → Project/Solution
4. Selecciona el `.sln` o crea uno nuevo
5. F5 para debuggear

---

## Optimización y Profiling

### Profiling con gprof

```bash
# Compilar con profiling
gcc -std=c11 -pg -Iinclude src/*.c -o bin/jpm

# Ejecutar
./bin/jpm init test-profile

# Generar reporte
gprof bin/jpm gmon.out > profile.txt
cat profile.txt
```

### Profiling con perf (Linux)

```bash
# Compilar normalmente
make

# Ejecutar con perf
perf record ./bin/jpm init test-perf
perf report
```

---

## Información de los Módulos Compilados

Los siguientes módulos serán compilados:

### Módulos Implementados (Funcionales)

1. **jpm_core.c** - Funciones core, logging, validación
2. **jpm_json.c** - Parser JSON completo
3. **jpm_metadata.c** - Gestión de jasboot.json
4. **jpm_commands.c** - Comandos: init, list, info, clean, uninstall
5. **jpm_main.c** - Punto de entrada

### Módulos Pendientes (Stubs)

- jpm_install.c - Instalación de paquetes
- jpm_pack.c - Empaquetado
- jpm_registry.c - Interacción con registro
- jpm_http.c - Cliente HTTP/HTTPS
- jpm_lock.c - Gestión de jpm.lock

---

## Contribuir

Si deseas contribuir al desarrollo de JPM:

1. Asegúrate de que tu código compila sin warnings
2. Usa el estilo de código existente
3. Prueba en múltiples plataformas si es posible
4. Documenta nuevas funciones

---

## Soporte

Para reportar problemas de compilación:

1. Incluye tu sistema operativo y versión
2. Incluye la versión del compilador (`gcc --version`)
3. Incluye el error completo de compilación
4. Menciona los pasos que seguiste

---

## Licencia

Este código es parte del proyecto Jasboot.

---

**Última actualización:** 2024  
**Mantenido por:** Equipo Jasboot