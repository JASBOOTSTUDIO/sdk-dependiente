/* Fusión mínima de `usar` para jbc-to-c (AOT): funciones exportadas + usar anidado en modulos. */
#if !defined(_WIN32) && !defined(_DEFAULT_SOURCE)
#define _DEFAULT_SOURCE
#endif

#include "aot_usar_merge.h"
#include "lexer.h"
#include "parser.h"
#include "token_vec.h"
#include "nodes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define PATH_SEP '\\'
#else
#include <unistd.h>
#include <limits.h>
#define PATH_SEP '/'
#endif

#define AOT_RED "\x1b[31m"
#define AOT_RESET "\x1b[0m"

static void path_dirname(const char *input_file, char *out, size_t size) {
    if (!input_file || !out || size == 0) return;
    size_t n = strlen(input_file);
    if (n + 1 > size) n = size - 1;
    memcpy(out, input_file, n + 1);
    char *ls = strrchr(out, '/');
    char *ls2 = strrchr(out, '\\');
    char *last = NULL;
    if (ls && ls2)
        last = (ls > ls2) ? ls : ls2;
    else
        last = ls ? ls : ls2;
    if (last)
        *last = '\0';
    else {
        out[0] = '.';
        out[1] = '\0';
    }
}

static int path_is_abs(const char *p) {
    if (!p || !p[0]) return 0;
#ifdef _WIN32
    if (p[0] == '/' || p[0] == '\\') return 1;
    if (p[0] && p[1] == ':') return 1;
    return 0;
#else
    return p[0] == '/';
#endif
}

static void norm_seps(char *s) {
#ifdef _WIN32
    for (; s && *s; s++)
        if (*s == '/') *s = '\\';
#else
    (void)s;
#endif
}

static int path_normalize(char *out, size_t osz, const char *in) {
    if (!out || osz == 0 || !in) return 0;
#ifdef _WIN32
    return _fullpath(out, in, (int)osz) != NULL;
#else
    {
        char *r = realpath(in, NULL);
        if (!r) return 0;
        snprintf(out, osz, "%s", r);
        free(r);
        return 1;
    }
#endif
}

typedef struct {
    char **paths;
    size_t n;
    size_t cap;
} LoadedPaths;

static void loaded_free(LoadedPaths *ld) {
    if (!ld) return;
    for (size_t i = 0; i < ld->n; i++) free(ld->paths[i]);
    free(ld->paths);
    ld->paths = NULL;
    ld->n = ld->cap = 0;
}

static int loaded_has(const LoadedPaths *ld, const char *canon) {
    if (!ld || !canon) return 0;
    for (size_t i = 0; i < ld->n; i++)
        if (ld->paths[i] && strcmp(ld->paths[i], canon) == 0) return 1;
    return 0;
}

static int loaded_add(LoadedPaths *ld, const char *canon) {
    if (!canon) return 0;
    if (loaded_has(ld, canon)) return 0;
    if (ld->n >= ld->cap) {
        size_t nc = ld->cap ? ld->cap * 2 : 8;
        char **p = (char **)realloc(ld->paths, nc * sizeof(char *));
        if (!p) return 1;
        ld->paths = p;
        ld->cap = nc;
    }
    ld->paths[ld->n] = strdup(canon);
    if (!ld->paths[ld->n]) return 1;
    ld->n++;
    return 0;
}

static char *read_file_alloc(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz < 0) {
        fclose(f);
        return NULL;
    }
    char *buf = (char *)malloc((size_t)sz + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    size_t n = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    buf[n] = '\0';
    return buf;
}

