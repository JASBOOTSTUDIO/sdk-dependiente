/*
 * JPM - Jasboot Package Manager
 * Módulo de Empaquetado: jpm_pack.c
 * 
 * Empaquetado y extracción de archivos .jpkg (formato ZIP)
 * Cálculo de hash SHA-512 para verificación de integridad
 * Copyright (c) 2024 Jasboot Team
 */

#include "../include/jpm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>

/* Macro para verificar si es directorio (compatible con Windows) */
#ifndef S_ISDIR
    #define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

#ifdef _WIN32
    #include <direct.h>
    #include <windows.h>
    #define PATH_SEP "\\"
    #define mkdir(path, mode) _mkdir(path)
#else
    #include <unistd.h>
    #include <dirent.h>
    #define PATH_SEP "/"
#endif

/* =============================================================================
 * IMPLEMENTACIÓN SHA-512 (Dominio Público)
 * Basado en FIPS 180-4
 * =============================================================================
 */

typedef struct {
    uint64_t state[8];
    uint64_t count[2];
    uint8_t buffer[128];
} sha512_ctx_t;

static const uint64_t K512[80] = {
    0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
    0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL, 0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
    0xd807aa98a3030242ULL, 0x12835b0145706fbeULL, 0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
    0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
    0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL, 0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
    0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
    0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL, 0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
    0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL, 0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
    0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
    0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
    0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL, 0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
    0xd192e819d6ef5218ULL, 0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
    0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL, 0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
    0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL, 0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
    0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
    0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
    0xca273eceea26619cULL, 0xd186b8c721c0c207ULL, 0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
    0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL, 0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
    0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
    0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL, 0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
};

#define ROR64(x, n) (((x) >> (n)) | ((x) << (64 - (n))))
#define SHR64(x, n) ((x) >> (n))

#define CH(x, y, z)  (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define SIGMA0(x) (ROR64(x, 28) ^ ROR64(x, 34) ^ ROR64(x, 39))
#define SIGMA1(x) (ROR64(x, 14) ^ ROR64(x, 18) ^ ROR64(x, 41))
#define sigma0(x) (ROR64(x, 1) ^ ROR64(x, 8) ^ SHR64(x, 7))
#define sigma1(x) (ROR64(x, 19) ^ ROR64(x, 61) ^ SHR64(x, 6))

