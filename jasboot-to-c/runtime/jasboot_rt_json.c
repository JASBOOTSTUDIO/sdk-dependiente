/* JSON AOT: arbol propio alineado con VMJsonKind (vm.h). */
#include "jasboot_rt.h"
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define JB_JSON_BUF 65536

typedef struct jb_json_node jb_json_node_t;

struct jb_json_node {
    uint8_t kind;
    uint8_t jbool;
    int64_t jint;
    double jfloat;
    char *jstr;
    jb_json_node_t **arr;
    uint32_t arr_len;
    uint32_t arr_cap;
    char **keys;
    jb_json_node_t **vals;
    uint32_t obj_len;
    uint32_t obj_cap;
};

static jb_var_t jb_json_wrap_owned(jb_json_node_t *n) {
    jb_var_t v;
    v.type = JB_TYPE_JSON;
    v.u.json = n;
    return v;
}

static void jb_json_free_node(jb_json_node_t *n) {
    uint32_t i;
    if (!n) return;
    free(n->jstr);
    n->jstr = NULL;
    for (i = 0; i < n->arr_len; i++) {
        jb_json_free_node(n->arr[i]);
        n->arr[i] = NULL;
    }
    free(n->arr);
    n->arr = NULL;
    for (i = 0; i < n->obj_len; i++) {
        free(n->keys[i]);
        n->keys[i] = NULL;
        jb_json_free_node(n->vals[i]);
        n->vals[i] = NULL;
    }
    free(n->keys);
    free(n->vals);
    n->keys = NULL;
    n->vals = NULL;
    free(n);
}

void jb_json_free_tree(jb_json_node_t *n) {
    jb_json_free_node(n);
}

static jb_json_node_t *jb_json_new_leaf(uint8_t kind) {
    jb_json_node_t *n = (jb_json_node_t *)calloc(1, sizeof(jb_json_node_t));
    if (!n) return NULL;
    n->kind = kind;
    return n;
}

static int jb_json_arr_push(jb_json_node_t *arrn, jb_json_node_t *item) {
    if (!arrn || !item) return 0;
    if (arrn->arr_len + 1 > arrn->arr_cap) {
        uint32_t nc = arrn->arr_cap ? arrn->arr_cap * 2u : 8u;
        jb_json_node_t **p = (jb_json_node_t **)realloc(arrn->arr, nc * sizeof(*p));
        if (!p) return 0;
        arrn->arr = p;
        arrn->arr_cap = nc;
    }
    arrn->arr[arrn->arr_len++] = item;
    return 1;
}

static int jb_json_obj_put(jb_json_node_t *obj, const char *key, jb_json_node_t *val) {
    char *kcopy;
    char **nk;
    jb_json_node_t **nv;
    uint32_t nc;
    if (!obj || !key || !val) return 0;
    if (obj->obj_len + 1 > obj->obj_cap) {
        nc = obj->obj_cap ? obj->obj_cap * 2u : 8u;
        nk = (char **)realloc(obj->keys, nc * sizeof(char *));
        nv = (jb_json_node_t **)realloc(obj->vals, nc * sizeof(jb_json_node_t *));
        if (!nk || !nv) {
            free(nk);
            free(nv);
            return 0;
        }
        obj->keys = nk;
        obj->vals = nv;
        obj->obj_cap = nc;
    }
    kcopy = strdup(key);
    if (!kcopy) return 0;
    obj->keys[obj->obj_len] = kcopy;
    obj->vals[obj->obj_len] = val;
    obj->obj_len++;
    return 1;
}

typedef struct {
    const char *p;
    char err[160];
} JBJsonParser;

static void jb_jp_err(JBJsonParser *jp, const char *msg) {
    if (!jp || jp->err[0]) return;
    snprintf(jp->err, sizeof jp->err, "%s", msg ? msg : "JSON invalido");
}

static void jb_jp_skip_ws(JBJsonParser *jp) {
    while (jp && jp->p && *jp->p && isspace((unsigned char)*jp->p)) jp->p++;
}

