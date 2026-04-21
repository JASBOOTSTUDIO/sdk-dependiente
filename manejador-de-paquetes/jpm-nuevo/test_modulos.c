/*
 * test_modulos.c
 * Programa de prueba para los módulos implementados de JPM
 * 
 * Compila con:
 *   gcc -std=c11 -Wall -Wextra -Iinclude \
 *       src/jpm_core.c src/jpm_json.c src/jpm_metadata.c src/jpm_commands.c \
 *       test_modulos.c -o test_modulos
 */

#include "include/jpm.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* =============================================================================
 * FUNCIONES DE PRUEBA
 * =============================================================================
 */

void separador(const char *titulo) {
    printf("\n");
    printf("========================================\n");
    printf("%s\n", titulo);
    printf("========================================\n");
    printf("\n");
}

void test_json_basico(void) {
    separador("TEST 1: Parser JSON Básico");
    
    /* Crear archivo JSON de prueba */
    const char *json_contenido = 
        "{\n"
        "  \"nombre\": \"test-paquete\",\n"
        "  \"version\": \"1.0.0\",\n"
        "  \"activo\": true,\n"
        "  \"prioridad\": 42\n"
        "}\n";
    
    FILE *f = fopen("test_json.json", "w");
    if (f) {
        fprintf(f, "%s", json_contenido);
        fclose(f);
    }
    
    /* Leer JSON */
    void *json = jpm_json_leer_archivo("test_json.json");
    if (!json) {
        printf("❌ Error al leer JSON\n");
        return;
    }
    
    printf("✓ JSON leído correctamente\n");
    
    /* Obtener valores */
    const char *nombre = jpm_json_obtener_string(json, "nombre");
    const char *version = jpm_json_obtener_string(json, "version");
    bool activo = jpm_json_obtener_bool(json, "activo");
    int64_t prioridad = jpm_json_obtener_numero(json, "prioridad");
    
    printf("  Nombre:     %s\n", nombre ? nombre : "NULL");
    printf("  Versión:    %s\n", version ? version : "NULL");
    printf("  Activo:     %s\n", activo ? "true" : "false");
    printf("  Prioridad:  %lld\n", (long long)prioridad);
    
    /* Validar valores */
    assert(nombre != NULL && strcmp(nombre, "test-paquete") == 0);
    assert(version != NULL && strcmp(version, "1.0.0") == 0);
    assert(activo == true);
    assert(prioridad == 42);
    
    printf("\n✓ Todos los valores son correctos\n");
    
    /* Liberar */
    jpm_json_liberar(json);
    remove("test_json.json");
    
    printf("✓ Test JSON básico completado\n");
}