static void sha512_transform(sha512_ctx_t *ctx, const uint8_t *data) {
    uint64_t W[80];
    uint64_t a, b, c, d, e, f, g, h;
    uint64_t T1, T2;
    int i;

    /* Preparar schedule */
    for (i = 0; i < 16; i++) {
        W[i] = ((uint64_t)data[i * 8] << 56) |
               ((uint64_t)data[i * 8 + 1] << 48) |
               ((uint64_t)data[i * 8 + 2] << 40) |
               ((uint64_t)data[i * 8 + 3] << 32) |
               ((uint64_t)data[i * 8 + 4] << 24) |
               ((uint64_t)data[i * 8 + 5] << 16) |
               ((uint64_t)data[i * 8 + 6] << 8) |
               ((uint64_t)data[i * 8 + 7]);
    }
    for (i = 16; i < 80; i++) {
        W[i] = sigma1(W[i - 2]) + W[i - 7] + sigma0(W[i - 15]) + W[i - 16];
    }

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    for (i = 0; i < 80; i++) {
        T1 = h + SIGMA1(e) + CH(e, f, g) + K512[i] + W[i];
        T2 = SIGMA0(a) + MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

static void sha512_init(sha512_ctx_t *ctx) {
    ctx->state[0] = 0x6a09e667f3bcc908ULL;
    ctx->state[1] = 0xbb67ae8584caa73bULL;
    ctx->state[2] = 0x3c6ef372fe94f82bULL;
    ctx->state[3] = 0xa54ff53a5f1d36f1ULL;
    ctx->state[4] = 0x510e527fade682d1ULL;
    ctx->state[5] = 0x9b05688c2b3e6c1fULL;
    ctx->state[6] = 0x1f83d9abfb41bd6bULL;
    ctx->state[7] = 0x5be0cd19137e2179ULL;
    ctx->count[0] = 0;
    ctx->count[1] = 0;
}

static void sha512_update(sha512_ctx_t *ctx, const uint8_t *data, size_t len) {
    size_t i, index, part_len;

    index = (size_t)((ctx->count[0] >> 3) & 0x7F);

    if ((ctx->count[0] += ((uint64_t)len << 3)) < ((uint64_t)len << 3)) {
        ctx->count[1]++;
    }
    ctx->count[1] += ((uint64_t)len >> 61);

    part_len = 128 - index;

    if (len >= part_len) {
        memcpy(&ctx->buffer[index], data, part_len);
        sha512_transform(ctx, ctx->buffer);

        for (i = part_len; i + 127 < len; i += 128) {
            sha512_transform(ctx, &data[i]);
        }
        index = 0;
    } else {
        i = 0;
    }

    memcpy(&ctx->buffer[index], &data[i], len - i);
}

static void sha512_final(uint8_t digest[64], sha512_ctx_t *ctx) {
    uint8_t bits[16];
    size_t index, pad_len;
    int i, j;

    /* Guardar longitud */
    for (i = 0; i < 8; i++) {
        bits[i] = (uint8_t)(ctx->count[1] >> (56 - i * 8));
    }
    for (i = 0; i < 8; i++) {
        bits[8 + i] = (uint8_t)(ctx->count[0] >> (56 - i * 8));
    }

    /* Padding */
    index = (size_t)((ctx->count[0] >> 3) & 0x7F);
    pad_len = (index < 112) ? (112 - index) : (240 - index);
    
    uint8_t padding[128];
    memset(padding, 0, sizeof(padding));
    padding[0] = 0x80;
    
    sha512_update(ctx, padding, pad_len);
    sha512_update(ctx, bits, 16);

    /* Salida */
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            digest[i * 8 + j] = (uint8_t)(ctx->state[i] >> (56 - j * 8));
        }
    }
}

/* =============================================================================
 * IMPLEMENTACIÓN ZIP SIMPLE (Sin compresión para simplificar)
 * Formato ZIP básico compatible con estándares
 * =============================================================================
 */

#pragma pack(push, 1)
typedef struct {
    uint32_t signature;           /* 0x04034b50 */
    uint16_t version_needed;
    uint16_t flags;
    uint16_t compression;
    uint16_t mod_time;
    uint16_t mod_date;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t filename_length;
    uint16_t extra_length;
} zip_local_header_t;

typedef struct {
    uint32_t signature;           /* 0x02014b50 */
    uint16_t version_made;
    uint16_t version_needed;
    uint16_t flags;
    uint16_t compression;
    uint16_t mod_time;
    uint16_t mod_date;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t filename_length;
    uint16_t extra_length;
    uint16_t comment_length;
    uint16_t disk_start;
    uint16_t internal_attr;
    uint32_t external_attr;
    uint32_t header_offset;
} zip_central_header_t;

typedef struct {
    uint32_t signature;           /* 0x06054b50 */
    uint16_t disk_number;
    uint16_t central_dir_disk;
    uint16_t entries_this_disk;
    uint16_t total_entries;
    uint32_t central_dir_size;
    uint32_t central_dir_offset;
    uint16_t comment_length;
} zip_end_record_t;
#pragma pack(pop)

static uint32_t crc32_table[256];
static int crc32_initialized = 0;

static void init_crc32_table(void) {
    if (crc32_initialized) return;
    
    for (int i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int j = 0; j < 8; j++) {
            c = (c & 1) ? (0xEDB88320 ^ (c >> 1)) : (c >> 1);
        }
        crc32_table[i] = c;
    }
    crc32_initialized = 1;
}

