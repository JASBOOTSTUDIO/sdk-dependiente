/*
 * test_nuevos_modulos.c
 * Test simple para verificar los módulos críticos del JPM
 * 
 * Compilar:
 *   Windows: gcc test_nuevos_modulos.c src/jpm_pack.c src/jpm_install.c src/jpm_lock.c src/jpm_core.c src/jpm_json.c src/jpm_metadata.c -Iinclude -o test_modulos.exe
 *   Linux:   gcc test_nuevos_modulos.c src/jpm_pack.c src/jpm_install.c src/jpm_lock.c src/jpm_core.c src/jpm_json.c src/jpm_metadata.c -Iinclude -o test_modulos
 * 
 * Ejecutar:
 *   ./test_modulos.exe  (Windows)
 *   ./test_modulos      (Linux)
 */

#include "jpm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* Colores para terminal (opcional) */
#ifdef _WIN32
    #define COLOR_RESET  ""
    #define COLOR_GREEN  ""
    #define COLOR_RED    ""
    #define COLOR_YELLOW ""
    #define COLOR_BLUE   ""
#else
    #define COLOR_RESET  "\033[0m"
    #define COLOR_GREEN  "\033[32m"
    #define COLOR_RED    "\033[31m"
    #define COLOR_YELLOW "\033[33m"
    #define COLOR_BLUE   "\033[34m"
#endif

/* Contadores de tests */
static int tests_passed = 0;
static int tests_failed = 0;
static int tests_total = 0;

/* Macros de testing */
#define TEST_START(name) \
    do { \
        printf("\n%s[TEST]%s %s\n", COLOR_BLUE, COLOR_RESET, name); \
        tests_total++; \
    } while(0)

#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            printf("  %s[OK]%s %s\n", COLOR_GREEN, COLOR_RESET, message); \
            tests_passed++; \
        } else { \
            printf("  %s[FAIL]%s %s\n", COLOR_RED, COLOR_RESET, message); \
            tests_failed++; \
        } \
    } while(0)

#define TEST_INFO(message) \
    printf("  %s[INFO]%s %s\n", COLOR_YELLOW, COLOR_RESET, message)

/* Crear directorio de prueba */
static int crear_directorio_test(const char *path) {
#ifdef _WIN32
    return _mkdir(path);
#else
    return mkdir(path, 0755);
#endif
}

/* Crear archivo de prueba */
static int crear_archivo_test(const char *path, const char *contenido) {
    FILE *f = fopen(path, "w");
    if (!f) return -1;
    
    if (contenido) {
        fprintf(f, "%s", contenido);
    }
    fclose(f);
    return 0;
}

/* Test 1: Hash SHA-512 */
void test_sha512() {
    TEST_START("SHA-512: Cálculo de hash");
    
    /* Crear archivo de prueba */
    const char *test_file = "test_hash.txt";
    const char *test_content = "Hola JPM! Este es un archivo de prueba.";
    
    TEST_ASSERT(crear_archivo_test(test_file, test_content) == 0, 
                "Crear archivo de prueba");
    
    /* Calcular hash */
    char hash[129];
    int result = jpm_calcular_hash(test_file, hash, sizeof(hash));
    
    TEST_ASSERT(result == JPM_EXITO, "Calcular hash exitoso");
    TEST_ASSERT(strlen(hash) == 128, "Hash tiene longitud correcta (128 chars)");
    TEST_ASSERT(hash[0] != '\0', "Hash no está vacío");
    
    TEST_INFO("Hash calculado:");
    printf("    %.32s...\n", hash);
    
    /* Limpiar */
    remove(test_file);
}

/* Test 2: Lock - Crear y manipular */
void test_lock_basico() {
    TEST_START("Lock: Crear y manipular estructura");
    
    /* Crear lock */
    jpm_lock_t *lock = jpm_lock_crear();
    TEST_ASSERT(lock != NULL, "Crear estructura de lock");
    TEST_ASSERT(lock->num_entradas == 0, "Lock inicialmente vacío");
    
    /* Agregar entrada */
    jpm_lock_entrada_t entrada = {0};
    strncpy(entrada.nombre, "paquete-test", JPM_MAX_NOMBRE - 1);
    strncpy(entrada.version, "1.0.0", JPM_MAX_VERSION - 1);
    strncpy(entrada.resuelto, "test.jpkg", JPM_MAX_URL - 1);
    strncpy(entrada.hash, "abc123", 128);
    
    int result = jpm_lock_agregar_entrada(lock, &entrada);
    TEST_ASSERT(result == JPM_EXITO, "Agregar entrada al lock");
    TEST_ASSERT(lock->num_entradas == 1, "Lock tiene 1 entrada");
    
    /* Verificar que contiene el paquete */
    bool contiene = jpm_lock_contiene_paquete(lock, "paquete-test");
    TEST_ASSERT(contiene == true, "Lock contiene el paquete agregado");
    
    /* Liberar */
    jpm_lock_liberar(lock);
    TEST_INFO("Lock liberado correctamente");
}

