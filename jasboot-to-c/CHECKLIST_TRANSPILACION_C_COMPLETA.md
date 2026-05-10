# Checklist: Jasboot completo y estable en transpilación a C

**Estado del SDK (última revisión del checklist):** cubiertas de forma sólida **Fases 0–3** y gran parte de la **Fase 4**. **Fusión `usar`** (`aot_usar_merge.c`) y **OOP básico** (clases, métodos, `este`, despacho estático, inferencia de tipo de receptor local para métodos) están en uso. **JMN en AOT** existe como **runtime ligero** en `jasboot_rt` (mapa + relaciones + archivo texto), alineado en lo probado con la VM para `buscar`/`recordar`/`define_concepto`, **no** el binario JMN completo del VM. **`nativo_mlp_entrenar`** tiene paridad VM↔AOT vía `jb_nativo_mlp_entrenar`. **Modo estricto** (`--strict` en `jbc-to-c`) y **tabla de compatibilidad** + guía de porting en README están cubiertos. Siguen abiertas: **cobertura total de `sistema_*`**, **lambdas**, **CI/corpus amplio**, **coerción 1:1 con VM** y **CHANGELOG** del subconjunto AOT.

Objetivo: llegar a un **AOT fiable** (`.jasb` → C → ejecutable) con **semántica alineada** al pipeline oficial (`.jbo` + VM), sin sustituir al compilador canónico sino **completando codegen + runtime** sobre el **mismo AST** (y opcionalmente IR más adelante).

Convención: `- [x]` hecho · `- [ ]` pendiente · En el texto **(parcial)** indica avance útil pero criterio de la casilla aún no cerrado del todo.

---

## Fase 0 — Cimientos (sin “romper lo que ya funciona”)

- [x] **Contrato de alcance**: tabla de soporte y ejemplos en `README.md` + `tests/lenguaje/99_smoke_aot.jasb`.
- [x] **Build reproducible**: `build_transpiler.bat` (Windows).
- [x] **Build POSIX**: `Makefile` o script shell equivalente.
- [x] **Cabeceras del transpilador**: `include/codegen_c.h` (API `jbc_generate_c` / `jbc_generate_c_opts`).
- [ ] **CI opcional pero deseable**: job que (1) construye `jbc-to-c.exe`, (2) transpila 1–3 `.jasb` de prueba, (3) ejecuta y compara salida esperada con diff o golden file.
- [ ] **Regla de oro en PRs**: cualquier cambio en `jas-compiler-c` que toque lexer/parser/AST debe pasar **tests del compilador + VM**; si afecta nodos ya emitidos en C, añadir o actualizar prueba AOT.

---

## Fase 1 — Runtime mínimo sólido (`jasboot_rt`)

- [x] **Modelo de valores**: `jb_var_t` con `nulo`, `entero`, `flotante`, `texto`, `bool`, `lista`, `mapa`, JSON; `jb_assign` / `jb_var_clone` / `jb_var_clear`.
- [x] **Operadores aritméticos**: `jb_add`, `jb_sub`, `jb_mul`, `jb_div`, `jb_mod`, `jb_neg`, concatenación textual en `+`.
- [x] **Comparaciones y lógica**: `jb_eq`, `jb_ne`, `jb_lt`, `jb_gt`, `jb_le`, `jb_ge`, `jb_land`, `jb_lor`, `jb_not`, `jb_truthy`.
- [ ] **Conversión / coerción**: alineación total con VM (sigue pendiente; muchos programas ya funcionan por rutas explícitas).
- [x] **E/S básica**: `jb_imprimir`, `jb_imprimir_sin_salto`, `jb_ingresar_texto`, entrada numérica, consola.
- [x] **Errores en runtime**: `jb_warn_aot`; división/módulo por cero con `intentar` activo → `jb_throw_val` (mínimo viable).

---

## Fase 2 — Codegen: núcleo imperativo

- [x] **Asignaciones** (`jb_assign` + l-value identificador; rutas de miembro/lista/mapas en subconjunto soportado).
- [x] **Expresiones**: literales, identificadores, binarios/unarios/ternario, llamadas **(parcial: operadores o nodos no cubiertos → `jb_warn_aot` / nulo)**.
- [x] **Bloques** (mismo ámbito C; sombras según C).
- [x] **`si` / `sino`** (`jb_truthy`).
- [x] **`mientras`**.
- [x] **`para`** (transformado a `while` con init/paso).
- [x] **`romper` / `continuar`**.
- [x] **`hacer_mientras`** (`NODE_DO_WHILE`).
- [x] **`retornar`** en funciones del mismo archivo (y tras fusión `usar` en un TU).

