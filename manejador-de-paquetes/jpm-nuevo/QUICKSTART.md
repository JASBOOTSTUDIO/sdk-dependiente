# JPM Quick Start - Inicio Rápido

**5 minutos para empezar a usar el Jasboot Package Manager**

---

## 🚀 Compilación Rápida

### Windows
```bash
build.bat
```

### Linux/macOS
```bash
make
```

**Resultado:** `bin/jpm.exe` (Windows) o `bin/jpm` (Linux/macOS)

---

## 📦 Comandos Básicos

### 1. Crear un nuevo proyecto
```bash
jpm init mi-proyecto
```

Crea `jasboot.json`:
```json
{
  "nombre": "mi-proyecto",
  "version": "1.0.0",
  "descripcion": "",
  "autor": "",
  "licencia": "MIT"
}
```

### 2. Empaquetar tu proyecto
```bash
jpm pack
```

Genera: `mi-proyecto-1.0.0.jpkg` con hash SHA-512 automático

### 3. Instalar un paquete
```bash
# Desde archivo local
jpm install ./paquete.jpkg

# Desde URL (requiere jpm_network.c)
jpm install https://ejemplo.com/paquete.jpkg

# Por nombre (requiere jpm_registry.c)
jpm install nombre-paquete@1.2.3
```

### 4. Instalar dependencias
```bash
# Instala todas las dependencias de jasboot.json
jpm install
```

### 5. Listar paquetes instalados
```bash
jpm list
```

---

## 📝 Ejemplo Completo

### Crear una biblioteca

```bash
# 1. Crear directorio
mkdir mi-biblioteca
cd mi-biblioteca

# 2. Inicializar proyecto
jpm init mi-biblioteca

# 3. Editar jasboot.json (agregar descripción, autor, etc.)

# 4. Crear código fuente
mkdir src
echo 'funcion saludar() { escribir("Hola desde mi biblioteca!") }' > src/principal.jasb

# 5. Crear .jpmignore (opcional)
echo "tests/" > .jpmignore
echo "*.tmp" >> .jpmignore

# 6. Empaquetar
jpm pack

# Resultado: mi-biblioteca-1.0.0.jpkg
```

### Usar una biblioteca

```bash
# 1. Crear aplicación
mkdir mi-app
cd mi-app
jpm init mi-app

# 2. Editar jasboot.json y agregar dependencia
{
  "nombre": "mi-app",
  "version": "1.0.0",
  "dependencias": {
    "mi-biblioteca": "^1.0.0"
  }
}

# 3. Instalar dependencias
jpm install

# 4. Usar en código
# src/principal.jasb:
# importar { saludar } desde "mi-biblioteca"
# funcion principal() { saludar() }
```

---

## 📁 Estructura de Directorios

```
mi-proyecto/
├── jasboot.json              # Metadatos del proyecto
├── jpm.lock                  # Versiones bloqueadas (generado automáticamente)
├── .jpmignore                # Archivos a ignorar al empaquetar
├── src/
│   └── principal.jasb
└── a-modulos/                # Dependencias instaladas (NO commitear en git)
    ├── biblioteca-a/
    └── biblioteca-b/
```

---

## 🔧 Solución Rápida de Problemas

### "GCC no encontrado"
```bash
# Instalar MinGW en Windows:
https://www.mingw-w64.org/

# Linux:
sudo apt install gcc make

# macOS:
xcode-select --install
```

### "jasboot.json no encontrado"
```bash
# Inicializar proyecto primero
jpm init mi-proyecto
```

### "Archivo ZIP inválido"
```bash
# Volver a empaquetar
jpm pack
```

### Ver logs detallados
```bash
jpm install --verboso
```

---

## 🎯 Archivo .jpmignore

Crear `.jpmignore` para excluir archivos del empaquetado:

```
# Comentarios con #
node_modules
.git/
.vscode/
*.tmp
*.log
tests/
docs/
```

---

## 🔒 Verificación de Integridad

Todos los paquetes incluyen hash SHA-512 automático:

```bash
# Al empaquetar
jpm pack
# [INFO] SHA-512: a3f5b9c2d8e1f4a7...

# Al instalar
jpm install paquete.jpkg
# [INFO] Verificando integridad...
# [EXITO] Hash verificado correctamente
```

---

## 📖 Más Información

- **Documentación completa:** `MODULOS_CRITICOS_IMPLEMENTADOS.md`
- **Guía de uso:** `README_MODULOS_CRITICOS.md`
- **Resumen técnico:** `IMPLEMENTACION_COMPLETA.md`

---

## ✅ Checklist Rápido

- [ ] Compilar JPM: `build.bat` o `make`
- [ ] Inicializar proyecto: `jpm init`
- [ ] Crear código fuente en `src/`
- [ ] Crear `.jpmignore` (opcional)
- [ ] Empaquetar: `jpm pack`
- [ ] Instalar: `jpm install archivo.jpkg`
- [ ] Agregar dependencias a `jasboot.json`
- [ ] Instalar dependencias: `jpm install`
- [ ] Usar en código: `importar { ... } desde "paquete"`

---

## 🎓 Próximos Pasos

1. Explorar opciones: `jpm --help`
2. Leer documentación completa
3. Crear tus propias bibliotecas
4. Compartir en el registro (próximamente)

---

**¡Listo para empaquetar y compartir código Jasboot!** 🚀