static uint32_t calculate_crc32(const uint8_t *data, size_t len) {
    init_crc32_table();
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) {
        crc = crc32_table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFF;
}

static void dos_datetime(uint16_t *date, uint16_t *time_out) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    *date = ((t->tm_year - 80) << 9) | ((t->tm_mon + 1) << 5) | t->tm_mday;
    *time_out = (t->tm_hour << 11) | (t->tm_min << 5) | (t->tm_sec / 2);
}

/* =============================================================================
 * UTILIDADES DE ARCHIVO
 * =============================================================================
 */

static bool es_ruta_relativa(const char *ruta, const char *base) {
    size_t base_len = strlen(base);
    if (strncmp(ruta, base, base_len) == 0) {
        const char *rel = ruta + base_len;
        while (*rel == '/' || *rel == '\\') rel++;
        return *rel != '\0';
    }
    return false;
}

static void normalizar_separadores(char *ruta) {
    for (char *p = ruta; *p; p++) {
        if (*p == '\\') *p = '/';
    }
}

static int agregar_archivo_a_zip(FILE *zip, const char *ruta_archivo, 
                                  const char *nombre_en_zip, uint32_t *offset) {
    FILE *f = fopen(ruta_archivo, "rb");
    if (!f) {
        fprintf(stderr, "[ERROR] No se pudo abrir archivo: %s\n", ruta_archivo);
        return JPM_ERROR_ARCHIVO;
    }

    /* Obtener tamaño del archivo */
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size < 0) {
        fclose(f);
        return JPM_ERROR_ARCHIVO;
    }

    /* Leer contenido */
    uint8_t *data = (uint8_t*)malloc(size);
    if (!data) {
        fclose(f);
        return JPM_ERROR_MEMORIA;
    }

    if (fread(data, 1, size, f) != (size_t)size) {
        free(data);
        fclose(f);
        return JPM_ERROR_ARCHIVO;
    }
    fclose(f);

    /* Calcular CRC32 */
    uint32_t crc = calculate_crc32(data, size);

    /* Crear header local */
    zip_local_header_t header = {0};
    header.signature = 0x04034b50;
    header.version_needed = 20;
    header.flags = 0;
    header.compression = 0;  /* Sin compresión */
    dos_datetime(&header.mod_date, &header.mod_time);
    header.crc32 = crc;
    header.compressed_size = size;
    header.uncompressed_size = size;
    header.filename_length = strlen(nombre_en_zip);
    header.extra_length = 0;

    /* Guardar offset actual */
    *offset = ftell(zip);

    /* Escribir header */
    fwrite(&header, sizeof(header), 1, zip);
    fwrite(nombre_en_zip, 1, header.filename_length, zip);

    /* Escribir datos */
    fwrite(data, 1, size, zip);

    free(data);
    return JPM_EXITO;
}

typedef struct archivo_info_s {
    char nombre[JPM_MAX_RUTA];
    char ruta_completa[JPM_MAX_RUTA];
    uint32_t offset;
    uint32_t size;
    uint32_t crc;
    struct archivo_info_s *siguiente;
} archivo_info_t;