static void apply_enviar_block_directives(ProgramNode *mp) {
    if (!mp) return;
    for (size_t j = 0; j < mp->n_globals; j++) {
        ASTNode *g = mp->globals[j];
        if (!g || g->type != NODE_EXPORT_DIRECTIVE) continue;
        ExportDirectiveNode *en = (ExportDirectiveNode *)g;
        for (size_t k = 0; k < en->n_names; k++) {
            const char *ename = en->names[k];
            char clean_name[128];
            size_t start = 0;
            while (ename[start] && (ename[start] == ' ' || ename[start] == '\t' || ename[start] == '\r' || ename[start] == '\n'))
                start++;
            size_t end = strlen(ename);
            while (end > start && (ename[end - 1] == ' ' || ename[end - 1] == '\t' || ename[end - 1] == '\r' || ename[end - 1] == '\n'))
                end--;
            size_t len = end - start;
            if (len >= sizeof(clean_name)) len = sizeof(clean_name) - 1;
            memcpy(clean_name, ename + start, len);
            clean_name[len] = '\0';

            for (size_t fidx = 0; fidx < mp->n_funcs; fidx++) {
                FunctionNode *fn = (FunctionNode *)mp->functions[fidx];
                if (fn && fn->name && strcmp(fn->name, clean_name) == 0) fn->is_exported = 1;
            }
            for (size_t gidx = 0; gidx < mp->n_globals; gidx++) {
                ASTNode *gg = mp->globals[gidx];
                if (gg && gg->type == NODE_VAR_DECL) {
                    VarDeclNode *vd = (VarDeclNode *)gg;
                    if (vd->name && strcmp(vd->name, clean_name) == 0) vd->is_exported = 1;
                } else if (gg && gg->type == NODE_STRUCT_DEF) {
                    StructDefNode *sd = (StructDefNode *)gg;
                    if (sd->name && strcmp(sd->name, clean_name) == 0) sd->is_exported = 1;
                }
            }
        }
    }
}

static int main_has_function_named(const ProgramNode *main, const char *name) {
    if (!main || !name) return 0;
    for (size_t i = 0; i < main->n_funcs; i++) {
        FunctionNode *f = (FunctionNode *)main->functions[i];
        if (f && f->name && strcmp(f->name, name) == 0) return 1;
    }
    return 0;
}

static int main_has_global_var_named(const ProgramNode *main, const char *name) {
    if (!main || !name) return 0;
    for (size_t i = 0; i < main->n_globals; i++) {
        ASTNode *g = main->globals[i];
        if (g && g->type == NODE_VAR_DECL) {
            VarDeclNode *v = (VarDeclNode *)g;
            if (v->name && strcmp(v->name, name) == 0) return 1;
        }
    }
    return 0;
}

static int main_has_struct_named(const ProgramNode *main, const char *name) {
    if (!main || !name) return 0;
    for (size_t i = 0; i < main->n_globals; i++) {
        ASTNode *g = main->globals[i];
        if (g && g->type == NODE_STRUCT_DEF) {
            StructDefNode *s = (StructDefNode *)g;
            if (s->name && strcmp(s->name, name) == 0) return 1;
        }
    }
    return 0;
}

static int validate_named_imports(const ProgramNode *mp, const ActivarModuloNode *spec, const char *diag_path) {
    if (!spec || spec->import_kind != USAR_IMPORT_NAMES) return 0;
    if (spec->n_import_names == 0) {
        fprintf(stderr, "%s%s: error: `usar { ... }` requiere al menos un nombre.%s\n", AOT_RED, diag_path, AOT_RESET);
        return 1;
    }
    for (size_t i = 0; i < spec->n_import_names; i++) {
        const char *want = spec->import_names[i];
        int found_fn = 0;
        for (size_t j = 0; j < mp->n_funcs; j++) {
            FunctionNode *cf = (FunctionNode *)mp->functions[j];
            if (cf && cf->name && strcmp(cf->name, want) == 0 && cf->is_exported) {
                found_fn = 1;
                break;
            }
        }
        int found_var = 0;
        if (!found_fn) {
            for (size_t j = 0; j < mp->n_globals; j++) {
                ASTNode *g = mp->globals[j];
                if (g && g->type == NODE_VAR_DECL) {
                    VarDeclNode *vd = (VarDeclNode *)g;
                    if (vd->name && strcmp(vd->name, want) == 0 && vd->is_exported) {
                        found_var = 1;
                        break;
                    }
                }
            }
        }
        int found_reg = 0;
        int found_clase = 0;
        if (!found_fn && !found_var) {
            for (size_t j = 0; j < mp->n_globals; j++) {
                ASTNode *g = mp->globals[j];
                if (g && g->type == NODE_STRUCT_DEF) {
                    StructDefNode *sd = (StructDefNode *)g;
                    if (!sd->name || strcmp(sd->name, want) != 0 || !sd->is_exported) continue;
                    if (sd->is_clase) found_clase = 1;
                    else found_reg = 1;
                    break;
                }
            }
        }
        if (!found_fn && !found_var && !found_reg && !found_clase) {
            fprintf(stderr,
                    "%s%s: error: `usar { ... }`: no se hallo simbolo exportado `%s` (funcion, variable global o "
                    "registro/clase) en el modulo.%s\n",
                    AOT_RED, diag_path, want, AOT_RESET);
            return 1;
        }
    }
    return 0;
}

