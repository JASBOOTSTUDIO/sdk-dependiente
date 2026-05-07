# SISTEMA DE CACHE - Jasboot Package Manager

## Arquitectura del Cache

El sistema de cache de JPM está diseñado para optimizar descargas, reducir tráfico de red y acelerar operaciones repetitivas.

---

## Componentes del Cache

### 1. Estructura de Directorios
```
$HOME/.jpm/cache/
|
+-- descargas/                 # Paquetes .jpkg descargados
|   +-- abc123def456...        # Hash SHA-512 como nombre
|   +-- 789ghi012jkl...
|   +-- metadata/
|   |   +-- abc123def456.json  # Metadatos del archivo
|   |   +-- 789ghi012jkl.json
|
+-- temporal/                   # Archivos temporales
|   +-- download_12345.jpkg.tmp
|   +-- extract_67890/
|   +-- build_abcde/
|
+-- metadatos/                  # Cache de metadatos
|   +-- registry/
|   |   +-- packages_index.json
|   |   +-- package_info.json
|   |   +-- versions_list.json
|   +-- search/
|   |   +-- keywords_index.json
|   |   +-- trends.json
|   +-- dependencies/
|   |   +-- resolved_graphs.cache
|   |   +-- version_locks.cache
|
+-- build/                      # Cache de compilaciones
|   +-- compiled/
|   |   +-- nombre-paquete-1.0.0.jbo
|   |   +-- otro-paquete-2.1.0.jbo
|   +-- artifacts/
|   |   +-- build_logs/
|   |   +-- test_results/
|
+-- http/                       # Cache HTTP
|   +-- responses/
|   |   +-- api_packages_get.json
|   |   +-- api_search_query.json
|   +-- etags/
|   |   +-- package_abc123.etag
|
+-- lru/                        # Índices LRU
|   +-- access_order.json
|   +-- timestamps.json
|   +-- sizes.json
```

---

## Tipos de Cache

### 1. Cache de Descargas
```go
type DownloadCache struct {
    Dir        string
    MaxSize    int64
    MaxFiles   int
    TTL        time.Duration
    mutex      sync.RWMutex
    lru        *LRUCache
}

type CacheEntry struct {
    Key        string    // Hash SHA-512 del contenido
    Path       string    // Ruta al archivo cacheado
    Size       int64     // Tamaño en bytes
    CreatedAt  time.Time // Fecha de creación
    AccessedAt time.Time // Último acceso
    ExpiresAt  time.Time // Fecha de expiración
    Hash       string    // Verificación de integridad
    Source     string    // URL o fuente original
    Metadata   *CacheMetadata
}

type CacheMetadata struct {
    ContentType   string            `json:"content_type"`
    ETag          string            `json:"etag"`
    LastModified  time.Time         `json:"last_modified"`
    Headers       map[string]string `json:"headers"`
    DownloadTime  time.Duration     `json:"download_time"`
    RetryCount    int               `json:"retry_count"`
}
```

### 2. Cache de Metadatos
```go
type MetadataCache struct {
    Dir       string
    TTL       time.Duration
    mutex     sync.RWMutex
    index     map[string]*MetadataEntry
}

type MetadataEntry struct {
    Package    string                 `json:"package"`
    Version    string                 `json:"version"`
    Data       map[string]interface{} `json:"data"`
    CachedAt   time.Time              `json:"cached_at"`
    ExpiresAt  time.Time              `json:"expires_at"`
    Source     string                 `json:"source"`
    Hash       string                 `json:"hash"`
}

// Métodos principales
func (mc *MetadataCache) Get(key string) (interface{}, bool)
func (mc *MetadataCache) Set(key string, data interface{}, ttl time.Duration)
func (mc *MetadataCache) Invalidate(key string)
func (mc *MetadataCache) InvalidatePattern(pattern string)
func (mc *MetadataCache) Cleanup() error
```

