# ESTRUCTURA INTERNA - Jasboot Package Manager (jpm)

## Arquitectura General

`jpm` es un gestor de paquetes modular diseñado para manejar el ecosistema Jasboot con eficiencia y seguridad.

---

## Componentes Principales

### 1. Core Engine
```
jpm/
|
+-- cmd/                    # Interfaz de línea de comandos
|   +-- root.go           # Comando principal y flags globales
|   +-- init.go           # Comando jpm init
|   +-- install.go        # Comando jpm install
|   +-- pack.go           # Comando jpm pack
|   +-- publish.go        # Comando jpm publish
|   +-- list.go           # Comando jpm list
|   +-- info.go           # Comando jpm info
|   +-- search.go         # Comando jpm search
|   +-- update.go         # Comando jpm update
|   +-- uninstall.go      # Comando jpm uninstall
|   +-- login.go          # Comando jpm login
|   +-- logout.go         # Comando jpm logout
|   +-- whoami.go         # Comando jpm whoami
|   +-- audit.go          # Comando jpm audit
|   +-- run.go            # Comando jpm run
|
+-- internal/              # Componentes internos
|   +-- pkg/              # Gestión de paquetes
|   |   +-- package.go    # Estructura de paquete
|   |   +-- metadata.go   # Manejo de metadatos JSON
|   |   +-- validator.go  # Validación de paquetes
|   |   +-- installer.go  # Instalador de paquetes
|   |   +-- uninstaller.go # Desinstalador
|   |   +-- resolver.go   # Resolución de dependencias
|   |   +-- lockfile.go   # Manejo de archivos lock
|   |
|   +-- registry/         # Interacción con registro
|   |   +-- client.go     # Cliente HTTP para registry
|   |   +-- auth.go       # Autenticación
|   |   +-- search.go     # Búsqueda de paquetes
|   |   +-- publish.go    # Publicación de paquetes
|   |   +-- download.go   # Descarga de paquetes
|   |
|   +-- storage/          # Almacenamiento local
|   |   +-- filesystem.go # Operaciones de sistema de archivos
|   |   +-- cache.go      # Gestión de caché
|   |   +-- database.go   # Base de datos local
|   |   +-- paths.go      # Rutas y directorios
|   |
|   +-- config/           # Configuración
|   |   +-- config.go     # Manejo de configuración
|   |   +-- defaults.go   # Valores por defecto
|   |   +-- loader.go     # Cargador de configuración
|   |
|   +-- security/         # Seguridad
|   |   +-- hash.go       # Cálculo de hashes SHA-512
|   |   +-- signature.go  # Verificación de firmas PGP
|   |   +-- trust.go      # Gestión de confianza
|   |   +-- audit.go      # Auditoría de seguridad
|   |
|   +-- utils/            # Utilidades
|   |   +-- version.go    # Manejo de versiones semánticas
|   |   +-- network.go    # Operaciones de red
|   |   +-- compression.go # Compresión/descompresión
|   |   +-- logger.go     # Sistema de logging
|   |   +-- progress.go   # Barras de progreso
|   |   +-- spinner.go    # Indicadores de actividad
|
+-- pkg/                   # API pública (para librerías)
|   +-- jpm/             # Librería para integración
|   |   +-- client.go    # Cliente programático
|   |   +-- types.go     # Tipos públicos
|   |   +-- errors.go    # Errores definidos
|
+-- test/                  # Pruebas
|   +-- integration/     # Tests de integración
|   +-- unit/           # Tests unitarios
|   +-- fixtures/       # Datos de prueba
|
+-- docs/                  # Documentación
|   +-- api.md          # Documentación API
|   +-- cli.md          # Guía de comandos
|   +-- examples/       # Ejemplos de uso
|
+-- scripts/               # Scripts de build
|   +-- build.sh        # Build para Linux/macOS
|   +-- build.bat       # Build para Windows
|   +-- release.sh      # Script de release
|
+-- go.mod                 # Módulo Go
+-- go.sum                 # Checksums de dependencias
+-- Makefile               # Build y tareas
+-- README.md              # Documentación principal
+-- LICENSE                # Licencia
```

---