static char *jb_jp_parse_string_raw(JBJsonParser *jp) {
    size_t cap = 32, len = 0;
    char *out;
    if (!jp || *jp->p != '"') return NULL;
    jp->p++;
    out = (char *)malloc(cap);
    if (!out) return NULL;
    while (*jp->p && *jp->p != '"') {
        unsigned char ch = (unsigned char)*jp->p++;
        if (ch == '\\') {
            unsigned char esc = (unsigned char)*jp->p++;
            switch (esc) {
                case '"': ch = '"'; break;
                case '\\': ch = '\\'; break;
                case '/': ch = '/'; break;
                case 'b': ch = '\b'; break;
                case 'f': ch = '\f'; break;
                case 'n': ch = '\n'; break;
                case 'r': ch = '\r'; break;
                case 't': ch = '\t'; break;
                case 'u':
                    for (int i = 0; i < 4 && isxdigit((unsigned char)*jp->p); i++) jp->p++;
                    ch = '?';
                    break;
                default:
                    free(out);
                    jb_jp_err(jp, "Escape JSON invalido");
                    return NULL;
            }
        }
        if (len + 2 > cap) {
            cap *= 2u;
            out = (char *)realloc(out, cap);
            if (!out) return NULL;
        }
        out[len++] = (char)ch;
    }
    if (*jp->p != '"') {
        free(out);
        jb_jp_err(jp, "Cadena JSON sin cierre");
        return NULL;
    }
    jp->p++;
    out[len] = '\0';
    return out;
}

static jb_json_node_t *jb_jp_parse_value(JBJsonParser *jp);

static jb_json_node_t *jb_jp_parse_number(JBJsonParser *jp) {
    const char *start;
    char *end = NULL;
    double d;
    int is_float = 0;
    jb_json_node_t *n;
    jb_jp_skip_ws(jp);
    start = jp->p;
    while (*jp->p && strchr("-+0123456789.eE", *jp->p)) {
        if (*jp->p == '.' || *jp->p == 'e' || *jp->p == 'E') is_float = 1;
        jp->p++;
    }
    errno = 0;
    d = strtod(start, &end);
    if (end == start || errno == ERANGE) {
        jb_jp_err(jp, "Numero JSON invalido");
        return NULL;
    }
    jp->p = end;
    n = jb_json_new_leaf(is_float ? JB_JSON_FLOAT : JB_JSON_INT);
    if (!n) return NULL;
    if (is_float) n->jfloat = d;
    else n->jint = (int64_t)d;
    return n;
}

static jb_json_node_t *jb_jp_parse_array(JBJsonParser *jp) {
    jb_json_node_t *h;
    if (*jp->p != '[') return NULL;
    jp->p++;
    h = jb_json_new_leaf(JB_JSON_ARRAY);
    if (!h) return NULL;
    jb_jp_skip_ws(jp);
    if (*jp->p == ']') {
        jp->p++;
        return h;
    }
    while (*jp->p) {
        jb_json_node_t *item;
        jb_jp_skip_ws(jp);
        item = jb_jp_parse_value(jp);
        if (!item) {
            jb_json_free_node(h);
            return NULL;
        }
        if (!jb_json_arr_push(h, item)) {
            jb_json_free_node(item);
            jb_json_free_node(h);
            jb_jp_err(jp, "Sin memoria para array JSON");
            return NULL;
        }
        jb_jp_skip_ws(jp);
        if (*jp->p == ']') {
            jp->p++;
            return h;
        }
        if (*jp->p != ',') {
            jb_json_free_node(h);
            jb_jp_err(jp, "Se esperaba ',' en array JSON");
            return NULL;
        }
        jp->p++;
    }
    jb_json_free_node(h);
    jb_jp_err(jp, "Array JSON sin cierre");
    return NULL;
}

