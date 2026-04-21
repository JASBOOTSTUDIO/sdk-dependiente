# 🚀 EMPEZAR AQUÍ - Jasboot Package Manager (JPM)

**¡Bienvenido al gestor de paquetes de Jasboot!**

Este documento te guiará para empezar a usar JPM en **menos de 5 minutos**.

---

## ✅ VERIFICACIÓN RÁPIDA

El JPM ya está **compilado y listo** para usar:

```bash
# Verificar que funciona
sdk-dependiente/bin/jpm.exe --version

# Deberías ver:
# jpm version 0.2.0
# Jasboot Package Manager - Sistema de gestión de paquetes
```

✅ **Si ves esto, JPM está funcionando correctamente.**

---

## 📍 UBICACIÓN DEL EJECUTABLE

```
C:\src\jasboot\sdk-dependiente\bin\jpm.exe
```

### Agregar a tu PATH (Opcional pero Recomendado)

**Windows:**
1. Presiona `Win + R`, escribe `sysdm.cpl` y presiona Enter
2. Ve a "Opciones Avanzadas" → "Variables de Entorno"
3. En "Variables del sistema", busca `Path` y haz clic en "Editar"
4. Agrega: `C:\src\jasboot\sdk-dependiente\bin`
5. Haz clic en "Aceptar" en todas las ventanas

**O simplemente crea un alias:**

```bash
# En tu shell (Git Bash, PowerShell, etc.)
alias jpm='C:/src/jasboot/sdk-dependiente/bin/jpm.exe'
```

---

## 🎯 PRIMER USO - 3 MINUTOS

### Paso 1: Crear tu primer paquete

```bash
# Navega a un directorio de trabajo
cd C:\temp

# Crea un nuevo paquete
C:\src\jasboot\sdk-dependiente\bin\jpm.exe init mi-primera-libreria

# Entra al directorio
cd mi-primera-libreria

# Verifica la estructura creada
dir
```

**Verás:**
```
📁 src/              ← Tu código aquí
📁 tests/            ← Tests aquí
📁 docs/             ← Documentación aquí
📄 jasboot.json      ← Metadatos del paquete
📄 README.md         ← Documentación
📄 .gitignore        ← Exclusiones Git
📄 .jpmignore        ← Exclusiones JPM
```

### Paso 2: Editar tu código

Abre `src/main.jasb` y escribe algo simple:

```jasboot
# Librería de saludos
funcion saludar texto nombre retorna texto {
    retorna "¡Hola, " + nombre + "!"
}

funcion despedir texto nombre retorna texto {
    retorna "¡Adiós, " + nombre + "!"
}
```

### Paso 3: Empaquetar

```bash
# Crea el archivo .jpkg
C:\src\jasboot\sdk-dependiente\bin\jpm.exe pack

# Verás algo como:
# [INFO] Creando paquete: mi-primera-libreria-0.1.0.jpkg
# [INFO] Paquete creado exitosamente
```

### Paso 4: Usar tu paquete en otro proyecto

```bash
# Crea otro proyecto
cd C:\temp
C:\src\jasboot\sdk-dependiente\bin\jpm.exe init mi-app

cd mi-app

# Instala tu librería
C:\src\jasboot\sdk-dependiente\bin\jpm.exe install ..\mi-primera-libreria\mi-primera-libreria-0.1.0.jpkg

# Verifica que se instaló
C:\src\jasboot\sdk-dependiente\bin\jpm.exe list
```

**¡Felicidades! 🎉 Ya creaste, empaquetaste e instalaste tu primer paquete JPM.**

---

## 📚 COMANDOS PRINCIPALES

### Crear paquete nuevo
```bash
jpm init nombre-del-paquete
```

### Empaquetar proyecto actual
```bash
jpm pack
```

### Instalar paquete local
```bash
jpm install paquete.jpkg
jpm install ../ruta/al/paquete.jpkg
```

### Instalar todas las dependencias (desde jasboot.json)
```bash
jpm install
```

### Listar paquetes instalados
```bash
jpm list
```

### Ver información de un paquete
```bash
jpm info nombre-paquete
```

### Desinstalar paquete
```bash
jpm uninstall nombre-paquete
```

### Limpiar caché
```bash
jpm clean
```

### Ver ayuda
```bash
jpm help
jpm --help
```

---

## 📝 ARCHIVO jasboot.json

Cada paquete tiene un archivo `jasboot.json` con sus metadatos:

```json
{
  "nombre": "mi-paquete",
  "version": "1.0.0",
  "descripcion": "Mi paquete increíble",
  "autor": "Tu Nombre",
  "licencia": "MIT",
  "principal": "src/main.jasb",
  "jasboot": "^1.0.0",
  "tipo": "biblioteca",
  
  "dependencias": {
    "otro-paquete": "^1.2.0"
  }
}
```

**Campos importantes:**
- `nombre` - Nombre único del paquete (obligatorio)
- `version` - Versión semántica X.Y.Z (obligatorio)
- `descripcion` - Qué hace tu paquete
- `autor` - Tu nombre
- `licencia` - Licencia del código (MIT, Apache, etc.)
- `principal` - Archivo de entrada (generalmente src/main.jasb)
- `dependencias` - Otros paquetes que necesitas

---

## 🔧 DEPENDENCIAS

### Agregar dependencias manualmente

Edita `jasboot.json`:

```json
{
  "dependencias": {
    "calculadora": "^1.0.0",
    "memoria-jmn": "~2.1.0",
    "analitica": ">=1.5.0"
  }
}
```

**Rangos de versión:**
- `1.2.3` - Exactamente 1.2.3
- `^1.2.3` - Compatible: ≥1.2.3 pero <2.0.0
- `~1.2.3` - Compatible en parche: ≥1.2.3 pero <1.3.0
- `>=1.2.3` - Mayor o igual a 1.2.3
- `<2.0.0` - Menor a 2.0.0

### Instalar dependencias

```bash
# Instala todas las dependencias listadas en jasboot.json
jpm install
```

---

## 📂 ESTRUCTURA DE DIRECTORIOS

Cuando instalas paquetes, JPM crea la carpeta `a-modulos/`:

```
mi-proyecto/
├── a-modulos/          ← Paquetes instalados aquí
│   ├── paquete-1/
│   └── paquete-2/
├── src/
│   └── main.jasb
├── jasboot.json        ← Metadatos de tu proyecto
├── jpm.lock            ← Lock file (creado automáticamente)
└── README.md
```

**No necesitas hacer nada con `a-modulos/`**, JPM lo gestiona automáticamente.

---

## 🎓 EJEMPLO COMPLETO

### 1. Crear una librería de matemáticas

```bash
cd C:\workspace
jpm init matematicas-basicas
cd matematicas-basicas
```

Edita `src/main.jasb`:

```jasboot
funcion sumar entero a, entero b retorna entero {
    retorna a + b
}

funcion restar entero a, entero b retorna entero {
    retorna a - b
}

funcion multiplicar entero a, entero b retorna entero {
    retorna a * b
}
```

Edita `jasboot.json`:

```json
{
  "nombre": "matematicas-basicas",
  "version": "1.0.0",
  "descripcion": "Operaciones matemáticas básicas",
  "autor": "Tu Nombre",
  "licencia": "MIT",
  "principal": "src/main.jasb",
  "jasboot": "^1.0.0",
  "tipo": "biblioteca"
}
```

Empaqueta:

```bash
jpm pack
# Crea: matematicas-basicas-1.0.0.jpkg
```

### 2. Usar la librería en una aplicación

```bash
cd C:\workspace
jpm init calculadora-app
cd calculadora-app
```

Instala la librería:

```bash
jpm install ..\matematicas-basicas\matematicas-basicas-1.0.0.jpkg
```

Edita `src/main.jasb`:

```jasboot
importar matematicas_basicas

funcion principal {
    entero x = 10
    entero y = 5
    
    entero suma = matematicas_basicas.sumar(x, y)
    imprimir("10 + 5 = ", suma)
    
    entero resta = matematicas_basicas.restar(x, y)
    imprimir("10 - 5 = ", resta)
}
```

Compila y ejecuta (cuando tengas el compilador listo):

```bash
jbc src/main.jasb -o build/app.jbo
jasboot-ir-vm build/app.jbo
```

---

## ❓ PREGUNTAS FRECUENTES

### ¿Dónde se instalan los paquetes?

En `./a-modulos/` (directorio local) o `~/.jpm/paquetes/` (global con `--global`)

### ¿Puedo instalar desde internet?

**No todavía.** Por ahora solo paquetes locales (archivos .jpkg).
La instalación desde URLs estará en la Fase 2.

### ¿Puedo publicar mis paquetes?

**No todavía.** El registro central (`jpm publish`) estará en la Fase 2.

### ¿Cómo actualizo un paquete?

Por ahora: desinstala (`jpm uninstall`) y reinstala la nueva versión.
El comando `jpm update` estará en la Fase 2.

### ¿Qué es jpm.lock?

Es un archivo que JPM crea automáticamente para "bloquear" las versiones exactas
de tus dependencias. Esto asegura que todos tengan las mismas versiones.
**Debes incluirlo en Git.**

### ¿Qué hago si algo no funciona?

1. Verifica la versión: `jpm --version`
2. Usa modo verboso: `jpm install paquete.jpkg --verbose`
3. Revisa los logs
4. Reporta el bug en el repositorio

---

## 📖 DOCUMENTACIÓN ADICIONAL

- **README completo:** `jpm-nuevo/README.md`
- **Guía de compilación:** `jpm-nuevo/README_BUILD.md`
- **Inicio rápido:** `jpm-nuevo/QUICKSTART.md`
- **Implementación completa:** `IMPLEMENTACION_JPM_COMPLETADA.md`
- **Estado del proyecto:** `ESTADO_ACTUAL_JPM.md`

---

## ✅ CHECKLIST DE INICIO

- [ ] Verificar que JPM funciona: `jpm --version`
- [ ] Crear primer paquete: `jpm init test`
- [ ] Empaquetar: `jpm pack`
- [ ] Instalar en otro proyecto: `jpm install test.jpkg`
- [ ] Listar paquetes: `jpm list`
- [ ] Leer README completo
- [ ] Explorar ejemplos

---

## 🚀 SIGUIENTE PASO

**Lee el README completo** para entender todas las capacidades de JPM:

```bash
# Windows
notepad jpm-nuevo\README.md

# O simplemente ábrelo con tu editor favorito
```

---

**¡Eso es todo! Ya estás listo para usar JPM. 🎉**

**¿Tienes preguntas?** Consulta la documentación completa en `jpm-nuevo/`.

---

*Jasboot Package Manager v0.2.0*  
*Implementado con ❤️ para la comunidad Jasboot*