static int agregar_directorio_recursivo(const char *dir_base, const char *dir_actual,
                                        char **patrones_ignorar, size_t num_patrones,
                                        archivo_info_t **lista_archivos) {
    char ruta_completa[JPM_MAX_RUTA];
    snprintf(ruta_completa, sizeof(ruta_completa), "%s", dir_actual);

#ifdef _WIN32
    WIN32_FIND_DATAA find_data;
    char patron[JPM_MAX_RUTA];
    snprintf(patron, sizeof(patron), "%s\\*", ruta_completa);
    
    HANDLE hFind = FindFirstFileA(patron, &find_data);
    if (hFind == INVALID_HANDLE_VALUE) {
        return JPM_ERROR_ARCHIVO;
    }

    do {
        if (strcmp(find_data.cFileName, ".") == 0 || 
            strcmp(find_data.cFileName, "..") == 0) {
            continue;
        }

        char ruta_item[JPM_MAX_RUTA];
        snprintf(ruta_item, sizeof(ruta_item), "%s\\%s", ruta_completa, find_data.cFileName);

        /* Verificar si debe ignorarse */
        if (jpm_debe_ignorar(find_data.cFileName, patrones_ignorar, num_patrones)) {
            continue;
        }

        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            agregar_directorio_recursivo(dir_base, ruta_item, patrones_ignorar, 
                                        num_patrones, lista_archivos);
        } else {
            /* Agregar archivo a la lista */
            archivo_info_t *info = (archivo_info_t*)malloc(sizeof(archivo_info_t));
            if (info) {
                strncpy(info->ruta_completa, ruta_item, JPM_MAX_RUTA - 1);
                
                /* Nombre relativo */
                const char *rel = ruta_item + strlen(dir_base);
                while (*rel == '/' || *rel == '\\') rel++;
                strncpy(info->nombre, rel, JPM_MAX_RUTA - 1);
                normalizar_separadores(info->nombre);
                
                info->siguiente = *lista_archivos;
                *lista_archivos = info;
            }
        }
    } while (FindNextFileA(hFind, &find_data));

    FindClose(hFind);
#else
    DIR *d = opendir(ruta_completa);
    if (!d) {
        return JPM_ERROR_ARCHIVO;
    }

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char ruta_item[JPM_MAX_RUTA];
        snprintf(ruta_item, sizeof(ruta_item), "%s/%s", ruta_completa, entry->d_name);

        /* Verificar si debe ignorarse */
        if (jpm_debe_ignorar(entry->d_name, patrones_ignorar, num_patrones)) {
            continue;
        }

        struct stat st;
        if (stat(ruta_item, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                agregar_directorio_recursivo(dir_base, ruta_item, patrones_ignorar,
                                            num_patrones, lista_archivos);
            } else {
                archivo_info_t *info = (archivo_info_t*)malloc(sizeof(archivo_info_t));
                if (info) {
                    strncpy(info->ruta_completa, ruta_item, JPM_MAX_RUTA - 1);
                    
                    const char *rel = ruta_item + strlen(dir_base);
                    while (*rel == '/') rel++;
                    strncpy(info->nombre, rel, JPM_MAX_RUTA - 1);
                    normalizar_separadores(info->nombre);
                    
                    info->siguiente = *lista_archivos;
                    *lista_archivos = info;
                }
            }
        }
    }
    closedir(d);
#endif

    return JPM_EXITO;
}

/* =============================================================================
 * FUNCIONES PÚBLICAS
 * =============================================================================
 */

