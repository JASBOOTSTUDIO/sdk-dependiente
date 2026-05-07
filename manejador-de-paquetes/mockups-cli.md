# MOCKUPS DE INTERFAZ DE LÍNEA DE COMANDOS - JPM

## Diseño de la CLI de JPM

Interfaz de línea de comandos intuitiva, consistente y eficiente para el gestor de paquetes Jasboot.

---

## Comandos Principales

### 1. jpm init
```bash
# Uso básico
$ jpm init

# Con nombre específico
$ jpm init mi-paquete

# Con opciones
$ jpm init mi-paquete --version 1.0.0 --description "Mi primer paquete Jasboot"

# Salida esperada
? Nombre del paquete: mi-paquete
? Versión inicial: 1.0.0
? Descripción breve: Mi primer paquete Jasboot
? Tipo de paquete: [Use arrow keys]
  libreria
  aplicacion
  framework
  herramienta
? Licencia: MIT
? Autor: Juan Pérez
? Email: juan@ejemplo.com

# Progreso
[1/6] Creando estructura de directorios...
[2/6] Generando jasboot.json...
[3/6] Creando archivos plantilla...
[4/6] Inicializando repositorio git...
[5/6] Creando README.md...
[6/6] Configurando .jpmignore...

# Resultado
¡Paquete 'mi-paquete' creado exitosamente!
Estructura creada:
  src/main.jasb
  docs/README.md
  tests/test_main.jasb
  jasboot.json

Siguientes pasos:
  1. Editar src/main.jasb
  2. Ejecutar 'jpm pack' para crear tu paquete
  3. Ejecutar 'jpm publish' para compartirlo
```

### 2. jpm install
```bash
# Instalar desde registry
$ jpm install analitica-neuronal

# Instalar versión específica
$ jpm install analitica-neuronal@1.5.0

# Instalar desde URL
$ jpm install https://github.com/ejemplo/mi-paquete/releases/download/v1.0.0/mi-paquete-1.0.0.jpkg

# Instalar desde archivo local
$ jpm install ./mi-paquete-1.0.0.jpkg

# Instalar dependencias del proyecto
$ jpm install

# Instalación con progreso
$ jpm install analitica-neuronal
Resolviendo dependencias...
[1/4] Buscando 'analitica-neuronal'...
[2/4] Descargando analitica-neuronal@1.5.2 (2.3 MB) [==================] 100%
[3/4] Verificando integridad SHA-512...
[4/4] Extrayendo paquete...

Instalando dependencias:
[1/2] math-lib@1.0.0 (1.1 MB) [==================] 100%
[2/2] estadistica@2.1.0 (3.7 MB) [==================] 100%

Configurando paquete...
Creando enlaces simbólicos...

¡Paquete 'analitica-neuronal@1.5.2' instalado exitosamente!
Ubicación: ~/.jpm/paquetes/analitica-neuronal/1.5.2
```

### 3. jpm pack
```bash
# Empaquetar proyecto actual
$ jpm pack

# Con opciones
$ jpm pack --output ./dist --compress-level 9

# Salida esperada
$ jpm pack
Leyendo jasboot.json...
Validando estructura del paquete...
Calculando hash SHA-512...
Firmando paquete (si aplica)...

Creando paquete:
[1/5] Agregando archivos fuente...
[2/5] Agregando documentación...
[3/5] Agregando pruebas...
[4/5] Agregando recursos...
[5/5] Comprimiendo paquete...

¡Paquete creado exitosamente!
Archivo: mi-paquete-1.0.0.jpkg
Tamaño: 4.2 MB
Hash: abc123def456...
Firma: [VERIFICADA]

Para publicar: jpm publish mi-paquete-1.0.0.jpkg
```

### 4. jpm list
```bash
# Listar paquetes instalados
$ jpm list

# Con formato específico
$ jpm list --format table
$ jpm list --format json
$ jpm list --format tree

# Filtrar por tipo
$ jpm list --type libreria
$ jpm list --type aplicacion

# Salida esperada
$ jpm list
Paquetes instalados (5):

analitica-neuronal@1.5.2
  Tipo: libreria
  Tamaño: 2.3 MB
  Instalado: 2026-04-17 15:30
  Ubicación: ~/.jpm/paquetes/analitica-neuronal/1.5.2

aurora-ia@1.1.0
  Tipo: aplicacion
  Tamaño: 5.7 MB
  Instalado: 2026-04-16 10:15
  Ubicación: ~/.jpm/paquetes/aurora-ia/1.1.0

memoria-conversacional@2.0.1
  Tipo: libreria
  Tamaño: 1.8 MB
  Instalado: 2026-04-15 14:22
  Ubicación: ~/.jpm/paquetes/memoria-conversacional/2.0.1

Total: 5 paquetes, 12.5 MB
```