## Flujo de Operaciones

### 1. Inicialización (`jpm init`)
```go
func InitProject(name, version, description string) error {
    // 1. Validar nombre del paquete
    if err := validatePackageName(name); err != nil {
        return err
    }
    
    // 2. Crear estructura de directorios
    dirs := []string{"src", "docs", "tests", "bin", "recursos"}
    for _, dir := range dirs {
        if err := os.MkdirAll(dir, 0755); err != nil {
            return err
        }
    }
    
    // 3. Generar jasboot.json
    metadata := &PackageMetadata{
        Name:        name,
        Version:     version,
        Description: description,
        Main:        "src/main.jasb",
        JasbootVersion: ">=1.0.0",
        License:     "MIT",
    }
    
    // 4. Crear archivos plantilla
    createTemplateFiles(metadata)
    
    // 5. Inicializar repositorio git (opcional)
    if shouldInitGit() {
        runGitInit()
    }
    
    return nil
}
```

### 2. Empaquetado (`jpm pack`)
```go
func PackPackage(sourceDir, outputDir string) error {
    // 1. Leer metadatos
    metadata, err := LoadMetadata(filepath.Join(sourceDir, "jasboot.json"))
    if err != nil {
        return err
    }
    
    // 2. Validar estructura
    if err := validatePackageStructure(sourceDir, metadata); err != nil {
        return err
    }
    
    // 3. Crear archivo ZIP temporal
    tempFile, err := createTempZip()
    if err != nil {
        return err
    }
    defer os.Remove(tempFile.Name())
    
    // 4. Agregar archivos al ZIP
    if err := addFilesToZip(tempFile, sourceDir, metadata); err != nil {
        return err
    }
    
    // 5. Calcular hash SHA-512
    hash, err := calculateSHA512(tempFile)
    if err != nil {
        return err
    }
    
    // 6. Firmar si es necesario
    if shouldSign() {
        signature, err := signPackage(tempFile)
        if err != nil {
            return err
        }
        metadata.Signature = signature
    }
    
    // 7. Actualizar metadatos con hash
    metadata.HashSHA512 = hash
    
    // 8. Renombrar a .jpkg final
    finalName := fmt.Sprintf("%s-%s.jpkg", metadata.Name, metadata.Version)
    finalPath := filepath.Join(outputDir, finalName)
    
    return os.Rename(tempFile.Name(), finalPath)
}
```

### 3. Instalación (`jpm install`)
```go
func InstallPackage(source string, global bool) error {
    // 1. Determinar tipo de fuente
    pkgSource, err := parsePackageSource(source)
    if err != nil {
        return err
    }
    
    // 2. Descargar paquete si es remoto
    localPath, err := downloadPackageIfNeeded(pkgSource)
    if err != nil {
        return err
    }
    
    // 3. Validar paquete
    if err := validatePackage(localPath); err != nil {
        return err
    }
    
    // 4. Extraer metadatos
    metadata, err := extractMetadata(localPath)
    if err != nil {
        return err
    }
    
    // 5. Resolver dependencias
    deps, err := resolveDependencies(metadata)
    if err != nil {
        return err
    }
    
    // 6. Instalar dependencias primero
    for _, dep := range deps {
        if err := InstallPackage(dep.String(), global); err != nil {
            return err
        }
    }
    
    // 7. Extraer paquete
    installPath, err := extractPackage(localPath, metadata, global)
    if err != nil {
        return err
    }
    
    // 8. Actualizar registro
    if err := updateRegistry(metadata, installPath); err != nil {
        return err
    }
    
    // 9. Crear enlaces simbólicos en bin/
    if metadata.Bin != nil {
        createBinSymlinks(metadata, installPath)
    }
    
    // 10. Actualizar lockfile
    return updateLockfile(metadata)
}
```