int jpm_empaquetar(const char *directorio, const char *archivo_salida) {
    if (!directorio || !archivo_salida) {
        fprintf(stderr, "[ERROR] Parámetros inválidos para empaquetado\n");
        return JPM_ERROR_VALIDACION;
    }

    /* Verificar que el directorio existe */
    struct stat st;
    if (stat(directorio, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "[ERROR] Directorio no existe: %s\n", directorio);
        return JPM_ERROR_ARCHIVO;
    }

    printf("[INFO] Empaquetando desde: %s\n", directorio);
    printf("[INFO] Archivo de salida: %s\n", archivo_salida);

    /* Leer patrones de ignorar */
    char ruta_ignorar[JPM_MAX_RUTA];
    snprintf(ruta_ignorar, sizeof(ruta_ignorar), "%s%s%s", directorio, PATH_SEP, JPM_ARCHIVO_IGNORAR);
    
    size_t num_patrones = 0;
    char **patrones = jpm_leer_archivo_ignorar(ruta_ignorar, &num_patrones);

    /* Recolectar archivos */
    archivo_info_t *lista_archivos = NULL;
    agregar_directorio_recursivo(directorio, directorio, patrones, num_patrones, &lista_archivos);

    /* Crear archivo ZIP */
    FILE *zip = fopen(archivo_salida, "wb");
    if (!zip) {
        fprintf(stderr, "[ERROR] No se pudo crear archivo: %s\n", archivo_salida);
        if (patrones) {
            for (size_t i = 0; i < num_patrones; i++) free(patrones[i]);
            free(patrones);
        }
        return JPM_ERROR_ARCHIVO;
    }

    /* Agregar archivos al ZIP y recolectar información */
    archivo_info_t *current = lista_archivos;
    int num_archivos = 0;
    
    while (current) {
        uint32_t offset = 0;
        FILE *f = fopen(current->ruta_completa, "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            current->size = ftell(f);
            fseek(f, 0, SEEK_SET);

            uint8_t *data = (uint8_t*)malloc(current->size);
            if (data) {
                fread(data, 1, current->size, f);
                current->crc = calculate_crc32(data, current->size);
                
                /* Escribir header local */
                zip_local_header_t header = {0};
                header.signature = 0x04034b50;
                header.version_needed = 20;
                header.compression = 0;
                dos_datetime(&header.mod_date, &header.mod_time);
                header.crc32 = current->crc;
                header.compressed_size = current->size;
                header.uncompressed_size = current->size;
                header.filename_length = strlen(current->nombre);
                
                current->offset = ftell(zip);
                fwrite(&header, sizeof(header), 1, zip);
                fwrite(current->nombre, 1, header.filename_length, zip);
                fwrite(data, 1, current->size, zip);
                
                free(data);
                num_archivos++;
                printf("  + %s (%u bytes)\n", current->nombre, current->size);
            }
            fclose(f);
        }
        current = current->siguiente;
    }

    /* Escribir directorio central */
    uint32_t central_offset = ftell(zip);
    current = lista_archivos;
    
    while (current) {
        zip_central_header_t central = {0};
        central.signature = 0x02014b50;
        central.version_made = 20;
        central.version_needed = 20;
        central.compression = 0;
        dos_datetime(&central.mod_date, &central.mod_time);
        central.crc32 = current->crc;
        central.compressed_size = current->size;
        central.uncompressed_size = current->size;
        central.filename_length = strlen(current->nombre);
        central.external_attr = 0x20;  /* Archive attribute */
        central.header_offset = current->offset;
        
        fwrite(&central, sizeof(central), 1, zip);
        fwrite(current->nombre, 1, central.filename_length, zip);
        
        current = current->siguiente;
    }

    uint32_t central_size = ftell(zip) - central_offset;

    /* Escribir registro final */
    zip_end_record_t end_record = {0};
    end_record.signature = 0x06054b50;
    end_record.entries_this_disk = num_archivos;
    end_record.total_entries = num_archivos;
    end_record.central_dir_size = central_size;
    end_record.central_dir_offset = central_offset;
    
    fwrite(&end_record, sizeof(end_record), 1, zip);
    fclose(zip);

    printf("[EXITO] Empaquetados %d archivos en %s\n", num_archivos, archivo_salida);

    /* Liberar memoria */
    while (lista_archivos) {
        archivo_info_t *next = lista_archivos->siguiente;
        free(lista_archivos);
        lista_archivos = next;
    }

    if (patrones) {
        for (size_t i = 0; i < num_patrones; i++) free(patrones[i]);
        free(patrones);
    }

    return JPM_EXITO;
}