static jb_json_node_t *jb_jp_parse_object(JBJsonParser *jp) {
    jb_json_node_t *h;
    if (*jp->p != '{') return NULL;
    jp->p++;
    h = jb_json_new_leaf(JB_JSON_OBJECT);
    if (!h) return NULL;
    jb_jp_skip_ws(jp);
    if (*jp->p == '}') {
        jp->p++;
        return h;
    }
    while (*jp->p) {
        char *key_raw;
        jb_json_node_t *val;
        jb_jp_skip_ws(jp);
        if (*jp->p != '"') {
            jb_json_free_node(h);
            jb_jp_err(jp, "Clave JSON invalida");
            return NULL;
        }
        key_raw = jb_jp_parse_string_raw(jp);
        if (!key_raw) {
            jb_json_free_node(h);
            return NULL;
        }
        jb_jp_skip_ws(jp);
        if (*jp->p != ':') {
            free(key_raw);
            jb_json_free_node(h);
            jb_jp_err(jp, "Se esperaba ':' en objeto JSON");
            return NULL;
        }
        jp->p++;
        jb_jp_skip_ws(jp);
        val = jb_jp_parse_value(jp);
        if (!val) {
            free(key_raw);
            jb_json_free_node(h);
            return NULL;
        }
        if (!jb_json_obj_put(h, key_raw, val)) {
            free(key_raw);
            jb_json_free_node(val);
            jb_json_free_node(h);
            jb_jp_err(jp, "Sin memoria para objeto JSON");
            return NULL;
        }
        free(key_raw);
        jb_jp_skip_ws(jp);
        if (*jp->p == '}') {
            jp->p++;
            return h;
        }
        if (*jp->p != ',') {
            jb_json_free_node(h);
            jb_jp_err(jp, "Se esperaba ',' en objeto JSON");
            return NULL;
        }
        jp->p++;
    }
    jb_json_free_node(h);
    jb_jp_err(jp, "Objeto JSON sin cierre");
    return NULL;
}

static jb_json_node_t *jb_jp_parse_value(JBJsonParser *jp) {
    jb_jp_skip_ws(jp);
    if (!jp || !*jp->p) {
        jb_jp_err(jp, "JSON vacio");
        return NULL;
    }
    if (*jp->p == '"') {
        char *raw = jb_jp_parse_string_raw(jp);
        jb_json_node_t *h;
        if (!raw) return NULL;
        h = jb_json_new_leaf(JB_JSON_STRING);
        if (!h) {
            free(raw);
            return NULL;
        }
        h->jstr = raw;
        return h;
    }
    if (*jp->p == '{') return jb_jp_parse_object(jp);
    if (*jp->p == '[') return jb_jp_parse_array(jp);
    if (strncmp(jp->p, "true", 4) == 0) {
        jb_json_node_t *h = jb_json_new_leaf(JB_JSON_BOOL);
        if (!h) return NULL;
        h->jbool = 1;
        jp->p += 4;
        return h;
    }
    if (strncmp(jp->p, "false", 5) == 0) {
        jb_json_node_t *h = jb_json_new_leaf(JB_JSON_BOOL);
        if (!h) return NULL;
        h->jbool = 0;
        jp->p += 5;
        return h;
    }
    if (strncmp(jp->p, "null", 4) == 0) {
        jp->p += 4;
        return jb_json_new_leaf(JB_JSON_NULL);
    }
    return jb_jp_parse_number(jp);
}

static jb_json_node_t *jb_json_parse_buf(char *text, char *err, size_t err_sz) {
    JBJsonParser jp = {0};
    jb_json_node_t *h;
    jp.p = text ? text : "";
    h = jb_jp_parse_value(&jp);
    if (h) {
        jb_jp_skip_ws(&jp);
        if (*jp.p != '\0') {
            jb_json_free_node(h);
            h = NULL;
            jb_jp_err(&jp, "Sobran caracteres tras el JSON");
        }
    }
    if (!h && err && err_sz > 0)
        snprintf(err, err_sz, "%s", jp.err[0] ? jp.err : "JSON invalido");
    return h;
}