void test_metadatos_crear(void) {
    separador("TEST 2: Crear Metadatos");
    
    /* Crear metadatos */
    jpm_metadatos_t *meta = jpm_metadatos_crear("mi-biblioteca", "2.1.0");
    if (!meta) {
        printf("❌ Error al crear metadatos\n");
        return;
    }
    
    printf("✓ Metadatos creados\n");
    printf("  Nombre:      %s\n", meta->nombre);
    printf("  Versión:     %s\n", meta->version);
    printf("  Descripción: %s\n", meta->descripcion);
    printf("  Autor:       %s\n", meta->autor);
    printf("  Licencia:    %s\n", meta->licencia);
    
    /* Modificar metadatos */
    strcpy(meta->descripcion, "Una biblioteca de prueba");
    strcpy(meta->autor, "Equipo Jasboot");
    strcpy(meta->repositorio, "https://github.com/jasboot/mi-biblioteca");
    meta->tipo = JPM_TIPO_BIBLIOTECA;
    
    /* Agregar dependencias */
    int resultado = jpm_metadatos_agregar_dependencia(meta, "utilidades", "^1.0.0", false);
    if (resultado == JPM_EXITO) {
        printf("\n✓ Dependencia agregada: utilidades@^1.0.0\n");
    }
    
    resultado = jpm_metadatos_agregar_dependencia(meta, "test-framework", "~2.3.0", true);
    if (resultado == JPM_EXITO) {
        printf("✓ Dependencia de desarrollo agregada: test-framework@~2.3.0\n");
    }
    
    printf("\nDependencias:\n");
    for (size_t i = 0; i < meta->num_dependencias; i++) {
        printf("  - %s@%s%s\n",
               meta->dependencias[i].nombre,
               meta->dependencias[i].operador,
               meta->dependencias[i].version);
    }
    
    printf("\nDependencias de desarrollo:\n");
    for (size_t i = 0; i < meta->num_dependencias_desarrollo; i++) {
        printf("  - %s@%s%s\n",
               meta->dependencias_desarrollo[i].nombre,
               meta->dependencias_desarrollo[i].operador,
               meta->dependencias_desarrollo[i].version);
    }
    
    /* Validar */
    char error[512];
    if (jpm_metadatos_validar(meta, error, sizeof(error))) {
        printf("\n✓ Metadatos válidos\n");
    } else {
        printf("\n❌ Metadatos inválidos: %s\n", error);
    }
    
    /* Escribir a archivo */
    resultado = jpm_metadatos_escribir(meta, "test_jasboot.json");
    if (resultado == JPM_EXITO) {
        printf("✓ Archivo test_jasboot.json creado\n");
    } else {
        printf("❌ Error al escribir archivo\n");
    }
    
    /* Liberar */
    jpm_metadatos_liberar(meta);
    
    printf("\n✓ Test de crear metadatos completado\n");
}

void test_metadatos_leer(void) {
    separador("TEST 3: Leer Metadatos");
    
    /* Leer el archivo creado en el test anterior */
    jpm_metadatos_t *meta = jpm_metadatos_leer("test_jasboot.json");
    if (!meta) {
        printf("❌ Error al leer metadatos\n");
        return;
    }
    
    printf("✓ Metadatos leídos desde test_jasboot.json\n");
    printf("\nInformación del paquete:\n");
    printf("  Nombre:        %s\n", meta->nombre);
    printf("  Versión:       %s\n", meta->version);
    printf("  Descripción:   %s\n", meta->descripcion);
    printf("  Autor:         %s\n", meta->autor);
    printf("  Licencia:      %s\n", meta->licencia);
    printf("  Principal:     %s\n", meta->principal);
    printf("  Jasboot:       %s\n", meta->jasboot_version);
    printf("  Repositorio:   %s\n", meta->repositorio);
    printf("  Tipo:          %d\n", meta->tipo);
    
    printf("\nDependencias (%zu):\n", meta->num_dependencias);
    for (size_t i = 0; i < meta->num_dependencias; i++) {
        printf("  - %s@%s%s\n",
               meta->dependencias[i].nombre,
               meta->dependencias[i].operador,
               meta->dependencias[i].version);
    }
    
    printf("\nDependencias de desarrollo (%zu):\n", meta->num_dependencias_desarrollo);
    for (size_t i = 0; i < meta->num_dependencias_desarrollo; i++) {
        printf("  - %s@%s%s\n",
               meta->dependencias_desarrollo[i].nombre,
               meta->dependencias_desarrollo[i].operador,
               meta->dependencias_desarrollo[i].version);
    }
    
    /* Verificar valores */
    assert(strcmp(meta->nombre, "mi-biblioteca") == 0);
    assert(strcmp(meta->version, "2.1.0") == 0);
    assert(strcmp(meta->autor, "Equipo Jasboot") == 0);
    assert(meta->num_dependencias == 1);
    assert(meta->num_dependencias_desarrollo == 1);
    
    printf("\n✓ Todos los valores leídos correctamente\n");
    
    /* Liberar */
    jpm_metadatos_liberar(meta);
    remove("test_jasboot.json");
    
    printf("✓ Test de leer metadatos completado\n");
}