### 3. Cache de Resolución de Dependencias
```go
type DependencyCache struct {
    Dir     string
    mutex   sync.RWMutex
    graphs  map[string]*DependencyGraph
}

type DependencyGraph struct {
    Name       string                    `json:"name"`
    Version    string                    `json:"version"`
    Hash       string                    `json:"hash"`
    Graph      map[string][]string       `json:"graph"`
    Resolved   map[string]string         `json:"resolved"`
    CreatedAt  time.Time                 `json:"created_at"`
    ExpiresAt  time.Time                 `json:"expires_at"`
}

func (dc *DependencyCache) GetGraph(pkgName, pkgVersion string) (*DependencyGraph, bool)
func (dc *DependencyCache) SetGraph(pkgName, pkgVersion string, graph *DependencyGraph)
func (dc *DependencyCache) InvalidatePackage(pkgName string)
```

---

## Algoritmos de Cache

### 1. LRU (Least Recently Used)
```go
type LRUCache struct {
    capacity int
    size     int
    head     *Node
    tail     *Node
    items    map[string]*Node
    mutex    sync.RWMutex
}

type Node struct {
    key   string
    value interface{}
    prev  *Node
    next  *Node
    size  int64
    time  time.Time
}

func (lru *LRUCache) Get(key string) (interface{}, bool) {
    lru.mutex.Lock()
    defer lru.mutex.Unlock()
    
    if node, exists := lru.items[key]; exists {
        lru.moveToHead(node)
        node.time = time.Now()
        return node.value, true
    }
    
    return nil, false
}

func (lru *LRUCache) Put(key string, value interface{}, size int64) {
    lru.mutex.Lock()
    defer lru.mutex.Unlock()
    
    if node, exists := lru.items[key]; exists {
        node.value = value
        node.size = size
        node.time = time.Now()
        lru.moveToHead(node)
        return
    }
    
    node := &Node{
        key:   key,
        value: value,
        size:  size,
        time:  time.Now(),
    }
    
    lru.items[key] = node
    lru.addToHead(node)
    lru.size += size
    
    for lru.size > lru.capacity {
        lru.removeTail()
    }
}
```

### 2. TTL (Time To Live)
```go
type TTLCache struct {
    items map[string]*TTLItem
    mutex sync.RWMutex
    ticker *time.Ticker
    done   chan bool
}

type TTLItem struct {
    value     interface{}
    expiresAt time.Time
    createdAt time.Time
}

func (ttl *TTLCache) Set(key string, value interface{}, duration time.Duration) {
    ttl.mutex.Lock()
    defer ttl.mutex.Unlock()
    
    ttl.items[key] = &TTLItem{
        value:     value,
        expiresAt: time.Now().Add(duration),
        createdAt: time.Now(),
    }
}

func (ttl *TTLCache) Get(key string) (interface{}, bool) {
    ttl.mutex.RLock()
    defer ttl.mutex.RUnlock()
    
    if item, exists := ttl.items[key]; exists {
        if time.Now().Before(item.expiresAt) {
            return item.value, true
        }
        delete(ttl.items, key)
    }
    
    return nil, false
}

func (ttl *TTLCache) cleanup() {
    ticker := time.NewTicker(1 * time.Minute)
    defer ticker.Stop()
    
    for {
        select {
        case <-ticker.C:
            ttl.mutex.Lock()
            now := time.Now()
            for key, item := range ttl.items {
                if now.After(item.expiresAt) {
                    delete(ttl.items, key)
                }
            }
            ttl.mutex.Unlock()
        case <-ttl.done:
            return
        }
    }
}
```

---

## Estrategias de Cache

### 1. Cache-Aside Pattern
```go
type CacheAside struct {
    cache    Cache
    backend  Backend
    missRate float64
}

func (ca *CacheAside) Get(key string) (interface{}, error) {
    // 1. Intentar obtener del cache
    if value, found := ca.cache.Get(key); found {
        return value, nil
    }
    
    // 2. Cache miss: obtener del backend
    value, err := ca.backend.Get(key)
    if err != nil {
        return nil, err
    }
    
    // 3. Almacenar en cache
    ca.cache.Set(key, value, 30*time.Minute)
    
    return value, nil
}

func (ca *CacheAside) Invalidate(key string) {
    ca.cache.Delete(key)
}
```

