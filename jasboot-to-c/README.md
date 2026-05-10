# Jasboot-to-C (transpilador AOT)

Herramienta experimental que convierte un **subconjunto** de programas **Jasboot** (`.jasb`) a **C**, enlaza con un **runtime mínimo** (`runtime/jasboot_rt.c`) y produce un ejecutable nativo con **GCC**.

No sustituye al pipeline oficial **compilador C → bytecode `.jbo` → VM `jasboot-ir`**. Es un camino paralelo para demos embebidas, arranque sin VM y, a medio plazo, despliegue AOT donde interese.

---

## Dirección del proyecto (visión)

1. **Reutilizar el front-end del lenguaje**  
   El `build_transpiler.bat` enlaza **lexer, parser, AST y utilidades** desde `sdk-dependiente/jas-compiler-c/`.  
   **No** se debe reescribir un segundo analizador léxico/sintáctico en paralelo: cualquier cambio de sintaxis del lenguaje debe vivir en `jas-compiler-c` y este SDK solo amplía **codegen C** y **runtime**.

2. **Subconjunto explícito y documentado**  
   Hoy solo se emite C para lo que `src/codegen_c.c` implemente. El resto de nodos del AST puede aparecer como comentarios `// Nodo no soportado` o similar. Eso es intencional: crecer por **capas** (imprimir, literales, declaraciones, luego control de flujo, etc.), no pretender paridad completa de golpe.

3. **Evitar divergencia de semántica**  
   El objetivo a largo plazo es que el significado de un programa soportado coincida con el del compilador oficial. Para ello, la prioridad es **compartir AST/parser** (hecho) y, si en el futuro se desea, **consumir IR (`.jbo`)** o una API común, en lugar de mantener dos gramáticas distintas.

4. **Runtime pequeño y acotado**  
   `jasboot_rt` modela valores tipo etiqueta (`jb_var_t`). Las operaciones crecen solo cuando el codegen las necesite; no es una segunda VM completa salvo que el alcance del proyecto lo exija explícitamente.

---

## Compatibilidad AOT (resumen)

Criterio: **Sí** = codegen estable para el patrón habitual · **Parcial** = casos limitados o semántica distinta de la VM · **No** = no transpila de forma útil (advertencia en runtime o rechazo con `--strict`).

| Construcción | Estado |
|--------------|--------|
| `principal`, bloques, `imprimir` / `imprimir_sin_salto`, `responder` | Sí |
| Literales `entero`, `flotante`, `texto`, `bool`, `caracter`, `nulo` | Sí |
| Variables locales/globales, asignación, `ingresar_texto` | Sí |
| Operadores `+ - * / %`, `<<` / `>>`, comparaciones, `y`, `o`, unario `no` / `-`, ternario | Sí |
| Otros operadores binarios/unarios | No (permisivo: `jb_warn_aot`; `--strict`: error) |
| `si`, `mientras`, `para`, `para cada`, `romper` / `continuar`, `hacer_mientras` | Sí |
| `seleccionar` / casos (incl. rangos según parser) | Parcial |
| `intentar` / `atrapar` / `lanzar` (setjmp/longjmp) | Parcial |
| Listas `lista_*` / `mem_lista_*` | Sí (formas soportadas; rutas `m.a.lista` y variable identificador; otras → aviso o `--strict`) |
| Mapas `mapa_*`, literales `lista` / `mapa` / `json` | Sí (subconjunto) |
| Texto, tiempo, bits, `sys_argc` / `sys_argv`, consola, E/S numérica | Sí (subconjunto documentado en codegen) |
| `sistema_*` / API nativa amplia | Parcial (lo no mapeado → `jb_warn_aot` o fallo con `--strict`) |
| Archivos `fs_*` / aliases | Sí (subconjunto) |
| JSON (`json_*`) | Sí |
| Funciones en el mismo TU (tras fusión `usar`, un solo `.c`) | Sí |
| `usar { }` / fusión de módulos (`aot_usar_merge.c`) | Sí (antes de codegen) |
| Nodos `usar`/`biblioteca` **sin** fusión (AST residual) | No (aviso; `--strict`: error) |
| `registro`, `clase`, OOP, `este` | Parcial (mapa + despacho; no paridad VM completa) |
| Lambdas | No |
| JMN (`recordar`, `buscar`, …) | Parcial (runtime **ligero** en `jasboot_rt`, no binario JMN de la VM) |
| `nativo_mlp_entrenar` | Sí (vía `jb_nativo_mlp_entrenar`) |

### Modo estricto `--strict`

Si el programa usa algo que el AOT solo cubre con `jb_warn_aot` o con C inválido, el transpilador **sale con código 2**, imprime el motivo en stderr y **intenta borrar** el `.c` parcial. Útil en CI o antes de distribuir un binario AOT.

### Guía breve de porting (hacia AOT)

1. Preferir un **único `.jasb`** o `usar { … }` que la CLI fusione; evitar `biblioteca`/`usar` sin pipeline de fusión.  
2. Evitar **lambdas** y llamadas a **sistema_*** / stdlib no enlazadas en `codegen_c.c` hasta comprobar que exista rama AOT.  
3. Asignaciones a **miembros anidados** (`a.b.c`): solo cadena de `.` desde un identificador raíz.  
4. `lista_limpiar` / `lista_liberar`: argumento **variable** o ruta `mapa.campo…` reconocida por el codegen.  
5. Para paridad con la VM, usar `tests/verify_vm_aot.cjs`; el JMN AOT no sustituye al grafo binario completo de `jasboot-jmn-core`.