### 5. jpm info
```bash
# Información de paquete instalado
$ jpm info analitica-neuronal

# Información de versión específica
$ jpm info analitica-neuronal@1.5.0

# Formato JSON
$ jpm info analitica-neuronal --json

# Salida esperada
$ jpm info analitica-neuronal
analitica-neuronal@1.5.2
========================
Descripción: Librería de análisis neuronal y machine learning para Jasboot
Autor: Jasboot Team <team@jasboot.org>
Licencia: MIT
Repositorio: https://github.com/jasboot/analitica-neuronal
Tipo: libreria
Versión Jasboot: >=1.2.0

Dependencias:
- math-lib@^1.0.0
- estadistica@^2.0.0

Exporta:
- inicializar_analitica()
- analizar_dataset()
- entrenar_modelo()
- predecir()

JMN Configurado: Sí
- Memoria: persistente
- Cache: activado
- Tamaño: 100MB

Archivos:
- src/analitica.jasb (principal)
- src/base.jasb
- src/math_lib.jasb
- src/estadistica/
- src/machine_learning/
- src/prediccion/

Estadísticas:
- Descargas: 1,234
- Estrellas: 45
- Forks: 12
- Última actualización: 2026-04-10

Instalado:
- Versión: 1.5.2
- Ubicación: ~/.jpm/paquetes/analitica-neuronal/1.5.2
- Fecha: 2026-04-17 15:30
- Hash verificado: abc123def456...
```

---

## Comandos Avanzados

### 6. jpm search
```bash
# Búsqueda simple
$ jpm search memoria

# Búsqueda con filtros
$ jpm search --type libreria --keyword ia
$ jpm search --author "Jasboot Team"
$ jpm search --license MIT

# Salida esperada
$ jpm search memoria neuronal
Buscando "memoria neuronal"... [ENCONTRADOS 3]

memoria-conversacional@2.0.1
  Descripción: Sistema de memoria neuronal para conversaciones
  Autor: Jasboot Team
  Descargas: 892
  Estrellas: 23

jmn-core@1.5.0
  Descripción: Core de memoria neuronal Jasboot
  Autor: Jasboot Team
  Descargas: 2,456
  Estrellas: 67

neural-cache@1.0.0
  Descripción: Cache neuronal optimizado para Jasboot
  Autor: AI Dev
  Descargas: 156
  Estrellas: 8

Mostrando 3 de 3 resultados
```

### 7. jpm publish
```bash
# Publicar paquete
$ jpm publish ./mi-paquete-1.0.0.jpkg

# Publicar con metadatos
$ jpm publish ./mi-paquete-1.0.0.jpkg --tag "v1.0.0" --release-notes "Primera versión estable"

# Salida esperada
$ jpm publish ./mi-paquete-1.0.0.jpkg
Autenticando con registry.jasboot.org...
Usuario: juan.perez
Password: [OCULTO]

Verificando paquete...
[1/4] Validando estructura...
[2/4] Verificando hash SHA-512...
[3/4] Verificando firma...
[4/4] Analizando seguridad...

Subiendo paquete (4.2 MB)...
[==================] 100%

Publicando metadatos...
Registrando versión...

¡Paquete publicado exitosamente!
URL: https://registry.jasboot.org/mi-paquete
Versión: 1.0.0
Publicado: 2026-04-17 16:45

Compartir: https://registry.jasboot.org/mi-paquete/1.0.0
```

### 8. jpm update
```bash
# Actualizar todos los paquetes
$ jpm update

# Actualizar paquete específico
$ jpm update analitica-neuronal

# Actualizar con opciones
$ jpm update --dry-run
$ jpm update --force

# Salida esperada
$ jpm update
Verificando actualizaciones...

Paquetes actualizables (2):
analitica-neuronal: 1.5.0 -> 1.5.2 (corrección de bugs)
memoria-conversacional: 2.0.0 -> 2.0.1 (mejora de rendimiento)

¿Desea continuar? [y/N] y

Actualizando paquetes...
[1/2] analitica-neuronal@1.5.2 (2.3 MB) [==================] 100%
[2/2] memoria-conversacional@2.0.1 (1.8 MB) [==================] 100%

¡2 paquetes actualizados exitosamente!
```