### 2. Write-Through Cache
```go
type WriteThroughCache struct {
    cache   Cache
    backend Backend
}

func (wt *WriteThroughCache) Set(key string, value interface{}) error {
    // 1. Escribir en backend primero
    if err := wt.backend.Set(key, value); err != nil {
        return err
    }
    
    // 2. Actualizar cache
    wt.cache.Set(key, value, 1*time.Hour)
    
    return nil
}
```

### 3. Write-Behind Cache
```go
type WriteBehindCache struct {
    cache    Cache
    backend  Backend
    queue    chan WriteOperation
    workers  int
    done     chan bool
}

type WriteOperation struct {
    Key   string
    Value interface{}
    Time  time.Time
}

func (wb *WriteBehindCache) Set(key string, value interface{}) {
    // 1. Actualizar cache inmediatamente
    wb.cache.Set(key, value, 1*time.Hour)
    
    // 2. Encolar para escritura asíncrona
    select {
    case wb.queue <- WriteOperation{Key: key, Value: value, Time: time.Now()}:
    default:
        // Queue llena, escribir sincrónicamente
        wb.backend.Set(key, value)
    }
}

func (wb *WriteBehindCache) worker() {
    for {
        select {
        case op := <-wb.queue:
            wb.backend.Set(op.Key, op.Value)
        case <-wb.done:
            return
        }
    }
}
```

---

## Optimizaciones de Rendimiento

### 1. Compresión de Cache
```go
type CompressedCache struct {
    cache Cache
    compressor Compressor
    threshold  int64
}

func (cc *CompressedCache) Set(key string, value interface{}, size int64) {
    if size > cc.threshold {
        // Comprimir antes de almacenar
        compressed := cc.compressor.Compress(value)
        cc.cache.Set(key, compressed, len(compressed))
    } else {
        cc.cache.Set(key, value, size)
    }
}

func (cc *CompressedCache) Get(key string) (interface{}, bool) {
    if value, found := cc.cache.Get(key); found {
        if cc.isCompressed(value) {
            return cc.compressor.Decompress(value), true
        }
        return value, true
    }
    return nil, false
}
```

### 2. Cache Particionado
```go
type PartitionedCache struct {
    partitions []*CachePartition
    hash       func(string) int
}

type CachePartition struct {
    cache Cache
    mutex sync.RWMutex
}

func (pc *PartitionedCache) Get(key string) (interface{}, bool) {
    partition := pc.partitions[pc.hash(key)%len(pc.partitions)]
    return partition.cache.Get(key)
}

func (pc *PartitionedCache) Set(key string, value interface{}, size int64) {
    partition := pc.partitions[pc.hash(key)%len(pc.partitions)]
    partition.cache.Set(key, value, size)
}
```

### 3. Cache Predictivo
```go
type PredictiveCache struct {
    cache    Cache
    predictor *AccessPredictor
    prefetch chan string
}

type AccessPredictor struct {
    patterns map[string][]string
    mutex    sync.RWMutex
}

func (pc *PredictiveCache) Get(key string) (interface{}, error) {
    // 1. Obtener del cache
    if value, found := pc.cache.Get(key); found {
        // 2. Predecir próximos accesos
        if predictions := pc.predictor.Predict(key); len(predictions) > 0 {
            for _, nextKey := range predictions {
                select {
                case pc.prefetch <- nextKey:
                default:
                    // Canal lleno, ignorar
                }
            }
        }
        return value, nil
    }
    
    return nil, fmt.Errorf("cache miss")
}

func (pc *PredictiveCache) prefetchWorker() {
    for key := range pc.prefetch {
        if _, found := pc.cache.Get(key); !found {
            // Cargar en background
            go pc.loadInBackground(key)
        }
    }
}
```

---

## Persistencia y Recuperación