jb_json_node_t *jb_json_clone_subtree(const jb_json_node_t *src) {
    jb_json_node_t *d;
    uint32_t i;
    if (!src) return NULL;
    d = (jb_json_node_t *)calloc(1, sizeof(jb_json_node_t));
    if (!d) return NULL;
    d->kind = src->kind;
    d->jbool = src->jbool;
    d->jint = src->jint;
    d->jfloat = src->jfloat;
    if (src->jstr) {
        d->jstr = strdup(src->jstr);
        if (!d->jstr) {
            free(d);
            return NULL;
        }
    }
    if (src->kind == JB_JSON_ARRAY) {
        for (i = 0; i < src->arr_len; i++) {
            jb_json_node_t *c = jb_json_clone_subtree(src->arr[i]);
            if (!c || !jb_json_arr_push(d, c)) {
                jb_json_free_node(d);
                return NULL;
            }
        }
    } else if (src->kind == JB_JSON_OBJECT) {
        for (i = 0; i < src->obj_len; i++) {
            jb_json_node_t *c = jb_json_clone_subtree(src->vals[i]);
            if (!c || !jb_json_obj_put(d, src->keys[i], c)) {
                jb_json_free_node(d);
                return NULL;
            }
        }
    }
    return d;
}

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} JBJsonBuf;

static int jb_jbuf_app(JBJsonBuf *b, const char *s) {
    size_t n = s ? strlen(s) : 0;
    if (!b) return 0;
    if (b->len + n + 1 > b->cap) {
        size_t cap = b->cap ? b->cap * 2u : 128u;
        while (cap < b->len + n + 1) cap *= 2u;
        b->data = (char *)realloc(b->data, cap);
        if (!b->data) return 0;
        b->cap = cap;
    }
    if (n) memcpy(b->data + b->len, s, n);
    b->len += n;
    b->data[b->len] = '\0';
    return 1;
}

static int jb_jbuf_spaces(JBJsonBuf *b, int n) {
    int i;
    for (i = 0; i < n; i++)
        if (!jb_jbuf_app(b, " ")) return 0;
    return 1;
}

static int jb_json_stringify_inner(const jb_json_node_t *v, JBJsonBuf *out, int indent_step, int depth);

static int jb_json_esc_string(JBJsonBuf *out, const char *buf) {
    size_t k;
    if (!jb_jbuf_app(out, "\"")) return 0;
    for (k = 0; buf[k]; k++) {
        char esc[3] = {0};
        switch (buf[k]) {
            case '"': if (!jb_jbuf_app(out, "\\\"")) return 0; break;
            case '\\': if (!jb_jbuf_app(out, "\\\\")) return 0; break;
            case '\n': if (!jb_jbuf_app(out, "\\n")) return 0; break;
            case '\r': if (!jb_jbuf_app(out, "\\r")) return 0; break;
            case '\t': if (!jb_jbuf_app(out, "\\t")) return 0; break;
            default:
                esc[0] = buf[k];
                if (!jb_jbuf_app(out, esc)) return 0;
                break;
        }
    }
    return jb_jbuf_app(out, "\"");
}