### 9. jpm uninstall
```bash
# Desinstalar paquete
$ jpm uninstall analitica-neuronal

# Forzar desinstalación
$ jpm uninstall analitica-neuronal --force

# Salida esperada
$ jpm uninstall analitica-neuronal
¡Cuidado! Esto desinstalará 'analitica-neuronal@1.5.2'

Paquetes dependientes:
- mi-proyecto@1.0.0 (usa analitica-neuronal)

¿Desea continuar? [y/N] y

[1/3] Eliminando enlaces simbólicos...
[2/3] Eliminando archivos del paquete...
[3/3] Actualizando registro...

¡Paquete 'analitica-neuronal' desinstalado exitosamente!
```

---

## Comandos de Gestión

### 10. jpm login / logout
```bash
# Iniciar sesión
$ jpm login
Usuario: juan.perez
Password: [OCULTO]
¿Guardar token? [Y/n] y

¡Autenticación exitosa!
Token guardado en ~/.jpm/config/auth.json

# Ver usuario actual
$ jpm whoami
juan.perez (juan@ejemplo.com)
Autenticado: 2026-04-17 16:45
Token expira: 2026-07-17 16:45

# Cerrar sesión
$ jpm logout
¿Cerrar sesión en registry.jasboot.org? [y/N] y
¡Sesión cerrada exitosamente!
```

### 11. jpm cache
```bash
# Estado del cache
$ jpm cache stats
Cache de descargas:
  Tamaño: 847 MB / 1 GB (84.7%)
  Archivos: 234 / 1000
  Hit rate: 78.5%
  Última limpieza: 2026-04-17 15:30

Cache de metadatos:
  Tamaño: 45 MB / 100 MB (45%)
  Entradas: 1,234
  TTL: 1 hora

# Limpiar cache
$ jpm cache clean
¿Limpiar todo el cache? [y/N] y
[1/3] Limpiando cache de descargas...
[2/3] Limpiando cache de metadatos...
[3/3] Limpiando cache temporal...

¡Cache limpiado exitosamente!
Liberado: 892 MB

# Limpiar cache específico
$ jpm cache clean --type downloads
$ jpm cache clean --older-than 7d
```

### 12. jpm audit
```bash
# Auditoría de seguridad
$ jpm audit
Auditando paquetes instalados (5)...

[SEGURIDAD] Todos los paquetes tienen firma válida
[INTEGRIDAD] Todos los hashes SHA-512 verificados
[VULNERABILIDADES] 0 vulnerabilidades conocidas
[PERMISOS] Todos los permisos correctos

¡Auditoría completada sin problemas!

# Auditoría detallada
$ jpm audit --verbose
Analizando analitica-neuronal@1.5.2...
  [OK] Firma PGP verificada
  [OK] Hash SHA-512 coincide
  [OK] Sin vulnerabilidades conocidas
  [OK] Permisos de archivos correctos

Analizando memoria-conversacional@2.0.1...
  [OK] Firma PGP verificada
  [OK] Hash SHA-512 coincide
  [OK] Sin vulnerabilidades conocidas
  [OK] Permisos de archivos correctos

...
```

---

## Comandos de Desarrollo

### 13. jpm run
```bash
# Ejecutar script
$ jpm run start
[INFO] Iniciando aplicación...

# Ejecutar con variables de entorno
$ jpm run dev --env NODE_ENV=development

# Listar scripts disponibles
$ jpm run --list
Scripts disponibles:
  start     - Inicia la aplicación
  dev        - Inicia en modo desarrollo
  test       - Ejecuta pruebas
  build      - Compila el proyecto
  clean      - Limpia build
```

### 14. jpm doctor
```bash
# Diagnóstico del sistema
$ jpm doctor
Verificando sistema JPM...

[OK] Versión JPM: 1.0.0
[OK] Versión Jasboot: 1.2.0
[OK] Conexión a registry.jasboot.org
[OK] Permisos de ~/.jpm/
[OK] Espacio en disco: 45 GB disponible
[OK] Cache funcional
[OK] Base de datos local intacta

¡Sistema saludable!
```

---

## Opciones Globales