static int append_merged_exported_global_vars(ProgramNode *main, ProgramNode *mp, const char *diag_path) {
    /* Misma idea que jas-compiler-c main.c al fusionar modulos: anexar al final de `globals`. */
    for (size_t i = 0; i < mp->n_globals; i++) {
        ASTNode *g = mp->globals[i];
        if (!g || g->type != NODE_VAR_DECL) continue;
        VarDeclNode *v = (VarDeclNode *)g;
        if (!v->is_exported) continue;
        const char *nm = v->name ? v->name : "_g";
        if (main_has_function_named(main, nm) || main_has_global_var_named(main, nm) || main_has_struct_named(main, nm)) {
            fprintf(stderr, "%s%s: error: nombre duplicado tras fusionar modulo: `%s` (funcion, variable o registro).%s\n",
                    AOT_RED, diag_path, nm, AOT_RESET);
            return 1;
        }
        size_t new_n = main->n_globals + 1;
        ASTNode **ng = (ASTNode **)realloc(main->globals, new_n * sizeof(ASTNode *));
        if (!ng) return 1;
        main->globals = ng;
        main->globals[new_n - 1] = g;
        main->n_globals = new_n;
        mp->globals[i] = NULL;
    }
    return 0;
}

static int append_merged_exported_registros(ProgramNode *main, ProgramNode *mp, const char *diag_path) {
    /* Registros exportados (no clase): metadato AST para tipos de usuario; mismo orden que jbc (anexar al final). */
    for (size_t i = 0; i < mp->n_globals; i++) {
        ASTNode *g = mp->globals[i];
        if (!g || g->type != NODE_STRUCT_DEF) continue;
        StructDefNode *sd = (StructDefNode *)g;
        if (!sd->is_exported || sd->is_clase) continue;
        const char *nm = sd->name ? sd->name : "_R";
        if (main_has_function_named(main, nm) || main_has_global_var_named(main, nm) || main_has_struct_named(main, nm)) {
            fprintf(stderr, "%s%s: error: nombre duplicado tras fusionar modulo: `%s` (funcion, variable o registro).%s\n",
                    AOT_RED, diag_path, nm, AOT_RESET);
            return 1;
        }
        size_t new_n = main->n_globals + 1;
        ASTNode **ng = (ASTNode **)realloc(main->globals, new_n * sizeof(ASTNode *));
        if (!ng) return 1;
        main->globals = ng;
        main->globals[new_n - 1] = g;
        main->n_globals = new_n;
        mp->globals[i] = NULL;
    }
    return 0;
}

