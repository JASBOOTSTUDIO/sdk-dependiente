# ESPECIFICACIÓN JPKG - Formato de Paquetes Jasboot

## Resumen

`.jpkg` es el formato de paquete para Jasboot, basado en ZIP con metadatos JSON estructurados.

---

## Estructura del Paquete

### Formato de Archivo
- **Extensión**: `.jpkg`
- **Contenido**: Archivo ZIP estándar
- **Compresión**: Deflate (estándar ZIP)

### Estructura Interna
```
nombre-paquete-version.jpkg
|
+-- jasboot.json          # Metadatos obligatorios
|
+-- src/                  # Código fuente (opcional)
|   +-- main.jasb        # Punto de entrada principal
|   +-- modulo1.jasb     # Módulos adicionales
|   +-- ...
|
+-- docs/                 # Documentación (opcional)
|   +-- README.md         # Documentación principal
|   +-- API.md           # Documentación de API
|   +-- ejemplos/        # Ejemplos de uso
|
+-- tests/                # Pruebas (opcional)
|   +-- test_*.jasb      # Suite de pruebas
|
+-- bin/                  # Ejecutables (opcional)
|   +-- *.exe            # Binarios compilados
|
+-- recursos/             # Recursos adicionales (opcional)
    +-- config/          # Archivos de configuración
    +-- datos/           # Datos de ejemplo
    +-- assets/          # Imágenes, etc.
```

---

## Metadatos (jasboot.json)

### Esquema JSON
```json
{
  "$schema": "https://jasboot.org/schema/jpkg-v1.json",
  "nombre": "nombre-paquete",
  "version": "1.0.0",
  "descripcion": "Descripción breve del paquete",
  "descripcionLarga": "Descripción detallada y completa del paquete",
  "autor": {
    "nombre": "Nombre del Autor",
    "email": "autor@ejemplo.com",
    "url": "https://github.com/autor"
  },
  "licencia": "MIT",
  "principal": "src/main.jasb",
  "tipo": "libreria|aplicacion|framework|herramienta",
  "jasbootVersion": ">=1.0.0",
  "dependencias": {
    "paquete-dependiente": "^1.2.0",
    "otro-paquete": "~2.0.0"
  },
  "dependenciasDesarrollo": {
    "paquete-test": "^0.5.0"
  },
  "scripts": {
    "compilar": "jbc src/main.jasb -o build/main.jbo",
    "ejecutar": "jasboot-ir-vm build/main.jbo",
    "test": "jbc tests/test_*.jasb && jasboot-ir-vm build/test_*.jbo",
    "limpiar": "rm -rf build/"
  },
  "palabrasClave": ["jasboot", "libreria", "ia", "memoria"],
  "repositorio": {
    "tipo": "git",
    "url": "https://github.com/usuario/nombre-paquete.git"
  },
  "bugs": {
    "url": "https://github.com/usuario/nombre-paquete/issues"
  },
  "pagina": "https://github.com/usuario/nombre-paquete#readme",
  "archivos": [
    "src/*.jasb",
    "docs/*.md",
    "tests/*.jasb"
  ],
  "ignorar": [
    "node_modules/",
    "build/",
    "*.tmp",
    ".DS_Store"
  ],
  "bin": {
    "comando": "bin/ejecutable.exe"
  },
  "exportar": [
    "funcion_principal",
    "clase_util",
    "constante_global"
  ],
  "jmn": {
    "requerido": true,
    "version": ">=2.0.0",
    "configuracion": {
      "memoria": "persistente",
      "cache": "activado"
    }
  },
  "compilacion": {
    "optimizacion": "velocidad",
    "debug": false,
    "advertencias": "estrictas"
  },
  "plataformas": [
    "windows",
    "linux",
    "macos"
  ],
  "fechaCreacion": "2026-04-17T18:00:00Z",
  "fechaPublicacion": "2026-04-17T18:00:00Z",
  "hashSha512": "abc123...",
  "firmaDigital": "-----BEGIN PGP SIGNATURE-----...",
  "estadisticas": {
    "descargas": 0,
    "estrellas": 0,
    "forks": 0
  }
}
```

### Campos Obligatorios

| Campo | Tipo | Descripción |
|-------|------|-------------|
| `nombre` | string | Nombre único del paquete (kebab-case) |
| `version` | string | Versión semántica (X.Y.Z) |
| `descripcion` | string | Descripción breve (máx 140 caracteres) |
| `principal` | string | Ruta al punto de entrada |
| `jasbootVersion` | string | Versión mínima de Jasboot requerida |
| `licencia` | string | Identificador SPDX de licencia |