/* Test 3: Lock - Escribir y leer archivo */
void test_lock_archivo() {
    TEST_START("Lock: Escribir y leer archivo jpm.lock");
    
    const char *lock_file = "test_jpm.lock";
    
    /* Crear lock con datos */
    jpm_lock_t *lock = jpm_lock_crear();
    
    jpm_lock_entrada_t entrada1 = {0};
    strncpy(entrada1.nombre, "biblioteca-a", JPM_MAX_NOMBRE - 1);
    strncpy(entrada1.version, "2.3.0", JPM_MAX_VERSION - 1);
    strncpy(entrada1.resuelto, "https://ejemplo.com/biblioteca-a-2.3.0.jpkg", JPM_MAX_URL - 1);
    jpm_lock_agregar_entrada(lock, &entrada1);
    
    jpm_lock_entrada_t entrada2 = {0};
    strncpy(entrada2.nombre, "biblioteca-b", JPM_MAX_NOMBRE - 1);
    strncpy(entrada2.version, "1.5.1", JPM_MAX_VERSION - 1);
    strncpy(entrada2.resuelto, "./local/biblioteca-b-1.5.1.jpkg", JPM_MAX_URL - 1);
    jpm_lock_agregar_entrada(lock, &entrada2);
    
    /* Escribir a archivo */
    int result = jpm_lock_escribir(lock, lock_file);
    TEST_ASSERT(result == JPM_EXITO, "Escribir lock a archivo");
    
    /* Verificar que el archivo existe */
    struct stat st;
    TEST_ASSERT(stat(lock_file, &st) == 0, "Archivo lock existe");
    
    jpm_lock_liberar(lock);
    
    /* Leer archivo */
    jpm_lock_t *lock_leido = jpm_lock_leer(lock_file);
    TEST_ASSERT(lock_leido != NULL, "Leer lock desde archivo");
    
    if (lock_leido) {
        TEST_INFO("Lock leído contiene entradas");
        jpm_lock_liberar(lock_leido);
    }
    
    /* Limpiar */
    remove(lock_file);
}

/* Test 4: Pack - Crear directorio de prueba y empaquetar */
void test_pack_basico() {
    TEST_START("Pack: Empaquetar directorio simple");
    
    const char *test_dir = "test_proyecto";
    const char *output_file = "test_proyecto.jpkg";
    
    /* Crear directorio de prueba */
    crear_directorio_test(test_dir);
    
    /* Crear jasboot.json */
    char json_path[256];
    snprintf(json_path, sizeof(json_path), "%s/jasboot.json", test_dir);
    const char *json_content = 
        "{\n"
        "  \"nombre\": \"test-proyecto\",\n"
        "  \"version\": \"1.0.0\",\n"
        "  \"descripcion\": \"Proyecto de prueba\",\n"
        "  \"autor\": \"Test\",\n"
        "  \"licencia\": \"MIT\"\n"
        "}\n";
    crear_archivo_test(json_path, json_content);
    
    /* Crear algunos archivos adicionales */
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/README.md", test_dir);
    crear_archivo_test(file_path, "# Proyecto de Prueba\n\nEste es un test.");
    
    /* Empaquetar */
    int result = jpm_empaquetar(test_dir, output_file);
    TEST_ASSERT(result == JPM_EXITO, "Empaquetar directorio");
    
    /* Verificar que se creó el archivo */
    struct stat st;
    if (stat(output_file, &st) == 0) {
        TEST_ASSERT(st.st_size > 0, "Archivo .jpkg tiene contenido");
        printf("    Tamaño: %ld bytes\n", (long)st.st_size);
    } else {
        TEST_ASSERT(0, "Archivo .jpkg creado");
    }
    
    /* Limpiar */
    remove(json_path);
    remove(file_path);
#ifdef _WIN32
    _rmdir(test_dir);
#else
    rmdir(test_dir);
#endif
    remove(output_file);
}