static int jb_json_stringify_inner(const jb_json_node_t *v, JBJsonBuf *out, int indent_step, int depth) {
    char tmp[64];
    int pretty = indent_step > 0;
    uint32_t i;
    if (!v) return jb_jbuf_app(out, "null");
    switch (v->kind) {
        case JB_JSON_NULL:
            return jb_jbuf_app(out, "null");
        case JB_JSON_BOOL:
            return jb_jbuf_app(out, v->jbool ? "true" : "false");
        case JB_JSON_INT:
            snprintf(tmp, sizeof tmp, "%lld", (long long)v->jint);
            return jb_jbuf_app(out, tmp);
        case JB_JSON_FLOAT:
            snprintf(tmp, sizeof tmp, "%.15g", v->jfloat);
            return jb_jbuf_app(out, tmp);
        case JB_JSON_STRING:
            return jb_json_esc_string(out, v->jstr ? v->jstr : "");
        case JB_JSON_OBJECT:
            if (!jb_jbuf_app(out, "{")) return 0;
            if (v->obj_len == 0) return jb_jbuf_app(out, "}");
            if (pretty) {
                if (!jb_jbuf_app(out, "\n")) return 0;
                for (i = 0; i < v->obj_len; i++) {
                    if (!jb_jbuf_spaces(out, (depth + 1) * indent_step)) return 0;
                    if (!jb_json_esc_string(out, v->keys[i] ? v->keys[i] : "")) return 0;
                    if (!jb_jbuf_app(out, ": ")) return 0;
                    if (!jb_json_stringify_inner(v->vals[i], out, indent_step, depth + 1)) return 0;
                    if (i + 1 < v->obj_len && !jb_jbuf_app(out, ",")) return 0;
                    if (!jb_jbuf_app(out, "\n")) return 0;
                }
                if (!jb_jbuf_spaces(out, depth * indent_step)) return 0;
                return jb_jbuf_app(out, "}");
            }
            for (i = 0; i < v->obj_len; i++) {
                if (i && !jb_jbuf_app(out, ",")) return 0;
                if (!jb_json_esc_string(out, v->keys[i] ? v->keys[i] : "")) return 0;
                if (!jb_jbuf_app(out, ":")) return 0;
                if (!jb_json_stringify_inner(v->vals[i], out, indent_step, depth + 1)) return 0;
            }
            return jb_jbuf_app(out, "}");
        case JB_JSON_ARRAY:
            if (!jb_jbuf_app(out, "[")) return 0;
            if (v->arr_len == 0) return jb_jbuf_app(out, "]");
            if (pretty) {
                if (!jb_jbuf_app(out, "\n")) return 0;
                for (i = 0; i < v->arr_len; i++) {
                    if (!jb_jbuf_spaces(out, (depth + 1) * indent_step)) return 0;
                    if (!jb_json_stringify_inner(v->arr[i], out, indent_step, depth + 1)) return 0;
                    if (i + 1 < v->arr_len && !jb_jbuf_app(out, ",")) return 0;
                    if (!jb_jbuf_app(out, "\n")) return 0;
                }
                if (!jb_jbuf_spaces(out, depth * indent_step)) return 0;
                return jb_jbuf_app(out, "]");
            }
            for (i = 0; i < v->arr_len; i++) {
                if (i && !jb_jbuf_app(out, ",")) return 0;
                if (!jb_json_stringify_inner(v->arr[i], out, indent_step, depth + 1)) return 0;
            }
            return jb_jbuf_app(out, "]");
        default:
            return 0;
    }
}

static const jb_json_node_t *jb_json_node_from_var(jb_var_t v) {
    if (v.type != JB_TYPE_JSON || !v.u.json) return NULL;
    return v.u.json;
}

jb_var_t jb_json_parse(jb_var_t text) {
    char buf[JB_JSON_BUF];
    char err[192];
    jb_json_node_t *root;
    if (text.type != JB_TYPE_TEXTO || !text.u.str) {
        jb_throw_val(jb_new_texto("json_parse: texto invalido"));
        return jb_new_nulo();
    }
    memset(buf, 0, sizeof buf);
    strncpy(buf, text.u.str, sizeof buf - 1);
    err[0] = 0;
    root = jb_json_parse_buf(buf, err, sizeof err);
    if (!root) {
        jb_throw_val(jb_new_texto(err[0] ? err : "json_parse: JSON invalido"));
        return jb_new_nulo();
    }
    return jb_json_wrap_owned(root);
}

jb_var_t jb_json_stringify(jb_var_t root, jb_var_t indent) {
    JBJsonBuf out = {0};
    int indent_step = 0;
    const jb_json_node_t *n;
    jb_var_t r;
    if (root.type == JB_TYPE_NULL)
        return jb_new_texto("null");
    if (root.type != JB_TYPE_JSON || !root.u.json) {
        jb_throw_val(jb_new_texto("json_stringify: valor JSON invalido"));
        return jb_new_texto("");
    }
    n = root.u.json;
    if (indent.type == JB_TYPE_ENTERO) indent_step = (int)indent.u.i64;
    else if (indent.type == JB_TYPE_FLOTANTE) indent_step = (int)indent.u.f64;
    if (indent_step < 0) indent_step = 0;
    if (indent_step > 16) indent_step = 16;
    if (!jb_json_stringify_inner(n, &out, indent_step, 0)) {
        free(out.data);
        jb_throw_val(jb_new_texto("json_stringify: valor JSON invalido"));
        return jb_new_texto("");
    }
    r = jb_new_texto(out.data ? out.data : "");
    free(out.data);
    return r;
}