static int append_merged_exported_clases(ProgramNode *main, ProgramNode *mp, const char *diag_path) {
    /* Clases exportadas: se anexan como STRUCT_DEF con is_clase=1 para resolver tipos/metodos. */
    for (size_t i = 0; i < mp->n_globals; i++) {
        ASTNode *g = mp->globals[i];
        if (!g || g->type != NODE_STRUCT_DEF) continue;
        StructDefNode *sd = (StructDefNode *)g;
        if (!sd->is_exported || !sd->is_clase) continue;
        const char *nm = sd->name ? sd->name : "_C";
        if (main_has_function_named(main, nm) || main_has_global_var_named(main, nm) || main_has_struct_named(main, nm)) {
            fprintf(stderr, "%s%s: error: nombre duplicado tras fusionar modulo: `%s` (funcion, variable o clase).%s\n",
                    AOT_RED, diag_path, nm, AOT_RESET);
            return 1;
        }
        size_t new_n = main->n_globals + 1;
        ASTNode **ng = (ASTNode **)realloc(main->globals, new_n * sizeof(ASTNode *));
        if (!ng) return 1;
        main->globals = ng;
        main->globals[new_n - 1] = g;
        main->n_globals = new_n;
        mp->globals[i] = NULL;
    }
    return 0;
}

static int append_merged_functions(ProgramNode *main, ProgramNode *mp, const char *diag_path) {
    for (size_t i = 0; i < mp->n_funcs; i++) {
        FunctionNode *f = (FunctionNode *)mp->functions[i];
        if (!f || !f->is_exported) continue;
        /* Igual que jbc: se fusionan todas las funciones exportadas del modulo (la lista en `usar { }` solo valida). */
        if (main_has_function_named(main, f->name) || main_has_global_var_named(main, f->name) ||
            main_has_struct_named(main, f->name)) {
            fprintf(stderr, "%s%s: error: nombre duplicado tras fusionar modulo: `%s` (funcion, variable o registro).%s\n",
                    AOT_RED, diag_path, f->name, AOT_RESET);
            return 1;
        }
        size_t new_n = main->n_funcs + 1;
        ASTNode **nf = (ASTNode **)realloc(main->functions, new_n * sizeof(ASTNode *));
        if (!nf) return 1;
        main->functions = nf;
        main->functions[main->n_funcs] = (ASTNode *)f;
        main->n_funcs = new_n;
        mp->functions[i] = NULL;
    }
    return 0;
}

static int parse_program_file(const char *full_path, const char *diag_path, ProgramNode **out_mp) {
    char *mbuf = read_file_alloc(full_path);
    if (!mbuf) {
        fprintf(stderr, "%s%s: error: no se puede leer el modulo `%s`.%s\n", AOT_RED, diag_path, full_path, AOT_RESET);
        return 1;
    }
    Lexer mlex;
    lexer_init(&mlex, mbuf);
    TokenVec mtvec;
    token_vec_init(&mtvec);
    Token mtok;
    while (lexer_next(&mlex, &mtok) == 0) {
        token_vec_push(&mtvec, &mtok);
        token_free_value(&mtok);
        if (mtok.type == TOK_EOF) break;
    }
    lexer_free(&mlex);
    Parser mpar;
    parser_init(&mpar, &mtvec, full_path, mbuf);
    ASTNode *mast = parser_parse(&mpar);
    token_vec_free(&mtvec);
    if (!mast || mpar.last_error) {
        if (mast) ast_free(mast);
        fprintf(stderr, "%s%s: error: modulo invalido `%s`.%s\n", AOT_RED, diag_path, full_path, AOT_RESET);
        if (mpar.last_error && mpar.last_error[0]) fprintf(stderr, "%s%s%s", AOT_RED, mpar.last_error, AOT_RESET);
        parser_free(&mpar);
        free(mbuf);
        return 1;
    }
    parser_free(&mpar);
    free(mbuf);
    if (mast->type != NODE_PROGRAM) {
        ast_free(mast);
        fprintf(stderr, "%s%s: error: el modulo no es un programa valido.%s\n", AOT_RED, diag_path, AOT_RESET);
        return 1;
    }
    *out_mp = (ProgramNode *)mast;
    return 0;
}