### 4. Resolución de Dependencias
```go
type DependencyResolver struct {
    registry    *RegistryClient
    localDB     *LocalDatabase
    cache       *Cache
}

func (dr *DependencyResolver) Resolve(metadata *PackageMetadata) ([]*PackageDependency, error) {
    // 1. Construir grafo de dependencias
    graph := NewDependencyGraph()
    
    // 2. Agregar paquete raíz
    if err := graph.AddPackage(metadata); err != nil {
        return nil, err
    }
    
    // 3. Agregar dependencias transitivas
    for name, constraint := range metadata.Dependencies {
        if err := dr.resolveDependency(graph, name, constraint); err != nil {
            return nil, err
        }
    }
    
    // 4. Detectar ciclos
    if cycles := graph.DetectCycles(); len(cycles) > 0 {
        return nil, fmt.Errorf("ciclos de dependencias detectados: %v", cycles)
    }
    
    // 5. Resolver conflictos de versiones
    resolved, err := graph.ResolveConflicts()
    if err != nil {
        return nil, err
    }
    
    // 6. Ordenar topológicamente
    return graph.TopologicalSort(resolved)
}

func (dr *DependencyResolver) resolveDependency(graph *DependencyGraph, name, constraint string) error {
    // 1. Buscar en registro remoto
    versions, err := dr.registry.GetPackageVersions(name)
    if err != nil {
        return err
    }
    
    // 2. Filtrar por constraint
    validVersions := filterVersions(versions, constraint)
    if len(validVersions) == 0 {
        return fmt.Errorf("no hay versión compatible para %s@%s", name, constraint)
    }
    
    // 3. Seleccionar versión más reciente
    latest := selectLatestVersion(validVersions)
    
    // 4. Obtener metadatos de la versión
    metadata, err := dr.registry.GetPackageMetadata(name, latest)
    if err != nil {
        return err
    }
    
    // 5. Agregar al grafo
    return graph.AddPackage(metadata)
}
```

---

## Estructuras de Datos

### 1. Paquete
```go
type Package struct {
    Metadata    *PackageMetadata
    Path        string
    Size        int64
    Hash        string
    Signature   string
    InstalledAt time.Time
    Source      PackageSource
}

type PackageSource struct {
    Type     string // "registry", "url", "local", "git"
    Location string
    Version  string
    Hash     string
}
```

### 2. Metadatos
```go
type PackageMetadata struct {
    Name                 string            `json:"nombre"`
    Version              string            `json:"version"`
    Description          string            `json:"descripcion"`
    DescriptionLong      string            `json:"descripcionLarga,omitempty"`
    Author               *Author           `json:"autor,omitempty"`
    License              string            `json:"licencia"`
    Main                 string            `json:"principal"`
    Type                 string            `json:"tipo,omitempty"`
    JasbootVersion       string            `json:"jasbootVersion"`
    Dependencies         map[string]string `json:"dependencias,omitempty"`
    DevDependencies      map[string]string `json:"dependenciasDesarrollo,omitempty"`
    Scripts              map[string]string `json:"scripts,omitempty"`
    Keywords             []string          `json:"palabrasClave,omitempty"`
    Repository           *Repository       `json:"repositorio,omitempty"`
    Bugs                 *Bugs             `json:"bugs,omitempty"`
    Homepage             string            `json:"pagina,omitempty"`
    Files                []string          `json:"archivos,omitempty"`
    Ignore               []string          `json:"ignorar,omitempty"`
    Bin                  map[string]string `json:"bin,omitempty"`
    Export               []string          `json:"exportar,omitempty"`
    JMN                  *JMNConfig        `json:"jmn,omitempty"`
    Compilation          *CompilationConfig `json:"compilacion,omitempty"`
    Platforms            []string          `json:"plataformas,omitempty"`
    CreatedAt            time.Time         `json:"fechaCreacion,omitempty"`
    PublishedAt          time.Time         `json:"fechaPublicacion,omitempty"`
    HashSHA512           string            `json:"hashSha512,omitempty"`
    Signature            string            `json:"firmaDigital,omitempty"`
    Statistics           *Statistics       `json:"estadisticas,omitempty"`
}
```