Los ejemplos `tests/lenguaje/01_hola_mundo.jasb` y `99_smoke_aot.jasb` sirven de humo AOT. **`105_atm_interactivo.jasb`** es una aplicación tipo cajero (PIN, menú, depósito/retiro, historial en lista, mapa de promoción, panel de bits/math, `ingresar_texto`); ejecución interactiva o con `< 105_atm_interactivo.stdin` en la consola.

**Paridad con la VM (`jbc` + `jasboot-ir`):** con el compilador y `jbc-to-c.exe` construidos, desde la raíz del repo:

```text
node sdk-dependiente/jasboot-to-c/tests/verify_vm_aot.cjs
node sdk-dependiente/jasboot-to-c/tests/verify_vm_aot.cjs ruta/al/archivo.jasb
```

Sin argumentos ejecuta el lote por defecto: **102–110** (`105` + `105_atm_interactivo.stdin` si existe; `106_aot_shift_alias.jasb` prueba `<<`/`>>` y `segmentar_palabras`/`palabras_de`; `107`–`109` archivos/`fs_*`; `110_aot_json.jasb` prueba JSON). El script transpila con `jbc-to-c` (sin `-e`), enlaza `runtime/jasboot_rt.c` y `runtime/jasboot_rt_json.c`, ejecuta el `.exe` generado y compara **stdout** con la VM; puede **normalizar** `verdadero`/`falso` ↔ `1`/`0` y comillas en líneas de texto. Si en Windows falla el enlace con *Permission denied*, cierre el ejecutable del ATM que esté abierto o bloqueado por el IDE.

---

## Qué no es este proyecto (anti-objetivos)

- No es el **runtime oficial** de Jasboot (eso sigue siendo la VM + JMN según el repo principal).
- No es un competidor del compilador `.jasb` → `.jbo` sin coordinación: **no duplicar** `jas-compiler-c` fuera de codegen + pegado de fuentes ya previsto en el script de build.
- No prometer paridad con **stdlib**, **memoria neuronal** ni **analítica** hasta que exista un plan de alcance por fases.

---

## Requisitos

- **GCC** en `PATH` (el `main.c` invoca `gcc` para enlazar el `.c` generado con `runtime/jasboot_rt.c`).
- Windows: script `build_transpiler.bat` asume entorno tipo MinGW/MSYS donde `gcc` resuelve.

---

## Construcción

Desde `sdk-dependiente/jasboot-to-c/`:

```bat
build_transpiler.bat
```

Genera `jbc-to-c.exe` enlazando `src/main.c`, `src/codegen_c.c` y las fuentes compartidas de `jas-compiler-c` listadas en el `.bat`.

---

## Uso

```text
jbc-to-c.exe <archivo.jasb> [-o programa.exe] [-e] [--strict]
```

- `-o` / `--output`: ejecutable (sin ruta: misma carpeta que el `.jasb`; con ruta, se respeta).
- `-e` / `--ejecutar`: tras compilar, ejecuta el binario.
- `--strict`: falla en codegen si aparece construcción no soportada (salida 2, borra `.c` parcial).

El transpilador escribe el `.c` y, por defecto, el `.exe` en la **misma carpeta que el `.jasb`**, y compila con `-I` apuntando a `runtime/`.

---

## Estructura del directorio

| Ruta | Rol |
|------|-----|
| `src/main.c` | CLI, lexer/parser compartidos, invocación a GCC. |
| `src/codegen_c.c` | Emisión de C desde el AST (**zona de extensión principal**). |
| `runtime/jasboot_rt.c` / `.h` | Runtime mínimo (valores, imprimir, init). |
| `tests/lenguaje/` | Ejemplos y pruebas manuales del subconjunto. |
| `build_transpiler.bat` | Build reproducible enlazando `jas-compiler-c`. |

---

## Hoja de ruta sugerida (orden lógico)

1. **Completar codegen** para los nodos que ya produce el parser (asignaciones, aritmética básica alineada con `jasboot_rt`, `si`/`mientras` reducidos, etc.).
2. **Tabla de compatibilidad** en este README (checkbox por construcción) actualizada en cada PR relevante.
3. **Evaluación opcional**: lector de `.jbo` o hook al pipeline IR para reducir duplicación semántica frente al codegen del compilador (solo cuando el subconjunto AOT sea estable).

**Checklist detallado** (fases 0–7, criterios de estabilidad y riesgos): ver [`CHECKLIST_TRANSPILACION_C_COMPLETA.md`](./CHECKLIST_TRANSPILACION_C_COMPLETA.md).

---

## Relación con el monorepo Jasboot

| Componente | Rol |
|-------------|-----|
| `jas-compiler-c` | Canon léxico/sintáctico y AST. |
| `jasboot-ir` | Ejecución estándar vía bytecode. |
| **jasboot-to-c** | Experimento AOT: mismo AST, salida C + GCC. |

Preguntas de diseño nuevas (palabras reservadas, tipos, imports) deben resolverse en el **compilador oficial**; aquí solo se decide **cómo se bajan a C** y qué piezas del runtime nativo las sustentan.

---

## Licencia y mantenimiento

Seguir la política del repositorio raíz `jasboot`. Para cambios que toquen sintaxis o AST compartido, coordinar con cambios en `jas-compiler-c` para no romper ni la VM ni este transpilador.