### Campos Opcionales Importantes

| Campo | Tipo | Descripción |
|-------|------|-------------|
| `descripcionLarga` | string | Descripción detallada |
| `autor` | object | Información del autor |
| `dependencias` | object | Dependencias de runtime |
| `dependenciasDesarrollo` | object | Dependencias de desarrollo |
| `scripts` | object | Scripts ejecutables |
| `palabrasClave` | array | Términos de búsqueda |
| `exportar` | array | Símbolos exportados |
| `jmn` | object | Configuración de memoria neuronal |
| `compilacion` | object | Opciones de compilación |

---

## Versionado Semántico

### Formato
```
MAJOR.MINOR.PATCH[-PRERELEASE][+BUILD]
```

### Reglas
- **MAJOR**: Cambios incompatibles hacia atrás
- **MINOR**: Funcionalidad nueva compatible
- **PATCH**: Correcciones de errores
- **PRERELEASE**: alpha, beta, rc (opcional)
- **BUILD**: Metadatos de build (opcional)

### Ejemplos
```
1.0.0          # Primera versión estable
1.2.3          # Versión con parches
2.0.0-alpha.1  # Versión alpha
1.5.0-beta.2   # Versión beta
1.0.0+build.1  # Con metadatos de build
```

---

## Dependencias

### Rangos de Versiones
```
^1.2.3    # >=1.2.3 <2.0.0 (compatible)
~1.2.3    # >=1.2.3 <1.3.0 (compatible en patch)
>=1.2.3   # 1.2.3 o superior
<=1.2.3   # 1.2.3 o inferior
>1.2.3    # Mayor a 1.2.3
<1.2.3    # Menor a 1.2.3
1.2.3     # Exactamente 1.2.3
*         # Cualquier versión
latest    # Última versión
```

### Resolución de Conflictos
1. **Prioridad de rangos**: Más específico gana
2. **Conflictos directos**: Error de instalación
3. **Dependencias transitivas**: Grafo de resolución
4. **Ciclos**: Detección y error

---

## Instalación

### Estructura de Directorios
```
$HOME/.jpm/
|
+-- paquetes/
|   +-- nombre-paquete/
|   |   +-- 1.0.0/
|   |   |   +-- jasboot.json
|   |   |   +-- src/
|   |   |   +-- docs/
|   |   |   +-- ...
|   |   +-- 1.1.0/
|   |   +-- current -> 1.1.0
|
+-- cache/
|   +-- descargas/
|   +-- temporal/
|
+-- registro/
|   +-- instalados.json
|   +-- lock.json
|
+-- bin/
|   +-- nombre-comando -> ../paquetes/paquete/version/bin/ejecutable
|
+-- logs/
|   +-- install.log
|   +-- error.log
```

### Archivo de Registro
```json
{
  "instalados": {
    "nombre-paquete": {
      "version": "1.2.3",
      "ruta": "$HOME/.jpm/paquetes/nombre-paquete/1.2.3",
      "fechaInstalacion": "2026-04-17T18:00:00Z",
      "instaladoDesde": "https://registry.jasboot.org/nombre-paquete",
      "hashVerificado": "abc123...",
      "dependencias": ["otro-paquete@^1.0.0"]
    }
  },
  "global": {
    "ultimaActualizacion": "2026-04-17T18:00:00Z",
    "versionJpm": "1.0.0",
    "versionJasboot": "1.2.0"
  }
}
```

---

## Cache

### Estrategia de Cache
1. **Descargas**: Archivos .jpkg descargados
2. **Extracciones**: Paquetes extraídos temporalmente
3. **Metadatos**: Información de paquetes remotos
4. **Resolución**: Resultados de resolución de dependencias

### Políticas
- **TTL**: 7 días para metadatos
- **Límite**: 1GB para cache total
- **Limpieza**: LRU automático
- **Verificación**: Hash SHA-512

---

## Seguridad

### Verificación de Integridad
1. **Hash SHA-512**: Obligatorio para todos los paquetes
2. **Firma Digital**: Opcional para paquetes oficiales
3. **Checksum**: Verificación automática al instalar

### Confianza
- **Niveles**: Confiable, Verificado, Desconocido
- **Fuentes**: Registro oficial, URL directa, local
- **Políticas**: Restriciones por configuración

---

## Compatibilidad