/* Test 5: Pack - Extraer paquete */
void test_extraer_basico() {
    TEST_START("Pack: Empaquetar y extraer");
    
    const char *test_dir = "test_pack_src";
    const char *extract_dir = "test_pack_dst";
    const char *jpkg_file = "test.jpkg";
    
    /* Crear directorio fuente */
    crear_directorio_test(test_dir);
    
    char json_path[256];
    snprintf(json_path, sizeof(json_path), "%s/jasboot.json", test_dir);
    crear_archivo_test(json_path, "{\"nombre\":\"test\",\"version\":\"1.0.0\"}");
    
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/test.txt", test_dir);
    crear_archivo_test(file_path, "Contenido de prueba");
    
    /* Empaquetar */
    int result = jpm_empaquetar(test_dir, jpkg_file);
    TEST_ASSERT(result == JPM_EXITO, "Empaquetar directorio");
    
    /* Extraer */
    result = jpm_extraer(jpkg_file, extract_dir);
    TEST_ASSERT(result == JPM_EXITO, "Extraer paquete");
    
    /* Verificar que los archivos existen */
    char extracted_json[256];
    snprintf(extracted_json, sizeof(extracted_json), "%s/jasboot.json", extract_dir);
    struct stat st;
    TEST_ASSERT(stat(extracted_json, &st) == 0, "jasboot.json extraído");
    
    char extracted_file[256];
    snprintf(extracted_file, sizeof(extracted_file), "%s/test.txt", extract_dir);
    TEST_ASSERT(stat(extracted_file, &st) == 0, "test.txt extraído");
    
    /* Limpiar */
    remove(json_path);
    remove(file_path);
    remove(jpkg_file);
    remove(extracted_json);
    remove(extracted_file);
#ifdef _WIN32
    _rmdir(test_dir);
    _rmdir(extract_dir);
#else
    rmdir(test_dir);
    rmdir(extract_dir);
#endif
}

/* Test 6: Verificación de integridad */
void test_integridad() {
    TEST_START("Pack: Verificación de integridad");
    
    /* Crear archivo de prueba */
    const char *test_file = "test_integrity.dat";
    crear_archivo_test(test_file, "Datos de prueba para verificación");
    
    /* Calcular hash */
    char hash1[129];
    jpm_calcular_hash(test_file, hash1, sizeof(hash1));
    
    /* Verificar integridad (debe coincidir) */
    bool valido = jpm_verificar_integridad(test_file, hash1);
    TEST_ASSERT(valido == true, "Hash coincide con archivo");
    
    /* Modificar archivo */
    FILE *f = fopen(test_file, "a");
    fprintf(f, "modificado");
    fclose(f);
    
    /* Verificar integridad (NO debe coincidir) */
    valido = jpm_verificar_integridad(test_file, hash1);
    TEST_ASSERT(valido == false, "Hash NO coincide con archivo modificado");
    
    /* Limpiar */
    remove(test_file);
}

/* Main */
int main(void) {
    printf("\n");
    printf("========================================\n");
    printf("  JPM - Tests de Módulos Críticos\n");
    printf("========================================\n");
    
    /* Ejecutar tests */
    test_sha512();
    test_lock_basico();
    test_lock_archivo();
    test_pack_basico();
    test_extraer_basico();
    test_integridad();
    
    /* Resumen */
    printf("\n");
    printf("========================================\n");
    printf("  RESUMEN DE TESTS\n");
    printf("========================================\n");
    printf("  Total:   %d tests\n", tests_total);
    printf("  %sPasados:%s %d\n", COLOR_GREEN, COLOR_RESET, tests_passed);
    printf("  %sFallados:%s %d\n", COLOR_RED, COLOR_RESET, tests_failed);
    printf("========================================\n");
    
    if (tests_failed == 0) {
        printf("\n%s✓ TODOS LOS TESTS PASARON%s\n\n", COLOR_GREEN, COLOR_RESET);
        return 0;
    } else {
        printf("\n%s✗ ALGUNOS TESTS FALLARON%s\n\n", COLOR_RED, COLOR_RESET);
        return 1;
    }
}