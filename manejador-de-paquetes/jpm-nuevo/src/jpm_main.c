/*
 * JPM - Jasboot Package Manager
 * Archivo Principal: jpm_main.c
 * 
 * Sistema de gestión de paquetes para Jasboot
 * Copyright (c) 2024 Jasboot Team
 */

#include "../include/jpm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Imprime la versión del JPM */
static void imprimir_version(void) {
    printf("jpm version %s\n", JPM_VERSION);
    printf("Jasboot Package Manager - Sistema de gestión de paquetes\n");
}

/* Imprime la ayuda general */
static void imprimir_ayuda(void) {
    printf("JPM - Jasboot Package Manager v%s\n\n", JPM_VERSION);
    printf("Uso: jpm <comando> [opciones] [argumentos]\n\n");
    
    printf("Comandos principales:\n");
    printf("  init <nombre>           Inicializa un nuevo paquete\n");
    printf("  pack [directorio]       Empaqueta el proyecto actual en .jpkg\n");
    printf("  install [fuente]        Instala paquete(s)\n");
    printf("  uninstall <paquete>     Desinstala un paquete\n");
    printf("  list                    Lista paquetes instalados\n");
    printf("  info <paquete>          Muestra información de un paquete\n");
    printf("  search <término>        Busca paquetes en el registro\n");
    printf("  update [paquete]        Actualiza paquete(s)\n");
    printf("  publish                 Publica paquete al registro\n");
    printf("  clean                   Limpia caché y temporales\n");
    printf("  run <script>            Ejecuta un script definido\n");
    printf("\n");
    
    printf("Comandos de información:\n");
    printf("  version                 Muestra la versión de jpm\n");
    printf("  help                    Muestra esta ayuda\n");
    printf("\n");
    
    printf("Opciones globales:\n");
    printf("  -v, --verbose           Modo verboso\n");
    printf("  -s, --silent            Modo silencioso\n");
    printf("  -g, --global            Instalación global\n");
    printf("  -f, --force             Forzar operación\n");
    printf("  --dry-run               Simular sin ejecutar\n");
    printf("\n");
    
    printf("Ejemplos:\n");
    printf("  jpm init mi-paquete\n");
    printf("  jpm install paquete.jpkg\n");
    printf("  jpm install https://example.com/paquete.jpkg\n");
    printf("  jpm install\n");
    printf("  jpm publish\n");
    printf("  jpm search neuronal\n");
    printf("\n");
    
    printf("Documentación completa: https://jasboot.org/docs/jpm\n");
}

/* Parsea las opciones globales */
static int parsear_opciones_globales(jpm_config_t *config, int *argc, char ***argv) {
    int i = 1;
    int argc_nuevo = 1;
    
    while (i < *argc) {
        if (strcmp((*argv)[i], "-v") == 0 || strcmp((*argv)[i], "--verbose") == 0) {
            config->modo_verboso = true;
            i++;
        } else if (strcmp((*argv)[i], "-s") == 0 || strcmp((*argv)[i], "--silent") == 0) {
            config->modo_silencioso = true;
            i++;
        } else if (strcmp((*argv)[i], "-g") == 0 || strcmp((*argv)[i], "--global") == 0) {
            config->es_global = true;
            i++;
        } else if (strcmp((*argv)[i], "-f") == 0 || strcmp((*argv)[i], "--force") == 0) {
            config->forzar = true;
            i++;
        } else if (strcmp((*argv)[i], "--dry-run") == 0) {
            config->simular = true;
            i++;
        } else {
            /* No es una opción global, mantenerla */
            (*argv)[argc_nuevo++] = (*argv)[i++];
        }
    }
    
    *argc = argc_nuevo;
    return 0;
}