---

## Fase 3 — Funciones y modularidad

- [x] **`funcion` / `fin_funcion`** en el mismo `.jasb` (`static jb_var_t jbf_*`, prototipos).
- [x] **`principal`**: `main` + `globals` estáticas inicializadas al arranque.
- [x] **`usar` / imports (AOT)**: fusión previa en `aot_usar_merge.c` (funciones, globales `enviar`, registros exportados, validación `usar { }`). `NODE_ACTIVAR_MODULO` sin pipeline de fusión sigue como aviso/no-op según caso.
- [x] **Visibilidad `enviar` (AOT)**: directivas `enviar { }` aplicadas al fusionar; `NODE_EXPORT_DIRECTIVE` en codegen es no-op donde corresponde.

---

## Fase 4 — Tipos y datos compuestos

- [x] **`lista`**: array dinámico en `jasboot_rt`; `lista_*` y aliases `mem_lista_*` en codegen (con límites en mutación por rutas complejas).
- [x] **`mapa`**: array de pares clave/valor (orden inserción); `mapa_*`, `mapa_tamano`, iteración `para cada` (claves si tipo `texto`, si no valores).
- [x] **`registro` / campos (AOT)** (parcial): `.campo` como `jb_map_get`/`jb_map_put`; variables de tipo usuario y `mapa` sin inicializador → `jb_new_map()`; `usar { Registro }` fusiona el nodo `registro` exportado.
- [x] **`clase` / OOP** (parcial en AOT): métodos con `este`, despacho estático por tipo de clase (global, parámetro, **variable local** con inferencia desde declaración); **sin** herencia/layout idéntico al objeto VM nativo; **lambdas** en codegen siguen **no soportadas**.
- [x] **`texto`**: literales, `+`/`jb_concat`, `longitud_texto`, mayúsculas/minúsculas, conversión desde texto a número (subconjunto).
- [ ] **Memoria y fugas**: estrategia RAII/manual documentada; pruebas con Valgrind/ASan en C generado en ejemplos medianos.

---

## Fase 5 — Características avanzadas del lenguaje

- [x] **`clase` / OOP** — ver Fase 4 (parcial); pendiente: herencia, paridad exhaustiva con VM, más tests.
- [x] **`intentar` / `atrapar`** (parcial): mapeo mínimo a `setjmp`/`longjmp` en AOT; falta **documentar** semántica frente a VM y casos límite (`probar`/`capturar` si el front usa otros nombres, unificar doc).
- [ ] **Lambdas** (`NODE_LAMBDA_*`): explícitamente **no** en AOT (advertencia en codegen).
- [ ] **Concurrencia** (si existe en Jasboot): explícitamente fuera de alcance o plan con hilos y modelo de memoria.

---

## Fase 6 — Biblioteca y sistema (lo más costoso)

- [x] **Subconjunto de stdlib** (parcial): barrido de transpilación `stdlib/analitica-neuronal/**/*.jasb` (script `tests/verify_analitica_aot.cjs`, 34/34 al transpilar); **no** implica paridad VM en todos los módulos.
- [x] **API nativa / `sistema_*`** (parcial): capa `jb_*` para FS, JSON, tiempo, bits, texto, `sys_argv`, `imprimir_flotante`, matemáticas básicas, **`nativo_mlp_entrenar`**, subconjunto JMN; el resto (cognición avanzada, `mem_*` grafo, `n_*`, FFI, bytes/red, vectores/matrices, etc.) **no** tiene rama en `gen_call` salvo trabajo explícito — **inventario por área** en la subsección siguiente (caen en `jb_warn_aot` / `--strict`).
- [x] **JMN / `recordar` / `buscar` / `define_concepto`** (parcial): implementación **ligera** en `jasboot_rt` (mapa valores + mapa relaciones + persistencia texto tipo líneas `V|`/`R|`); `buscar` prioriza asociaciones como `OP_MEM_OBTENER_VALOR` en VM; **`recordar`** también crea arista 0.9 como VM. **No** es enlace a `jasboot-jmn-core` ni formato `.jmn` binario idéntico al VM.

### Inventario explícito — áreas típicamente **sin** rama `jb_*` en `gen_call`

> Orientación para porting y `--strict`: lo que no esté enlazado en `codegen_c.c` cae en **llamada no implementada** (`jb_warn_aot` o fallo estricto). La VM puede exponer muchas más `sistema_*`; esta tabla **no** pretende listar todas, solo los bloques que suelen chocar con AOT.