### 3. Configuración
```go
type Config struct {
    RegistryURL     string        `yaml:"registry_url"`
    CacheDir        string        `yaml:"cache_dir"`
    InstallDir      string        `yaml:"install_dir"`
    GlobalInstall   bool          `yaml:"global_install"`
    AutoUpdate      bool          `yaml:"auto_update"`
    LogLevel        string        `yaml:"log_level"`
    Timeout         time.Duration `yaml:"timeout"`
    MaxConcurrent   int           `yaml:"max_concurrent"`
    ProxyURL        string        `yaml:"proxy_url,omitempty"`
    NoSSLVerify    bool          `yaml:"no_ssl_verify"`
    TrustStore      string        `yaml:"trust_store,omitempty"`
    SigningKey      string        `yaml:"signing_key,omitempty"`
}
```

### 4. Cache
```go
type Cache struct {
    Dir         string
    MaxSize     int64
    CurrentSize int64
    TTL         time.Duration
    mutex       sync.RWMutex
}

type CacheEntry struct {
    Key        string
    Path       string
    Size       int64
    CreatedAt  time.Time
    AccessedAt time.Time
    ExpiresAt  time.Time
    Hash       string
}
```

---

## Gestión de Estado

### 1. Base de Datos Local
```go
type LocalDatabase struct {
    dbPath string
    mutex  sync.RWMutex
}

type InstalledPackage struct {
    Name           string    `json:"name"`
    Version        string    `json:"version"`
    Path           string    `json:"path"`
    InstalledAt    time.Time `json:"installed_at"`
    InstalledFrom  string    `json:"installed_from"`
    HashVerified   string    `json:"hash_verified"`
    Dependencies   []string  `json:"dependencies"`
    Type           string    `json:"type"`
}

type Lockfile struct {
    Version    string                    `json:"version"`
    Packages   map[string]LockfileEntry `json:"packages"`
    Hash       string                    `json:"hash"`
    Generated  time.Time                 `json:"generated"`
    JPMVersion string                    `json:"jpm_version"`
}

type LockfileEntry struct {
    Version     string   `json:"version"`
    Resolved    string   `json:"resolved"`
    Hash        string   `json:"hash"`
    Dependencies []string `json:"dependencies"`
}
```

### 2. Registro de Operaciones
```go
type OperationLog struct {
    Timestamp time.Time `json:"timestamp"`
    Operation string    `json:"operation"`
    Package   string    `json:"package"`
    Version   string    `json:"version"`
    Status    string    `json:"status"`
    Error     string    `json:"error,omitempty"`
    Duration  int64     `json:"duration_ms"`
    User      string    `json:"user,omitempty"`
}
```

---

## Manejo de Errores

### Jerarquía de Errores
```go
type JPMError struct {
    Code    string `json:"code"`
    Message string `json:"message"`
    Details string `json:"details,omitempty"`
    Cause   error  `json:"-"`
}

const (
    ErrCodeInvalidPackage     = "INVALID_PACKAGE"
    ErrCodeDependencyConflict = "DEPENDENCY_CONFLICT"
    ErrCodeNetworkError       = "NETWORK_ERROR"
    ErrCodePermissionDenied   = "PERMISSION_DENIED"
    ErrCodeCorruptPackage     = "CORRUPT_PACKAGE"
    ErrCodeVersionMismatch    = "VERSION_MISMATCH"
    ErrCodeCircularDependency = "CIRCULAR_DEPENDENCY"
    ErrCodeRegistryError      = "REGISTRY_ERROR"
    ErrCodeAuthFailed        = "AUTH_FAILED"
    ErrCodeDiskSpace         = "DISK_SPACE"
    ErrCodeChecksumMismatch  = "CHECKSUM_MISMATCH"
)
```

### Manejo de Recuperación
```go
type RecoveryManager struct {
    tempDir    string
    backupDir  string
    maxRetries int
}

func (rm *RecoveryManager) RecoverFromFailedInstall(pkgName string) error {
    // 1. Limpiar instalación parcial
    if err := rm.cleanupPartialInstall(pkgName); err != nil {
        return err
    }
    
    // 2. Restaurar desde backup si existe
    if backup, err := rm.findBackup(pkgName); err == nil {
        return rm.restoreFromBackup(backup)
    }
    
    // 3. Revertir cambios en registro
    return rm.revertRegistryChanges(pkgName)
}
```

---

## Concurrencia y Paralelismo