### 1. Serialización del Cache
```go
type CacheSerializer struct {
    format string // "json", "gob", "msgpack"
}

func (cs *CacheSerializer) Serialize(cache *LRUCache) ([]byte, error) {
    data := make(map[string]interface{})
    
    // Recorrer todos los items
    current := cache.head
    for current != nil {
        data[current.key] = CacheItem{
            Value: current.value,
            Size:  current.size,
            Time:  current.time,
        }
        current = current.next
    }
    
    switch cs.format {
    case "json":
        return json.Marshal(data)
    case "gob":
        var buf bytes.Buffer
        encoder := gob.NewEncoder(&buf)
        err := encoder.Encode(data)
        return buf.Bytes(), err
    case "msgpack":
        return msgpack.Marshal(data)
    default:
        return nil, fmt.Errorf("formato no soportado: %s", cs.format)
    }
}

func (cs *CacheSerializer) Deserialize(data []byte, cache *LRUCache) error {
    var items map[string]CacheItem
    
    switch cs.format {
    case "json":
        err := json.Unmarshal(data, &items)
        if err != nil {
            return err
        }
    case "gob":
        decoder := gob.NewDecoder(bytes.NewReader(data))
        err := decoder.Decode(&items)
        if err != nil {
            return err
        }
    default:
        return fmt.Errorf("formato no soportado: %s", cs.format)
    }
    
    // Restaurar items
    for key, item := range items {
        cache.Put(key, item.Value, item.Size)
    }
    
    return nil
}
```

### 2. Checkpointing
```go
type CheckpointManager struct {
    cache     Cache
    interval  time.Duration
    dir       string
    maxFiles  int
    ticker    *time.Ticker
    done      chan bool
}

func (cm *CheckpointManager) Start() {
    cm.ticker = time.NewTicker(cm.interval)
    
    go func() {
        for {
            select {
            case <-cm.ticker.C:
                cm.createCheckpoint()
            case <-cm.done:
                return
            }
        }
    }()
}

func (cm *CheckpointManager) createCheckpoint() error {
    timestamp := time.Now().Format("2006-01-02-15-04-05")
    filename := fmt.Sprintf("cache_checkpoint_%s.dat", timestamp)
    filepath := path.Join(cm.dir, filename)
    
    // Serializar cache
    serializer := &CacheSerializer{format: "gob"}
    data, err := serializer.Serialize(cm.cache.(*LRUCache))
    if err != nil {
        return err
    }
    
    // Escribir a archivo
    return ioutil.WriteFile(filepath, data, 0644)
}

func (cm *CheckpointManager) Restore(latest bool) error {
    files, err := filepath.Glob(path.Join(cm.dir, "cache_checkpoint_*.dat"))
    if err != nil {
        return err
    }
    
    if len(files) == 0 {
        return fmt.Errorf("no hay checkpoints disponibles")
    }
    
    // Seleccionar archivo más reciente
    var targetFile string
    if latest {
        targetFile = cm.getLatestFile(files)
    } else {
        targetFile = files[0]
    }
    
    // Cargar y restaurar
    data, err := ioutil.ReadFile(targetFile)
    if err != nil {
        return err
    }
    
    serializer := &CacheSerializer{format: "gob"}
    return serializer.Deserialize(data, cm.cache.(*LRUCache))
}
```

---

## Monitoreo y Métricas

### 1. Estadísticas del Cache
```go
type CacheMetrics struct {
    Hits        int64     `json:"hits"`
    Misses      int64     `json:"misses"`
    HitRate     float64   `json:"hit_rate"`
    Evictions   int64     `json:"evictions"`
    Size        int64     `json:"size"`
    MaxSize     int64     `json:"max_size"`
    Items       int64     `json:"items"`
    AvgItemSize float64   `json:"avg_item_size"`
    LastAccess  time.Time `json:"last_access"`
    CreatedAt   time.Time `json:"created_at"`
}

type MetricsCollector struct {
    cache   Cache
    metrics *CacheMetrics
    mutex   sync.RWMutex
    ticker  *time.Ticker
}

func (mc *MetricsCollector) Collect() *CacheMetrics {
    mc.mutex.RLock()
    defer mc.mutex.RUnlock()
    
    total := mc.metrics.Hits + mc.metrics.Misses
    if total > 0 {
        mc.metrics.HitRate = float64(mc.metrics.Hits) / float64(total)
    }
    
    if mc.metrics.Items > 0 {
        mc.metrics.AvgItemSize = float64(mc.metrics.Size) / float64(mc.metrics.Items)
    }
    
    return mc.metrics
}
```