jb_var_t jb_json_objeto_obtener(jb_var_t root, jb_var_t key) {
    const jb_json_node_t *o;
    uint32_t i;
    const char *ks;
    if (root.type != JB_TYPE_JSON || !root.u.json || root.u.json->kind != JB_JSON_OBJECT) {
        jb_throw_val(jb_new_texto("json_objeto_obtener: el valor no es un objeto JSON"));
        return jb_new_nulo();
    }
    if (key.type != JB_TYPE_TEXTO || !key.u.str) {
        jb_throw_val(jb_new_texto("json_objeto_obtener: clave JSON inexistente"));
        return jb_new_nulo();
    }
    o = root.u.json;
    ks = key.u.str;
    for (i = 0; i < o->obj_len; i++) {
        if (o->keys[i] && strcmp(o->keys[i], ks) == 0) {
            jb_json_node_t *c = jb_json_clone_subtree(o->vals[i]);
            if (!c) {
                jb_throw_val(jb_new_texto("json_objeto_obtener: sin memoria"));
                return jb_new_nulo();
            }
            return jb_json_wrap_owned(c);
        }
    }
    jb_throw_val(jb_new_texto("json_objeto_obtener: clave JSON inexistente"));
    return jb_new_nulo();
}

jb_var_t jb_json_lista_obtener(jb_var_t root, jb_var_t idx) {
    uint32_t ix;
    const jb_json_node_t *a;
    if (root.type != JB_TYPE_JSON || !root.u.json || root.u.json->kind != JB_JSON_ARRAY) {
        jb_throw_val(jb_new_texto("json_lista_obtener: indice JSON invalido"));
        return jb_new_nulo();
    }
    a = root.u.json;
    ix = (uint32_t)(idx.type == JB_TYPE_ENTERO ? idx.u.i64 : idx.type == JB_TYPE_FLOTANTE ? (int64_t)idx.u.f64 : 0);
    if (ix >= a->arr_len) {
        jb_throw_val(jb_new_texto("json_lista_obtener: indice JSON invalido"));
        return jb_new_nulo();
    }
    {
        jb_json_node_t *c = jb_json_clone_subtree(a->arr[ix]);
        if (!c) {
            jb_throw_val(jb_new_texto("json_lista_obtener: sin memoria"));
            return jb_new_nulo();
        }
        return jb_json_wrap_owned(c);
    }
}

jb_var_t jb_json_lista_tamano(jb_var_t root) {
    const jb_json_node_t *a = jb_json_node_from_var(root);
    if (!a || a->kind != JB_JSON_ARRAY) {
        jb_throw_val(jb_new_texto("json_lista_tamano: el valor no es un array JSON"));
        return jb_new_entero(0);
    }
    return jb_new_entero((int64_t)a->arr_len);
}

static int jb_json_parse_i64_strict(const char *buf, int64_t *out) {
    char *end = NULL;
    long long v;
    errno = 0;
    if (!buf || !out) return 0;
    v = strtoll(buf, &end, 10);
    if (errno || end == buf || (end && *end)) return 0;
    *out = (int64_t)v;
    return 1;
}

jb_var_t jb_json_a_texto(jb_var_t root) {
    const jb_json_node_t *v = jb_json_node_from_var(root);
    char tmp[128];
    JBJsonBuf out = {0};
    if (!v) {
        jb_throw_val(jb_new_texto("json_a_texto: handle JSON invalido"));
        return jb_new_texto("");
    }
    switch (v->kind) {
        case JB_JSON_STRING:
            return jb_new_texto(v->jstr ? v->jstr : "");
        case JB_JSON_INT:
            snprintf(tmp, sizeof tmp, "%lld", (long long)v->jint);
            return jb_new_texto(tmp);
        case JB_JSON_FLOAT:
            snprintf(tmp, sizeof tmp, "%.15g", v->jfloat);
            return jb_new_texto(tmp);
        case JB_JSON_BOOL:
            return jb_new_texto(v->jbool ? "true" : "false");
        case JB_JSON_NULL:
            return jb_new_texto("null");
        default:
            if (!jb_json_stringify_inner(v, &out, 0, 0)) {
                free(out.data);
                jb_throw_val(jb_new_texto("json_a_texto: no se pudo serializar el valor"));
                return jb_new_texto("");
            }
            {
                jb_var_t r = jb_new_texto(out.data ? out.data : "");
                free(out.data);
                return r;
            }
    }
}