### Instalación Paralela
```go
type ParallelInstaller struct {
    semaphore chan struct{}
    wg        sync.WaitGroup
    errors    chan error
    maxWorkers int
}

func (pi *ParallelInstaller) InstallPackages(packages []string) error {
    // 1. Crear semáforo para limitar concurrencia
    pi.semaphore = make(chan struct{}, pi.maxWorkers)
    
    // 2. Resolver dependencias en orden
    sorted, err := pi.resolveDependencyOrder(packages)
    if err != nil {
        return err
    }
    
    // 3. Instalar en paralelo cuando sea posible
    for _, pkg := range sorted {
        pi.wg.Add(1)
        go pi.installPackageAsync(pkg)
    }
    
    // 4. Esperar completion
    pi.wg.Wait()
    close(pi.errors)
    
    // 5. Recolectar errores
    var errors []error
    for err := range pi.errors {
        errors = append(errors, err)
    }
    
    if len(errors) > 0 {
        return fmt.Errorf("errores en instalación: %v", errors)
    }
    
    return nil
}
```

---

## Extensibilidad

### Plugin System
```go
type Plugin interface {
    Name() string
    Version() string
    Init(config map[string]interface{}) error
    Execute(ctx *PluginContext) error
    Cleanup() error
}

type PluginManager struct {
    plugins map[string]Plugin
    hooks   map[string][]PluginHook
}

type PluginHook interface {
    BeforeInstall(pkg *Package) error
    AfterInstall(pkg *Package) error
    BeforeUninstall(pkg *Package) error
    AfterUninstall(pkg *Package) error
    BeforePublish(pkg *Package) error
    AfterPublish(pkg *Package) error
}
```

### Hooks del Sistema
```go
type SystemHooks struct {
    PreInstall    []func(*Package) error
    PostInstall   []func(*Package) error
    PreUninstall  []func(*Package) error
    PostUninstall []func(*Package) error
    PrePublish    []func(*Package) error
    PostPublish   []func(*Package) error
    PreBuild      []func(*Package) error
    PostBuild     []func(*Package) error
}
```

---

## Monitoreo y Métricas

### Métricas Internas
```go
type Metrics struct {
    PackagesInstalled    int64     `json:"packages_installed"`
    PackagesUninstalled  int64     `json:"packages_uninstalled"`
    DownloadsTotal       int64     `json:"downloads_total"`
    CacheHits           int64     `json:"cache_hits"`
    CacheMisses         int64     `json:"cache_misses"`
    AverageInstallTime  time.Duration `json:"average_install_time"`
    ErrorRate           float64   `json:"error_rate"`
    LastUpdate          time.Time `json:"last_update"`
}

type MetricsCollector struct {
    metrics *Metrics
    mutex   sync.RWMutex
    storage MetricsStorage
}
```

### Health Checks
```go
type HealthChecker struct {
    checks []HealthCheck
}

type HealthCheck interface {
    Name() string
    Check() error
    Timeout() time.Duration
}

func (hc *HealthChecker) RunChecks() map[string]error {
    results := make(map[string]error)
    
    for _, check := range hc.checks {
        ctx, cancel := context.WithTimeout(context.Background(), check.Timeout())
        err := check.Check()
        cancel()
        results[check.Name()] = err
    }
    
    return results
}
```

---

## Consideraciones de Rendimiento

### Optimizaciones
1. **Cache Inteligente**: LRU con TTL adaptativo
2. **Descargas Paralelas**: Múltiples conexiones HTTP
3. **Compresión Delta**: Solo descargar cambios
4. **Índices Locales**: Búsqueda rápida en metadatos
5. **Lazy Loading**: Cargar información bajo demanda

### Perfil de Memoria
```go
type MemoryProfile struct {
    CacheUsage    int64 `json:"cache_usage"`
    DatabaseSize  int64 `json:"database_size"`
    TempFiles     int64 `json:"temp_files"`
    NetworkBuffers int64 `json:"network_buffers"`
    PeakMemory    int64 `json:"peak_memory"`
}
```

---

*Esta estructura interna define la arquitectura completa del gestor de paquetes Jasboot, asegurando modularidad, escalabilidad y mantenibilidad.*
