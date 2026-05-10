# Laboratorio pesado: JSON, binario y `usar` (3 módulos)

Demo interactiva para **VM** y **AOT** (`jbc-to-c`): menú por consola, lectura de JSON (una línea), archivos binarios temporales y estrés de escritura. El programa principal `pl_main.jasb` importa tres módulos con `usar { ... } de "..."`.

## Archivos

| Archivo | Rol |
|--------|-----|
| `pl_main.jasb` | `principal`, menú y orquestación |
| `pl_m_ui.jasb` | Textos de menú / banner |
| `pl_m_json.jasb` | JSON de ejemplo y utilidades (`pl_json_*`) |
| `pl_m_bin.jasb` | Rutas temp, marcador binario, estrés (`pl_bin_*`) |
| `pl_main.stdin` | Entrada automática para pruebas (misma base que `pl_main.jasb`) |

No edites manualmente `pl_main.c` ni el `.exe` generados: se regeneran con `jbc-to-c` y el compilador del repo. El transpilador omite `return jb_new_nulo();` al final de una `funcion` cuando el cuerpo ya termina en `retornar` (sin código muerto tras el último `return`).

## Cómo ejecutar

Desde la raíz del repo Jasboot (donde están `sdk-dependiente/...`):

```text
node sdk-dependiente/jasboot-to-c/tests/verify_vm_aot.cjs sdk-dependiente/jasboot-to-c/tests/lenguaje/pesado_bin_json_lab/pl_main.jasb
```

O interactivo: compilar con `jbc` y ejecutar la VM o el binario AOT con la misma carpeta de trabajo que el `.jasb` para que resuelvan bien las rutas relativas de los módulos.

## Notas de sintaxis

- Tras `llamar`, la llamada debe ir entre paréntesis: `llamar (f(x))`.
- Tras la opción **2 (JSON demo)**, las dos líneas numéricas son **códigos internos** (tipo de raíz JSON y conteo de elementos del arreglo `items`), no son opciones del menú.

## Entrada scriptada

No insertes líneas en blanco entre una línea JSON multibyte y la siguiente opción numérica del menú: una línea vacía se consumiría como `ingresar_texto` y rompería la secuencia.