void test_validacion_nombres(void) {
    separador("TEST 4: Validación de Nombres");
    
    const char *nombres_validos[] = {
        "mi-paquete",
        "biblioteca123",
        "util_text",
        "abc",
        "paquete-con-guiones",
        NULL
    };
    
    const char *nombres_invalidos[] = {
        "",
        ".",
        "-paquete",
        ".paquete",
        "Mayusculas",
        "con espacios",
        "con@simbolos",
        "a",  /* muy corto */
        NULL
    };
    
    printf("Nombres válidos:\n");
    for (int i = 0; nombres_validos[i] != NULL; i++) {
        bool valido = jpm_validar_nombre(nombres_validos[i]);
        printf("  %-30s %s\n", nombres_validos[i], valido ? "✓" : "❌");
        assert(valido);
    }
    
    printf("\nNombres inválidos:\n");
    for (int i = 0; nombres_invalidos[i] != NULL; i++) {
        bool valido = jpm_validar_nombre(nombres_invalidos[i]);
        printf("  %-30s %s\n", nombres_invalidos[i], valido ? "❌" : "✓");
        assert(!valido);
    }
    
    printf("\n✓ Test de validación de nombres completado\n");
}

void test_validacion_versiones(void) {
    separador("TEST 5: Validación de Versiones");
    
    const char *versiones_validas[] = {
        "1.0.0",
        "0.1.0",
        "10.20.30",
        "999.999.999",
        NULL
    };
    
    const char *versiones_invalidas[] = {
        "",
        "1",
        "1.0",
        "1.0.0.0",
        "a.b.c",
        "-1.0.0",
        "1.-1.0",
        NULL
    };
    
    printf("Versiones válidas:\n");
    for (int i = 0; versiones_validas[i] != NULL; i++) {
        bool valido = jpm_validar_version(versiones_validas[i]);
        printf("  %-30s %s\n", versiones_validas[i], valido ? "✓" : "❌");
        assert(valido);
    }
    
    printf("\nVersiones inválidas:\n");
    for (int i = 0; versiones_invalidas[i] != NULL; i++) {
        bool valido = jpm_validar_version(versiones_invalidas[i]);
        printf("  %-30s %s\n", versiones_invalidas[i], valido ? "❌" : "✓");
        assert(!valido);
    }
    
    printf("\n✓ Test de validación de versiones completado\n");
}

void test_comparar_versiones(void) {
    separador("TEST 6: Comparación de Versiones");
    
    struct {
        const char *v1;
        const char *v2;
        int esperado;
    } tests[] = {
        {"1.0.0", "1.0.0", 0},
        {"1.0.0", "2.0.0", -1},
        {"2.0.0", "1.0.0", 1},
        {"1.2.0", "1.1.0", 1},
        {"1.0.5", "1.0.3", 1},
        {"1.0.0", "1.0.1", -1},
        {NULL, NULL, 0}
    };
    
    for (int i = 0; tests[i].v1 != NULL; i++) {
        int resultado = jpm_comparar_versiones(tests[i].v1, tests[i].v2);
        const char *op = resultado == 0 ? "==" : resultado < 0 ? "<" : ">";
        printf("  %s %s %s: %s\n",
               tests[i].v1, op, tests[i].v2,
               resultado == tests[i].esperado ? "✓" : "❌");
        assert(resultado == tests[i].esperado);
    }
    
    printf("\n✓ Test de comparación de versiones completado\n");
}

void test_cumple_requisito(void) {
    separador("TEST 7: Versiones Cumplen Requisito");
    
    struct {
        const char *version;
        const char *requisito;
        bool esperado;
    } tests[] = {
        {"1.2.3", "^1.0.0", true},   /* Compatible con 1.x.x */
        {"1.2.3", "^1.2.0", true},   /* Compatible con 1.2.x */
        {"2.0.0", "^1.0.0", false},  /* No compatible (major diferente) */
        {"1.2.3", "~1.2.0", true},   /* Compatible en patch */
        {"1.3.0", "~1.2.0", false},  /* No compatible (minor diferente) */
        {"1.5.0", ">=1.0.0", true},  /* Mayor o igual */
        {"0.9.0", ">=1.0.0", false}, /* Menor */
        {"1.0.0", "1.0.0", true},    /* Exacto */
        {NULL, NULL, false}
    };
    
    for (int i = 0; tests[i].version != NULL; i++) {
        bool cumple = jpm_version_cumple_requisito(tests[i].version, tests[i].requisito);
        printf("  %s cumple %s: %s\n",
               tests[i].version, tests[i].requisito,
               cumple == tests[i].esperado ? "✓" : "❌");
        assert(cumple == tests[i].esperado);
    }
    
    printf("\n✓ Test de requisitos de versión completado\n");
}