/* Función principal */
int main(int argc, char *argv[]) {
    int resultado = JPM_EXITO;
    jpm_contexto_t *ctx = NULL;
    
    /* Sin argumentos, mostrar ayuda */
    if (argc < 2) {
        imprimir_ayuda();
        return JPM_EXITO;
    }
    
    /* Inicializar contexto */
    ctx = jpm_inicializar();
    if (!ctx) {
        fprintf(stderr, "[ERROR] No se pudo inicializar el contexto de JPM\n");
        return JPM_ERROR_MEMORIA;
    }
    
    /* Parsear opciones globales */
    parsear_opciones_globales(&ctx->config, &argc, &argv);
    
    /* Comando */
    const char *comando = argv[1];
    
    /* Procesar comandos */
    if (strcmp(comando, "version") == 0 || strcmp(comando, "-v") == 0 || strcmp(comando, "--version") == 0) {
        imprimir_version();
        
    } else if (strcmp(comando, "help") == 0 || strcmp(comando, "-h") == 0 || strcmp(comando, "--help") == 0) {
        imprimir_ayuda();
        
    } else if (strcmp(comando, "init") == 0) {
        if (argc < 3) {
            fprintf(stderr, "[ERROR] Uso: jpm init <nombre>\n");
            resultado = JPM_ERROR_VALIDACION;
        } else {
            const char *nombre = argv[2];
            resultado = jpm_cmd_init(ctx, nombre);
        }
        
    } else if (strcmp(comando, "pack") == 0) {
        const char *directorio = (argc >= 3) ? argv[2] : ".";
        resultado = jpm_cmd_pack(ctx, directorio);
        
    } else if (strcmp(comando, "install") == 0) {
        if (argc >= 3) {
            /* Instalar paquete específico */
            const char *fuente = argv[2];
            resultado = jpm_cmd_install(ctx, fuente);
        } else {
            /* Instalar todas las dependencias */
            resultado = jpm_cmd_install_todas(ctx);
        }
        
    } else if (strcmp(comando, "uninstall") == 0 || strcmp(comando, "remove") == 0) {
        if (argc < 3) {
            fprintf(stderr, "[ERROR] Uso: jpm uninstall <paquete>\n");
            resultado = JPM_ERROR_VALIDACION;
        } else {
            const char *nombre = argv[2];
            resultado = jpm_cmd_uninstall(ctx, nombre);
        }
        
    } else if (strcmp(comando, "list") == 0 || strcmp(comando, "ls") == 0) {
        resultado = jpm_cmd_list(ctx);
        
    } else if (strcmp(comando, "info") == 0 || strcmp(comando, "show") == 0) {
        if (argc < 3) {
            fprintf(stderr, "[ERROR] Uso: jpm info <paquete>\n");
            resultado = JPM_ERROR_VALIDACION;
        } else {
            const char *nombre = argv[2];
            resultado = jpm_cmd_info(ctx, nombre);
        }
        
    } else if (strcmp(comando, "search") == 0 || strcmp(comando, "find") == 0) {
        if (argc < 3) {
            fprintf(stderr, "[ERROR] Uso: jpm search <término>\n");
            resultado = JPM_ERROR_VALIDACION;
        } else {
            const char *termino = argv[2];
            resultado = jpm_cmd_search(ctx, termino);
        }
        
    } else if (strcmp(comando, "publish") == 0) {
        resultado = jpm_cmd_publish(ctx);
        
    } else if (strcmp(comando, "update") == 0 || strcmp(comando, "upgrade") == 0) {
        const char *nombre = (argc >= 3) ? argv[2] : NULL;
        resultado = jpm_cmd_update(ctx, nombre);
        
    } else if (strcmp(comando, "clean") == 0) {
        resultado = jpm_cmd_clean(ctx);
        
    } else if (strcmp(comando, "run") == 0) {
        if (argc < 3) {
            fprintf(stderr, "[ERROR] Uso: jpm run <script> [args...]\n");
            resultado = JPM_ERROR_VALIDACION;
        } else {
            const char *script = argv[2];
            int script_argc = argc - 3;
            char **script_argv = (argc > 3) ? &argv[3] : NULL;
            resultado = jpm_cmd_run(ctx, script, script_argc, script_argv);
        }
        
    } else {
        fprintf(stderr, "[ERROR] Comando desconocido: %s\n", comando);
        fprintf(stderr, "Ejecuta 'jpm help' para ver los comandos disponibles.\n");
        resultado = JPM_ERROR_VALIDACION;
    }
    
    /* Liberar contexto */
    jpm_liberar(ctx);
    
    return resultado;
}