jb_var_t jb_json_a_entero(jb_var_t root) {
    const jb_json_node_t *v = jb_json_node_from_var(root);
    int64_t iv = 0;
    if (!v) {
        jb_throw_val(jb_new_texto("json_a_entero: handle JSON invalido"));
        return jb_new_entero(0);
    }
    if (v->kind == JB_JSON_INT) iv = v->jint;
    else if (v->kind == JB_JSON_BOOL) iv = v->jbool ? 1 : 0;
    else if (v->kind == JB_JSON_FLOAT) iv = (int64_t)v->jfloat;
    else if (v->kind == JB_JSON_STRING && v->jstr && jb_json_parse_i64_strict(v->jstr, &iv))
        ;
    else {
        jb_throw_val(jb_new_texto("json_a_entero: conversion JSON invalida"));
        return jb_new_entero(0);
    }
    return jb_new_entero(iv);
}

jb_var_t jb_json_a_flotante(jb_var_t root) {
    const jb_json_node_t *v = jb_json_node_from_var(root);
    double fv = 0.0;
    char *end = NULL;
    if (!v) {
        jb_throw_val(jb_new_texto("json_a_flotante: handle JSON invalido"));
        return jb_new_flotante(0.0);
    }
    if (v->kind == JB_JSON_FLOAT) fv = v->jfloat;
    else if (v->kind == JB_JSON_INT) fv = (double)v->jint;
    else if (v->kind == JB_JSON_BOOL) fv = v->jbool ? 1.0 : 0.0;
    else if (v->kind == JB_JSON_STRING && v->jstr) {
        errno = 0;
        fv = strtod(v->jstr, &end);
        if (errno || !end || *end) fv = NAN;
    } else
        fv = NAN;
    if (isnan(fv)) {
        jb_throw_val(jb_new_texto("json_a_flotante: conversion JSON invalida"));
        return jb_new_flotante(0.0);
    }
    return jb_new_flotante(fv);
}

jb_var_t jb_json_a_bool(jb_var_t root) {
    const jb_json_node_t *v = jb_json_node_from_var(root);
    uint64_t bv = 0;
    if (!v) {
        jb_throw_val(jb_new_texto("json_a_bool: handle JSON invalido"));
        return jb_new_bool(false);
    }
    switch (v->kind) {
        case JB_JSON_NULL:
            bv = 0;
            break;
        case JB_JSON_BOOL:
            bv = v->jbool ? 1u : 0u;
            break;
        case JB_JSON_INT:
            bv = v->jint != 0 ? 1u : 0u;
            break;
        case JB_JSON_FLOAT:
            bv = v->jfloat != 0.0 ? 1u : 0u;
            break;
        case JB_JSON_STRING:
            bv = (v->jstr && v->jstr[0]) ? 1u : 0u;
            break;
        default:
            bv = (v->kind == JB_JSON_ARRAY && v->arr_len) || (v->kind == JB_JSON_OBJECT && v->obj_len) ? 1u : 0u;
            break;
    }
    return jb_new_bool(bv != 0);
}

jb_var_t jb_json_tipo(jb_var_t root) {
    const jb_json_node_t *v = jb_json_node_from_var(root);
    return jb_new_entero((int64_t)(v ? (int)v->kind : 0));
}

void jb_json_print_compact(jb_var_t v) {
    JBJsonBuf out = {0};
    const jb_json_node_t *n;
    if (v.type != JB_TYPE_JSON || !v.u.json) {
        fputs("(json?)", stdout);
        return;
    }
    n = v.u.json;
    if (!jb_json_stringify_inner(n, &out, 0, 0)) {
        fputs("(json)", stdout);
        free(out.data);
        return;
    }
    fputs(out.data ? out.data : "", stdout);
    free(out.data);
}