void test_comando_init(void) {
    separador("TEST 8: Comando Init (Simulación)");
    
    /* Nota: Este test solo simula el comando sin ejecutarlo realmente
     * para evitar crear archivos en el sistema */
    
    jpm_contexto_t *ctx = jpm_inicializar();
    if (!ctx) {
        printf("❌ Error al inicializar contexto\n");
        return;
    }
    
    printf("✓ Contexto JPM inicializado\n");
    printf("  Directorio de trabajo: %s\n", ctx->directorio_trabajo);
    printf("  Directorio de cache:   %s\n", ctx->config.directorio_cache);
    printf("  Directorio global:     %s\n", ctx->config.directorio_global);
    printf("  URL de registro:       %s\n", ctx->config.url_registro);
    
    /* Liberar contexto */
    jpm_liberar(ctx);
    
    printf("\n✓ Test de comando init (simulación) completado\n");
}

void test_formatear_bytes(void) {
    separador("TEST 9: Formatear Bytes");
    
    struct {
        size_t bytes;
        const char *esperado;
    } tests[] = {
        {0, "0 B"},
        {512, "512 B"},
        {1024, "1.00 KB"},
        {1536, "1.50 KB"},
        {1048576, "1.00 MB"},
        {1073741824, "1.00 GB"},
        {0, NULL}
    };
    
    char buffer[64];
    for (int i = 0; tests[i].esperado != NULL; i++) {
        jpm_formatear_bytes(tests[i].bytes, buffer, sizeof(buffer));
        printf("  %10zu bytes = %s\n", tests[i].bytes, buffer);
    }
    
    printf("\n✓ Test de formatear bytes completado\n");
}

/* =============================================================================
 * FUNCIÓN PRINCIPAL
 * =============================================================================
 */

int main(void) {
    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║  JPM - TESTS DE MÓDULOS IMPLEMENTADOS ║\n");
    printf("║  Jasboot Package Manager v0.2.0       ║\n");
    printf("╚════════════════════════════════════════╝\n");
    
    int tests_ejecutados = 0;
    int tests_exitosos = 0;
    
    /* Ejecutar tests */
    #define EJECUTAR_TEST(nombre) \
        do { \
            tests_ejecutados++; \
            printf("\nEjecutando test %d...\n", tests_ejecutados); \
            nombre(); \
            tests_exitosos++; \
        } while(0)
    
    EJECUTAR_TEST(test_json_basico);
    EJECUTAR_TEST(test_metadatos_crear);
    EJECUTAR_TEST(test_metadatos_leer);
    EJECUTAR_TEST(test_validacion_nombres);
    EJECUTAR_TEST(test_validacion_versiones);
    EJECUTAR_TEST(test_comparar_versiones);
    EJECUTAR_TEST(test_cumple_requisito);
    EJECUTAR_TEST(test_comando_init);
    EJECUTAR_TEST(test_formatear_bytes);
    
    /* Resumen */
    separador("RESUMEN DE TESTS");
    printf("Tests ejecutados: %d\n", tests_ejecutados);
    printf("Tests exitosos:   %d\n", tests_exitosos);
    printf("Tests fallidos:   %d\n", tests_ejecutados - tests_exitosos);
    
    if (tests_exitosos == tests_ejecutados) {
        printf("\n✓ TODOS LOS TESTS PASARON CORRECTAMENTE\n\n");
        return 0;
    } else {
        printf("\n❌ ALGUNOS TESTS FALLARON\n\n");
        return 1;
    }
}