int jpm_extraer(const char *archivo_jpkg, const char *directorio_destino) {
    if (!archivo_jpkg || !directorio_destino) {
        fprintf(stderr, "[ERROR] Parámetros inválidos para extracción\n");
        return JPM_ERROR_VALIDACION;
    }

    printf("[INFO] Extrayendo: %s\n", archivo_jpkg);
    printf("[INFO] Destino: %s\n", directorio_destino);

    FILE *zip = fopen(archivo_jpkg, "rb");
    if (!zip) {
        fprintf(stderr, "[ERROR] No se pudo abrir archivo: %s\n", archivo_jpkg);
        return JPM_ERROR_ARCHIVO;
    }

    /* Crear directorio destino si no existe */
    if (jpm_crear_directorio_recursivo(directorio_destino) != JPM_EXITO) {
        fprintf(stderr, "[ERROR] No se pudo crear directorio destino: %s\n", directorio_destino);
        fclose(zip);
        return JPM_ERROR_ARCHIVO;
    }

    /* Buscar el registro final (end of central directory) */
    fseek(zip, -((long)sizeof(zip_end_record_t)), SEEK_END);
    
    zip_end_record_t end_record;
    if (fread(&end_record, sizeof(end_record), 1, zip) != 1 ||
        end_record.signature != 0x06054b50) {
        fprintf(stderr, "[ERROR] Archivo ZIP inválido o corrupto\n");
        fclose(zip);
        return JPM_ERROR_ARCHIVO;
    }

    /* Ir al directorio central */
    fseek(zip, end_record.central_dir_offset, SEEK_SET);

    int num_extraidos = 0;

    /* Leer headers centrales y extraer archivos */
    for (int i = 0; i < end_record.total_entries; i++) {
        zip_central_header_t central;
        if (fread(&central, sizeof(central), 1, zip) != 1 ||
            central.signature != 0x02014b50) {
            fprintf(stderr, "[ERROR] Header central inválido\n");
            break;
        }

        /* Leer nombre del archivo */
        char nombre[JPM_MAX_RUTA];
        if (central.filename_length >= JPM_MAX_RUTA) {
            fprintf(stderr, "[ERROR] Nombre de archivo demasiado largo\n");
            fseek(zip, central.filename_length + central.extra_length + central.comment_length, SEEK_CUR);
            continue;
        }

        fread(nombre, 1, central.filename_length, zip);
        nombre[central.filename_length] = '\0';

        /* Saltar extra y comentario */
        fseek(zip, central.extra_length + central.comment_length, SEEK_CUR);

        /* Guardar posición actual */
        long pos_actual = ftell(zip);

        /* Ir a los datos del archivo */
        fseek(zip, central.header_offset, SEEK_SET);

        zip_local_header_t local;
        if (fread(&local, sizeof(local), 1, zip) != 1 ||
            local.signature != 0x04034b50) {
            fprintf(stderr, "[ERROR] Header local inválido para %s\n", nombre);
            fseek(zip, pos_actual, SEEK_SET);
            continue;
        }

        /* Saltar nombre y extra del header local */
        fseek(zip, local.filename_length + local.extra_length, SEEK_CUR);

        /* Leer datos del archivo */
        uint8_t *data = (uint8_t*)malloc(central.uncompressed_size);
        if (!data) {
            fprintf(stderr, "[ERROR] Fallo de memoria al extraer %s\n", nombre);
            fseek(zip, pos_actual, SEEK_SET);
            continue;
        }

        if (fread(data, 1, central.uncompressed_size, zip) != central.uncompressed_size) {
            fprintf(stderr, "[ERROR] Fallo al leer datos de %s\n", nombre);
            free(data);
            fseek(zip, pos_actual, SEEK_SET);
            continue;
        }

        /* Verificar CRC32 */
        uint32_t crc_calculado = calculate_crc32(data, central.uncompressed_size);
        if (crc_calculado != central.crc32) {
            fprintf(stderr, "[AVISO] CRC32 no coincide para %s\n", nombre);
        }

        /* Crear ruta completa */
        char ruta_destino[JPM_MAX_RUTA];
        snprintf(ruta_destino, sizeof(ruta_destino), "%s%s%s", 
                 directorio_destino, PATH_SEP, nombre);

        /* Normalizar separadores de ruta */
        for (char *p = ruta_destino; *p; p++) {
            if (*p == '/') *p = PATH_SEP[0];
        }

        /* Crear directorios intermedios */
        char *ultimo_sep = strrchr(ruta_destino, PATH_SEP[0]);
        if (ultimo_sep) {
            *ultimo_sep = '\0';
            if (jpm_crear_directorio_recursivo(ruta_destino) != JPM_EXITO) {
                fprintf(stderr, "[ERROR] No se pudo crear directorio: %s\n", ruta_destino);
                free(data);
                fseek(zip, pos_actual, SEEK_SET);
                continue;
            }
            *ultimo_sep = PATH_SEP[0];
        }

        /* Escribir archivo */
        FILE *f_out = fopen(ruta_destino, "wb");
        if (f_out) {
            size_t escritos = fwrite(data, 1, central.uncompressed_size, f_out);
            fclose(f_out);
            if (escritos != central.uncompressed_size) {
                fprintf(stderr, "[ERROR] Fallo al escribir archivo: %s (escritos %zu de %u)\n", 
                        ruta_destino, escritos, central.uncompressed_size);
                free(data);
                fseek(zip, pos_actual, SEEK_SET);
                continue;
            }
            printf("  + %s (%u bytes)\n", nombre, central.uncompressed_size);
            num_extraidos++;
        } else {
            fprintf(stderr, "[ERROR] No se pudo crear archivo: %s (errno=%d)\n", ruta_destino, errno);
            free(data);
            fseek(zip, pos_actual, SEEK_SET);
            continue;
        }

        free(data);

        /* Volver al directorio central */
        fseek(zip, pos_actual, SEEK_SET);
    }

    fclose(zip);

    printf("[EXITO] Extraídos %d archivos\n", num_extraidos);
    return JPM_EXITO;
}