### Flags Comunes
```bash
# --help / -h
$ jpm --help
$ jpm install --help

# --version / -V
$ jpm --version
jpm 1.0.0
jasboot 1.2.0

# --verbose / -v
$ jpm install analitica-neuronal --verbose
[DEBUG] Resolviendo dependencias...
[DEBUG] Buscando en registry...
[INFO] Descargando paquete...

# --quiet / -q
$ jpm install analitica-neuronal --quiet
# Solo muestra errores y resultado final

# --dry-run
$ jpm update --dry-run
Paquetes que se actualizarían:
- analitica-neuronal: 1.5.0 -> 1.5.2
- memoria-conversacional: 2.0.0 -> 2.0.1

# --force / -f
$ jpm install analitica-neuronal --force
# Fuerza reinstalación

# --global / -g
$ jpm install analitica-neuronal --global
# Instala globalmente

# --config
$ jpm --config ~/.jpm/custom.json install analitica-neuronal
# Usa archivo de configuración específico
```

---

## Configuración

### Archivo de Configuración
```bash
# Ver configuración actual
$ jpm config
Registry URL: https://registry.jasboot.org
Cache Dir: ~/.jpm/cache
Max Cache Size: 1GB
Log Level: info
Timeout: 30s
Max Concurrent: 5

# Establecer configuración
$ jpm config set registry.url https://custom-registry.org
$ jpm config set cache.max_size 2GB
$ jpm config set log.level debug

# Obtener valor específico
$ jpm config get registry.url
https://custom-registry.org

# Restablecer valor
$ jpm config reset registry.url
```

---

## Salida Formateada

### Formatos Disponibles
```bash
# Tabla (default)
$ jpm list --format table

# JSON
$ jpm list --format json
[
  {
    "name": "analitica-neuronal",
    "version": "1.5.2",
    "type": "libreria",
    "size": 2345678
  }
]

# YAML
$ jpm list --format yaml
- name: analitica-neuronal
  version: 1.5.2
  type: libreria
  size: 2345678

# Tree
$ jpm list --format tree
mi-proyecto@1.0.0
  analitica-neuronal@1.5.2
    math-lib@1.0.0
    estadistica@2.0.0
  memoria-conversacional@2.0.1
```

---

## Integración con Shell

### Autocompletado
```bash
# Bash completion
$ jpm ins<TAB>
install  info

# Completion de paquetes
$ jpm install ana<TAB>
analitica-neuronal  analizador-texto

# Completion de versiones
$ jpm install analitica-neuronal@<TAB>
1.5.0  1.5.1  1.5.2
```

### Aliases Comunes
```bash
# Aliases sugeridos
alias jpmi='jpm install'
alias jpml='jpm list'
alias jpms='jpm search'
alias jpmu='jpm update'
alias jpmr='jpm run'

# Ejemplos
$ jpmi analitica-neuronal
$ jpml --type libreria
```

---

## Mensajes de Error

### Errores Comunes
```bash
# Paquete no encontrado
$ jpm install paquete-inexistente
Error: Paquete 'paquete-inexistente' no encontrado en registry
Sugerencias:
  - Verificar el nombre del paquete
  - Buscar paquetes similares: jpm search paquete

# Conflicto de dependencias
$ jpm install paquete-conflictivo
Error: Conflicto de dependencias: analitica-neuronal@^1.5.0 vs ^2.0.0
Solución: jpm install analitica-neuronal@2.0.0 --force

# Error de red
$ jpm install paquete
Error: No se puede conectar a registry.jasboot.org
Solución: Verificar conexión a internet o usar --offline

# Permiso denegado
$ jpm install --global paquete
Error: Permiso denegado para instalar en /usr/local/bin
Solución: Ejecutar con sudo o usar --user
```

---

## Interactividad

### Prompts Interactivos
```bash
# Confirmaciones
$ jpm uninstall paquete-importante
¡Cuidado! 'paquete-importante' es usado por 2 otros paquetes:
  - mi-proyecto@1.0.0
  - otro-proyecto@2.1.0

¿Está seguro? [y/N] 

# Selección múltiple
$ jpm update --interactive
Paquetes actualizables:
  [ ] analitica-neuronal@1.5.0 -> 1.5.2
  [ ] memoria-conversacional@2.0.0 -> 2.0.1
  [x] aurora-ia@1.0.0 -> 1.1.0

Seleccionar paquetes a actualizar [Espacio para seleccionar, Enter para confirmar]:

# Progreso con detalles
$ jpm install paquete-grande
[1/4] Descargando paquete-grande@2.0.0 (156.7 MB)
  Velocidad: 5.2 MB/s
  Tiempo restante: 28s
  Progreso: [========     ] 45%
```

---

*Estos mockups proporcionan una visión completa de la experiencia de usuario esperada con la CLI de JPM, asegurando consistencia, usabilidad y eficiencia en todas las operaciones.*