### 2. Health Checks
```go
type CacheHealthChecker struct {
    cache Cache
}

func (chc *CacheHealthChecker) Check() error {
    // Verificar tamaño
    if chc.cache.Size() > chc.cache.MaxSize() {
        return fmt.Errorf("cache excede tamaño máximo")
    }
    
    // Verificar integridad
    if err := chc.verifyIntegrity(); err != nil {
        return err
    }
    
    // Verificar rendimiento
    if err := chc.benchmarkPerformance(); err != nil {
        return err
    }
    
    return nil
}

func (chc *CacheHealthChecker) verifyIntegrity() error {
    // Implementar verificación de checksums
    return nil
}

func (chc *CacheHealthChecker) benchmarkPerformance() error {
    start := time.Now()
    
    // Operación de prueba
    testKey := "health_check_test"
    testValue := "test_data"
    
    chc.cache.Set(testKey, testValue, 1024)
    _, found := chc.cache.Get(testKey)
    
    duration := time.Since(start)
    
    if duration > 100*time.Millisecond {
        return fmt.Errorf("rendimiento del cache lento: %v", duration)
    }
    
    if !found {
        return fmt.Errorf("cache no funciona correctamente")
    }
    
    return nil
}
```

---

## Configuración y Tuning

### 1. Configuración por Defecto
```json
{
  "cache": {
    "download": {
      "max_size": "1GB",
      "max_files": 1000,
      "ttl": "7d",
      "compression": true,
      "compression_threshold": "1MB"
    },
    "metadata": {
      "max_size": "100MB",
      "ttl": "1h",
      "refresh_interval": "30m"
    },
    "dependencies": {
      "max_size": "50MB",
      "ttl": "24h"
    },
    "build": {
      "max_size": "500MB",
      "ttl": "7d"
    },
    "http": {
      "max_size": "50MB",
      "ttl": "15m",
      "respect_etags": true
    }
  },
  "performance": {
    "partitions": 16,
    "workers": 4,
    "prefetch_enabled": true,
    "predictive_cache": true,
    "checkpoint_interval": "1h",
    "cleanup_interval": "30m"
  },
  "persistence": {
    "serialize_format": "gob",
    "compression": true,
    "encryption": false,
    "max_checkpoints": 10
  }
}
```

### 2. Auto-Tuning
```go
type AutoTuner struct {
    cache      Cache
    metrics    *MetricsCollector
    config     *CacheConfig
    interval   time.Duration
}

func (at *AutoTuner) Tune() {
    metrics := at.metrics.Collect()
    
    // Ajustar tamaño basado en hit rate
    if metrics.HitRate < 0.8 && metrics.Size < at.config.MaxSize {
        at.increaseSize()
    } else if metrics.HitRate > 0.95 && metrics.Size > at.config.MinSize {
        at.decreaseSize()
    }
    
    // Ajustar TTL basado en access patterns
    at.adjustTTL(metrics)
    
    // Ajustar particiones basado en contention
    at.adjustPartitions(metrics)
}

func (at *AutoTuner) increaseSize() {
    newSize := at.cache.MaxSize() * 1.2
    at.cache.SetMaxSize(newSize)
}

func (at *AutoTuner) adjustTTL(metrics *CacheMetrics) {
    // Implementar lógica de ajuste de TTL
}
```

---

*Este sistema de cache proporciona una base robusta y optimizada para el gestor de paquetes Jasboot, asegurando alto rendimiento y uso eficiente de recursos.*