int jpm_calcular_hash(const char *archivo, char *hash_salida, size_t hash_len) {
    if (!archivo || !hash_salida || hash_len < 129) {
        fprintf(stderr, "[ERROR] Parámetros inválidos para cálculo de hash\n");
        return JPM_ERROR_VALIDACION;
    }

    FILE *f = fopen(archivo, "rb");
    if (!f) {
        fprintf(stderr, "[ERROR] No se pudo abrir archivo para hash: %s\n", archivo);
        return JPM_ERROR_ARCHIVO;
    }

    sha512_ctx_t ctx;
    sha512_init(&ctx);

    uint8_t buffer[8192];
    size_t bytes_leidos;

    while ((bytes_leidos = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        sha512_update(&ctx, buffer, bytes_leidos);
    }

    uint8_t digest[64];
    sha512_final(digest, &ctx);

    fclose(f);

    /* Convertir a hexadecimal */
    for (int i = 0; i < 64; i++) {
        snprintf(hash_salida + (i * 2), 3, "%02x", digest[i]);
    }
    hash_salida[128] = '\0';

    return JPM_EXITO;
}

bool jpm_verificar_integridad(const char *archivo, const char *hash_esperado) {
    if (!archivo || !hash_esperado) {
        fprintf(stderr, "[ERROR] Parámetros inválidos para verificación\n");
        return false;
    }

    char hash_calculado[129];
    if (jpm_calcular_hash(archivo, hash_calculado, sizeof(hash_calculado)) != JPM_EXITO) {
        return false;
    }

    /* Comparar hashes (case-insensitive) */
    for (int i = 0; i < 128; i++) {
        char c1 = hash_calculado[i];
        char c2 = hash_esperado[i];
        
        /* Convertir a minúsculas */
        if (c1 >= 'A' && c1 <= 'F') c1 += 32;
        if (c2 >= 'A' && c2 <= 'F') c2 += 32;
        
        if (c1 != c2) {
            fprintf(stderr, "[ERROR] Hash no coincide\n");
            fprintf(stderr, "  Esperado: %s\n", hash_esperado);
            fprintf(stderr, "  Calculado: %s\n", hash_calculado);
            return false;
        }
    }

    printf("[EXITO] Hash verificado correctamente\n");
    return true;
}