static void remove_global_at(ProgramNode *main, size_t ix) {
    if (!main || ix >= main->n_globals) return;
    ast_free(main->globals[ix]);
    for (size_t j = ix + 1; j < main->n_globals; j++) main->globals[j - 1] = main->globals[j];
    main->n_globals--;
    if (main->n_globals == 0) {
        free(main->globals);
        main->globals = NULL;
        return;
    }
    ASTNode **ng = (ASTNode **)realloc(main->globals, main->n_globals * sizeof(ASTNode *));
    if (ng) main->globals = ng;
}

static size_t find_first_usar_global(ProgramNode *main) {
    if (!main) return (size_t)-1;
    for (size_t i = 0; i < main->n_globals; i++) {
        ASTNode *g = main->globals[i];
        if (g && g->type == NODE_ACTIVAR_MODULO) return i;
    }
    return (size_t)-1;
}

static int merge_one_module_recursive(ProgramNode *main, ActivarModuloNode *an, const char *referrer_diag,
                                      const char *base_dir, LoadedPaths *loaded) {
    if (!an->module_path || an->module_path->type != NODE_LITERAL) return 0;
    LiteralNode *ln = (LiteralNode *)an->module_path;
    if (!ln->type_name || strcmp(ln->type_name, "texto") != 0 || !ln->value.str) return 0;
    const char *rel = ln->value.str;
    char full[4096];
    if (path_is_abs(rel))
        snprintf(full, sizeof(full), "%s", rel);
    else
        snprintf(full, sizeof(full), "%s%c%s", base_dir, PATH_SEP, rel);
    norm_seps(full);

    char canon[4096];
    if (!path_normalize(canon, sizeof(canon), full)) snprintf(canon, sizeof(canon), "%s", full);

    if (loaded_has(loaded, canon)) return 0;

    ProgramNode *mp = NULL;
    if (parse_program_file(full, referrer_diag, &mp)) return 1;

    apply_enviar_block_directives(mp);

    char child_base[4096];
    path_dirname(full, child_base, sizeof(child_base));

    for (size_t gi = 0; gi < mp->n_globals; gi++) {
        ASTNode *g = mp->globals[gi];
        if (!g || g->type != NODE_ACTIVAR_MODULO) continue;
        if (merge_one_module_recursive(main, (ActivarModuloNode *)g, referrer_diag, child_base, loaded)) {
            ast_free((ASTNode *)mp);
            return 1;
        }
    }

    if (validate_named_imports(mp, an, referrer_diag)) {
        ast_free((ASTNode *)mp);
        return 1;
    }

    if (append_merged_exported_global_vars(main, mp, referrer_diag)) {
        ast_free((ASTNode *)mp);
        return 1;
    }

    if (append_merged_exported_registros(main, mp, referrer_diag)) {
        ast_free((ASTNode *)mp);
        return 1;
    }

    if (append_merged_exported_clases(main, mp, referrer_diag)) {
        ast_free((ASTNode *)mp);
        return 1;
    }

    if (append_merged_functions(main, mp, referrer_diag)) {
        ast_free((ASTNode *)mp);
        return 1;
    }

    if (loaded_add(loaded, canon)) {
        ast_free((ASTNode *)mp);
        return 1;
    }

    ast_free((ASTNode *)mp);
    return 0;
}

int aot_merge_usar_modules(ASTNode *program, const char *entry_source_path) {
    if (!program || program->type != NODE_PROGRAM || !entry_source_path) return 0;
    ProgramNode *main = (ProgramNode *)program;

    char entry_dir[4096];
    path_dirname(entry_source_path, entry_dir, sizeof(entry_dir));

    LoadedPaths loaded = {0};

    while (1) {
        size_t ix = find_first_usar_global(main);
        if (ix == (size_t)-1) break;
        ASTNode *g = main->globals[ix];
        if (!g || g->type != NODE_ACTIVAR_MODULO) break;
        ActivarModuloNode *an = (ActivarModuloNode *)g;
        if (merge_one_module_recursive(main, an, entry_source_path, entry_dir, &loaded)) {
            loaded_free(&loaded);
            return 1;
        }
        remove_global_at(main, ix);
    }

    loaded_free(&loaded);
    return 0;
}