### Versiones de Jasboot
- **Mínima**: 1.0.0 (características básicas)
- **Recomendada**: 1.2.0+ (mejoras de JMN)
- **Óptima**: 2.0.0+ (características completas)

### Características Especiales
- **JMN**: Configuración opcional de memoria neuronal
- **Módulos**: Sistema de importación/exportación
- **Clases**: Herencia y encapsulamiento
- **Compilación**: Opciones avanzadas

---

## Ejemplos

### Paquete Básico
```json
{
  "nombre": "hola-mundo",
  "version": "1.0.0",
  "descripcion": "Ejemplo básico de paquete Jasboot",
  "principal": "src/main.jasb",
  "jasbootVersion": ">=1.0.0",
  "licencia": "MIT"
}
```

### Paquete con JMN
```json
{
  "nombre": "memoria-conversacional",
  "version": "2.1.0",
  "descripcion": "Sistema de memoria neuronal para conversaciones",
  "principal": "src/jmn_conversacional.jasb",
  "jasbootVersion": ">=2.0.0",
  "licencia": "Apache-2.0",
  "jmn": {
    "requerido": true,
    "version": ">=2.0.0",
    "configuracion": {
      "memoria": "persistente",
      "cache": "activado",
      "tamano": "100MB"
    }
  },
  "exportar": [
    "inicializar_jmn",
    "recordar_conversacion",
    "recuperar_contexto"
  ]
}
```

### Aplicación Completa
```json
{
  "nombre": "aurora-ia",
  "version": "1.1.0",
  "descripcion": "Asistente conversacional con memoria neuronal",
  "descripcionLarga": "Aurora IA es un asistente conversacional avanzado que utiliza memoria neuronal JMN para mantener contexto y aprender de cada interacción.",
  "autor": {
    "nombre": "Jasboot Team",
    "email": "team@jasboot.org",
    "url": "https://jasboot.org"
  },
  "principal": "src/aurora.jasb",
  "tipo": "aplicacion",
  "jasbootVersion": ">=2.0.0",
  "licencia": "MIT",
  "dependencias": {
    "memoria-conversacional": "^2.0.0",
    "analitica-neuronal": "^1.5.0"
  },
  "scripts": {
    "iniciar": "jbc src/aurora.jasb -o build/aurora.jbo && jasboot-ir-vm build/aurora.jbo",
    "test": "jbc tests/test_*.jasb",
    "limpiar": "rm -rf build/"
  },
  "palabrasClave": ["ia", "conversacional", "jmn", "asistente"],
  "repositorio": {
    "tipo": "git",
    "url": "https://github.com/jasboot/aurora-ia.git"
  },
  "jmn": {
    "requerido": true,
    "version": ">=2.0.0",
    "configuracion": {
      "memoria": "persistente",
      "cache": "activado",
      "tamano": "500MB"
    }
  },
  "compilacion": {
    "optimizacion": "rendimiento",
    "debug": false,
    "advertencias": "estrictas"
  }
}
```

---

## Validación

### Reglas de Validación
1. **Sintaxis JSON**: Válido según RFC 8259
2. **Esquema**: Cumplimiento del esquema JPKG
3. **Nomenclatura**: kebab-case para nombres
4. **Versiones**: Semántico válido
5. **Dependencias**: No ciclos, rangos válidos
6. **Archivos**: Rutas existentes en el paquete
7. **Hash**: SHA-512 coincide con contenido

### Herramientas de Validación
```bash
# Validar paquete local
jpm validate ./mi-paquete.jpkg

# Validar metadatos
jpm validate-metadata jasboot.json

# Verificar integridad
jpm verify ./mi-paquete.jpkg
```

---

## Consideraciones de Diseño

### Principios
1. **Simplicidad**: Fácil de entender y usar
2. **Compatibilidad**: Funciona con versiones existentes
3. **Extensibilidad**: Permite características futuras
4. **Seguridad**: Verificación de integridad por defecto
5. **Rendimiento**: Cache y optimización integrados

### Decisiones Técnicas
- **ZIP**: Formato universal y compresión eficiente
- **JSON**: Legible por humanos y fácil de procesar
- **SemVer**: Estándar de la industria
- **SHA-512**: Criptográficamente seguro
- **Firma PGP**: Estándar de confianza

---

*Esta especificación define el formato .jpkg para paquetes Jasboot, asegurando compatibilidad, seguridad y facilidad de uso para el ecosistema Jasboot.*