| Área | Ejemplos (sin implementación AOT dedicada en `gen_call`; salvo trabajo futuro) |
|------|----------------------------------------------------------------------------------|
| **Cognición / JMN avanzado** | `pensar`, `pensar_respuesta`, `procesar_texto`, `obtener_todos_conceptos`, `obtener_relacionados`, `obtener_nombre_concepto`, `imprimir_id`, `propiedad_concepto`, `asociar_*` (relación, similitud, secuencia, …), `comparar_patrones`, `buscar_en_memoria*`, `decae*`, `ventana_percepcion`, `percepcion_*`, `rastro_*`, `elegir_por_peso*`, … |
| **Memoria `mem_*` (no JMN ligero AOT)** | `mem_crear`, `mem_cerrar`, `mem_asociar`, `mem_*_u32`, `mem_aprender_peso_reg`, `mem_obtener_fuerza`, `mem_obtener_relacion`, … (distinto del subconjunto `mem_lista_*` ya mapeado donde aplica) |
| **Grafo `n_*`** | `n_abrir_grafo`, `n_recordar`, `n_buscar_*`, `n_tamano_grafo`, … |
| **Vectores / matrices** | **AOT:** `vec2`/`vec3`/`vec4` constructores, `+`/`-`/escalar`*`, `vec*_longitud`, `vec*_dot`, `vec*_normalizar`, `vec3_cross`, `vec*_sumar`/`restar`, `mat3_mul`/`mat3_mul_vec3`, `mat4_*` (`mul`, `mul_vec4`, `identidad`, `transpuesta`, `inversa`) en `jasboot_rt` + `gen_call`; miembros `.x`…`.w` y `.e0`…`.e15` vía `jb_vec_mat_component` / `jb_put_vec_mat_member` |
| **FFI** | `ffi_cargar`, `ffi_simbolo`, `ffi_llamar` |
| **Bytes / red** | `bytes_*`, `dns_resolver`, `tcp_*`, `tls_*` |
| **Varios** | `obtener_campo`, `finalizar` / `olvidar`, `reservar` / `liberar` / `ir_escribir`, `pausa` (sin milisegundos), `diferencia_en_segundos`, `comparar_gt_flt`, … |

- [ ] **Cerrar huecos por bloque** (opcional, por prioridad de producto): elegir un bloque de la tabla, implementar `jb_*` + rama en `gen_call`, prueba VM↔AOT y documentar en README.

---

## Fase 7 — Paridad y estabilidad (“completo”)

- [x] **Corpus de regresión** (parcial): `tests/verify_vm_aot.cjs` — lote por defecto (~21 pruebas: lenguaje 102–117, analítica 200–203, lab `pl_main`) comparando stdout VM vs AOT con normalización; **falta** ampliar a más `tests/` y stdlib completa con misma barra.
- [x] **Tabla de compatibilidad** en README: resumen por construcción (soportado / parcial / no) + guía de porting.
- [x] **Modo estricto**: flag `--strict` que **falla** (exit 2) si el codegen iba a emitir `jb_warn_aot` por soporte incompleto (llamadas no implementadas, lambdas, `usar` sin fusión, operadores no soportados, etc.).
- [ ] **Opción IR (opcional)**: spike de “`.jbo` → C” o “IR interno compartido” para reducir divergencia semántica frente a `codegen.c` del compilador — solo tras Fase 2–4 estables.
- [ ] **Versionado**: etiqueta de versión del subconjunto AOT (p. ej. `AOT_SPEC_0.1`) desacoplada de la versión del lenguaje completo.
- [x] **Guía de porting** para usuarios: bloque corto en README (qué evitar en AOT).

---

## Criterios de “estable” (definición operativa)

Marca esta fase cuando se cumplan **todas**:

- [ ] CI verde con corpus mínimo AOT + tests del compilador compartido en PRs que toquen el front.
- [ ] Lista de **breaking changes** del subconjunto AOT mantenida en CHANGELOG del directorio `jasboot-to-c`.
- [ ] Ningún programa soportado depende de **comportamiento indefinido** en C sin documentarlo.
- [ ] Tiempo de build y tamaño del binario aceptables para el caso de uso declarado (embebido vs escritorio).

---

## Recordatorio de riesgos

- **Duplicar semántica** entre VM y C sin tests comparados → bugs “fantasma”.
- **Crecer el runtime hasta ser una segunda VM** sin plan → deuda imposible de mantener.
- **Cambiar `jas-compiler-c` solo por AOT** sin validar `.jbo` → regresiones en el producto principal.

Mantén el transpilador como **consumidor** del canon del lenguaje; la estabilidad viene de **pruebas** y de **límites claros** hasta que el subconjunto crezca con evidencia.
