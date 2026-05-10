#include "vm_analitica_mlp.h"
#include "memoria_neuronal/memoria_neuronal.h"
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static JMNMemoria* mlp_pick_mem(VM* vm, uint32_t list_id) {
    (void)list_id;
    if (vm->mem_colecciones && jmn_lista_existe(vm->mem_colecciones, list_id))
        return vm->mem_colecciones;
    if (vm->mem_neuronal && jmn_lista_existe(vm->mem_neuronal, list_id))
        return vm->mem_neuronal;
    return vm->mem_colecciones;
}

static float reg_lo_f(uint64_t r) {
    union {
        uint32_t u;
        float f;
    } u;
    u.u = (uint32_t)(r & 0xFFFFFFFFu);
    return u.f;
}

static uint32_t reg_u32(uint64_t r) { return (uint32_t)(r & 0xFFFFFFFFu); } 

static double sigmoid(double z) {
    if (z > 35.0)
        return 1.0;
    if (z < -35.0)
        return 0.0;
    return 1.0 / (1.0 + exp(-z));
}

static void softmax_stable(const double* logits, int n, double* probs) {
    if (!logits || !probs || n <= 0) return;
    double max_v = logits[0];
    for (int i = 1; i < n; i++) {
        if (logits[i] > max_v) max_v = logits[i];
    }
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        double e = exp(logits[i] - max_v);
        probs[i] = e;
        sum += e;
    }
    if (sum <= 0.0) {
        double inv = 1.0 / (double)n;
        for (int i = 0; i < n; i++) probs[i] = inv;
        return;
    }
    for (int i = 0; i < n; i++) probs[i] /= sum;
}

float vm_analitica_mlp_fit_native(VM* vm, uint8_t base_reg) {
    if (!vm)
        return 1.0f;
    uint64_t* R = vm->registers;

    uint32_t pesos_id = reg_u32(R[base_reg + 0]);
    uint32_t sesgos_id = reg_u32(R[base_reg + 1]);
    uint32_t X_id = reg_u32(R[base_reg + 2]);
    uint32_t y_id = reg_u32(R[base_reg + 3]);
    uint32_t capas_id = reg_u32(R[base_reg + 4]);
    float lr = reg_lo_f(R[base_reg + 5]);
    uint32_t epochs = reg_u32(R[base_reg + 6]);

    if (lr <= 0.f || lr > 10.f || epochs == 0 || epochs > 10000000u) {
        fprintf(stderr, "[native_mlp] parametros lr/epochs invalidos\n");
        return 1.0f;
    }

    JMNMemoria* mp = mlp_pick_mem(vm, pesos_id);
    JMNMemoria* ms = mlp_pick_mem(vm, sesgos_id);
    JMNMemoria* mx = mlp_pick_mem(vm, X_id);
    JMNMemoria* my = mlp_pick_mem(vm, y_id);
    JMNMemoria* mc = mlp_pick_mem(vm, capas_id);
    if (!mp || !ms || !mx || !my || !mc) {
        fprintf(stderr, "[native_mlp] memoria de listas no disponible\n");
        return 1.0f;
    }

    int H = 0;
    if (jmn_lista_tamano(mc, capas_id) < 1) {
        fprintf(stderr, "[native_mlp] capas_ocultas vacia\n");
        return 1.0f;
    }
    {
        JMNValor v0 = jmn_lista_obtener(mc, capas_id, 0);
        float hf = v0.f;
        H = (int)(hf + 0.5f);
        if (H <= 0)
            H = (int)v0.u;
        if (H <= 0 || H > 8192) {
            fprintf(stderr, "[native_mlp] H oculto invalido: %d\n", H);
            return 1.0f;
        }
    }

    uint32_t N = jmn_lista_tamano(mx, X_id);
    if (N == 0 || jmn_lista_tamano(my, y_id) != N) {
        fprintf(stderr, "[native_mlp] X/y tamano inconsistente\n");
        return 1.0f;
    }

    uint32_t row0 = jmn_lista_obtener(mx, X_id, 0).u;
    JMNMemoria* mrow0 = mlp_pick_mem(vm, row0);
    if (!mrow0) {
        fprintf(stderr, "[native_mlp] fila X invalida\n");
        return 1.0f;
    }
    int n_in = (int)jmn_lista_tamano(mrow0, row0);
    if (n_in <= 0 || n_in > 1024) {
        fprintf(stderr, "[native_mlp] n_in invalido\n");
        return 1.0f;
    }

    if (jmn_lista_tamano(mp, pesos_id) < 2 || jmn_lista_tamano(ms, sesgos_id) < 2) {
        fprintf(stderr, "[native_mlp] se esperan 2 capas de pesos/sesgos\n");
        return 1.0f;
    }

    size_t W1sz = (size_t)n_in * (size_t)H;
    int n_out = 1;
    bool y_vectorial = false;
    {
        JMNValor y0v = jmn_lista_obtener(my, y_id, 0);
        uint32_t y0id = y0v.u;
        JMNMemoria* my0 = mlp_pick_mem(vm, y0id);
        if (my0 && jmn_lista_existe(my0, y0id)) {
            n_out = (int)jmn_lista_tamano(my0, y0id);
            y_vectorial = true;
        }
        if (n_out <= 0 || n_out > 512) {
            fprintf(stderr, "[native_mlp] n_out invalido: %d\n", n_out);
            return 1.0f;
        }
    }

    size_t W2sz = (size_t)H * (size_t)n_out;
    double* W1 = (double*)calloc(W1sz, sizeof(double));
    double* b1 = (double*)calloc((size_t)H, sizeof(double));
    double* W2 = (double*)calloc(W2sz, sizeof(double));
    double* b2 = (double*)calloc((size_t)n_out, sizeof(double));
    double* xs = (double*)calloc((size_t)N * (size_t)n_in, sizeof(double));
    double* ys = (double*)calloc((size_t)N * (size_t)n_out, sizeof(double));
    double* z1 = (double*)calloc((size_t)H, sizeof(double));
    double* a1 = (double*)calloc((size_t)H, sizeof(double));
    double* d1 = (double*)calloc((size_t)H, sizeof(double));

    double* z2 = (double*)calloc((size_t)n_out, sizeof(double));
    double* out = (double*)calloc((size_t)n_out, sizeof(double));
    double* d2 = (double*)calloc((size_t)n_out, sizeof(double));

    if (!W1 || !b1 || !W2 || !b2 || !xs || !ys || !z1 || !a1 || !d1 || !z2 || !out || !d2) {
        free(W1);
        free(b1);
        free(W2);
        free(b2);
        free(xs);
        free(ys);
        free(z1);
        free(a1);
        free(d1);
        free(z2);
        free(out);
        free(d2);
        return 1.0f;
    }

    for (uint32_t s = 0; s < N; s++) {
        uint32_t inner = jmn_lista_obtener(mx, X_id, s).u;
        JMNMemoria* mi = mlp_pick_mem(vm, inner);
        if (!mi || jmn_lista_tamano(mi, inner) != (uint32_t)n_in) {
            fprintf(stderr, "[native_mlp] muestra %u dim incorrecta\n", (unsigned)s);
            goto fail_buf;
        }
        for (int j = 0; j < n_in; j++) {
            float xf = jmn_lista_obtener(mi, inner, (uint32_t)j).f;
            xs[(size_t)s * (size_t)n_in + (size_t)j] = (double)xf;
        }
        if (y_vectorial) {
            uint32_t yrow = jmn_lista_obtener(my, y_id, s).u;
            JMNMemoria* myr = mlp_pick_mem(vm, yrow);
            if (!myr || jmn_lista_tamano(myr, yrow) != (uint32_t)n_out) {
                fprintf(stderr, "[native_mlp] objetivo vectorial invalido en muestra %u\n", (unsigned)s);
                goto fail_buf;
            }
            for (int k = 0; k < n_out; k++) {
                ys[(size_t)s * (size_t)n_out + (size_t)k] = (double)jmn_lista_obtener(myr, yrow, (uint32_t)k).f;
            }
        } else {
            ys[(size_t)s * (size_t)n_out] = (double)jmn_lista_obtener(my, y_id, s).f;
        }
    }

    uint32_t L0 = jmn_lista_obtener(mp, pesos_id, 0).u;
    JMNMemoria* mL0 = mlp_pick_mem(vm, L0);
    if (!mL0 || jmn_lista_tamano(mL0, L0) != (uint32_t)n_in) {
        fprintf(stderr, "[native_mlp] capa0 filas != n_in\n");
        goto fail_buf;
    }
    for (int i = 0; i < n_in; i++) {
        uint32_t row = jmn_lista_obtener(mL0, L0, (uint32_t)i).u;
        JMNMemoria* mr = mlp_pick_mem(vm, row);
        if (!mr || jmn_lista_tamano(mr, row) != (uint32_t)H) {
            fprintf(stderr, "[native_mlp] capa0 ancho != H\n");
            goto fail_buf;
        }
        for (int j = 0; j < H; j++) {
            float w = jmn_lista_obtener(mr, row, (uint32_t)j).f;
            W1[(size_t)i * (size_t)H + (size_t)j] = (double)w;
        }
    }

    uint32_t S0 = jmn_lista_obtener(ms, sesgos_id, 0).u;
    JMNMemoria* mS0 = mlp_pick_mem(vm, S0);
    if (!mS0 || jmn_lista_tamano(mS0, S0) != (uint32_t)H) {
        fprintf(stderr, "[native_mlp] sesgos capa0 invalidos\n");
        goto fail_buf;
    }
    for (int j = 0; j < H; j++)
        b1[j] = (double)jmn_lista_obtener(mS0, S0, (uint32_t)j).f;

    uint32_t L1 = jmn_lista_obtener(mp, pesos_id, 1).u;
    JMNMemoria* mL1 = mlp_pick_mem(vm, L1);
    if (!mL1 || jmn_lista_tamano(mL1, L1) != (uint32_t)H) {
        fprintf(stderr, "[native_mlp] capa1 filas != H\n");
        goto fail_buf;
    }
    for (int i = 0; i < H; i++) {
        uint32_t row = jmn_lista_obtener(mL1, L1, (uint32_t)i).u;
        JMNMemoria* mr = mlp_pick_mem(vm, row);
        if (!mr || jmn_lista_tamano(mr, row) < (uint32_t)n_out) {
            fprintf(stderr, "[native_mlp] capa1 ancho\n");
            goto fail_buf;
        }
        for (int k = 0; k < n_out; k++) {
            float w = jmn_lista_obtener(mr, row, (uint32_t)k).f;
            W2[(size_t)i * (size_t)n_out + (size_t)k] = (double)w;
        }
    }

    uint32_t S1 = jmn_lista_obtener(ms, sesgos_id, 1).u;
    JMNMemoria* mS1 = mlp_pick_mem(vm, S1);
    if (!mS1 || jmn_lista_tamano(mS1, S1) < (uint32_t)n_out) {
        fprintf(stderr, "[native_mlp] sesgos salida\n");
        goto fail_buf;
    }
    for (int k = 0; k < n_out; k++) {
        b2[k] = (double)jmn_lista_obtener(mS1, S1, (uint32_t)k).f;
    }

    double lr64 = (double)lr;
    float last_mse = 1.0f;

    for (uint32_t ep = 0; ep < epochs; ep++) {
        double errsum = 0.0;
        for (uint32_t s = 0; s < N; s++) {
            const double* x = xs + (size_t)s * (size_t)n_in;

            const double* y = ys + (size_t)s * (size_t)n_out;
            for (int j = 0; j < H; j++) {
                double sum = b1[j];
                for (int i = 0; i < n_in; i++)
                    sum += W1[(size_t)i * (size_t)H + (size_t)j] * x[i];
                z1[j] = sum;
                a1[j] = sum > 0.0 ? sum : 0.0;
            }
            for (int k = 0; k < n_out; k++) {
                double sum2 = b2[k];
                for (int j = 0; j < H; j++) {
                    sum2 += W2[(size_t)j * (size_t)n_out + (size_t)k] * a1[j];
                }
                z2[k] = sum2;
            }
            if (n_out > 1) {
                softmax_stable(z2, n_out, out);
                for (int k = 0; k < n_out; k++) {
                    double e = out[k] - y[k];
                    errsum += e * e;
                    /* dL/dz con softmax + CE: p - y */
                    d2[k] = e;
                }
            } else {
                out[0] = sigmoid(z2[0]);
                double e = out[0] - y[0];
                errsum += e * e;
                d2[0] = e * (out[0] * (1.0 - out[0]));
            }

            for (int i = 0; i < H; i++) {
                double chain = 0.0;
                for (int k = 0; k < n_out; k++) {
                    chain += W2[(size_t)i * (size_t)n_out + (size_t)k] * d2[k];
                }
                double relu_p = z1[i] > 0.0 ? 1.0 : 0.0;
                d1[i] = chain * relu_p;
            }

            for (int i = 0; i < H; i++) {
                for (int k = 0; k < n_out; k++) {
                    double g = d2[k] * a1[i];
                    W2[(size_t)i * (size_t)n_out + (size_t)k] -= lr64 * g;
                }
            }
            for (int k = 0; k < n_out; k++) {
                b2[k] -= lr64 * d2[k];
            }

            for (int i = 0; i < n_in; i++) {
                for (int j = 0; j < H; j++) {
                    double g = d1[j] * x[i];
                    W1[(size_t)i * (size_t)H + (size_t)j] -= lr64 * g;
                }
            }
            for (int j = 0; j < H; j++)
                b1[j] -= lr64 * d1[j];
        }
        last_mse = (float)(errsum / ((double)N * (double)n_out));
    }

    for (int i = 0; i < n_in; i++) {
        uint32_t row = jmn_lista_obtener(mL0, L0, (uint32_t)i).u;
        JMNMemoria* mr = mlp_pick_mem(vm, row);
        for (int j = 0; j < H; j++) {
            JMNValor vw;
            vw.f = (float)W1[(size_t)i * (size_t)H + (size_t)j];
            jmn_lista_poner(mr, row, (uint32_t)j, vw);
        }
    }
    /* Reconstruir sesgos capa 0 por agregar (evita celdas residuales no-float tras realloc / mezclas). */
    jmn_vector_limpiar(mS0, S0);
    for (int j = 0; j < H; j++) {
        JMNValor vb;
        vb.f = (float)b1[j];
        jmn_lista_agregar(mS0, S0, vb);
    }
    for (int i = 0; i < H; i++) {
        uint32_t row = jmn_lista_obtener(mL1, L1, (uint32_t)i).u;
        JMNMemoria* mr = mlp_pick_mem(vm, row);
        for (int k = 0; k < n_out; k++) {
            JMNValor vw;
            vw.f = (float)W2[(size_t)i * (size_t)n_out + (size_t)k];
            jmn_lista_poner(mr, row, (uint32_t)k, vw);
        }
    }
    jmn_vector_limpiar(mS1, S1);
    for (int k = 0; k < n_out; k++) {
        JMNValor vb;
        vb.f = (float)b2[k];
        jmn_lista_agregar(mS1, S1, vb);
    }

    free(W1);
    free(b1);
    free(W2);
    free(b2);
    free(xs);
    free(ys);
    free(z1);
    free(a1);
    free(d1);
    free(z2);
    free(out);
    free(d2);
    return last_mse;

fail_buf:
    free(W1);
    free(b1);
    free(W2);
    free(b2);
    free(xs);
    free(ys);
    free(z1);
    free(a1);
    free(d1);
    free(z2);
    free(out);
    free(d2);
    return 1.0f;
}

float vm_analitica_mlp_predict_native(VM* vm, uint8_t base_reg) {
    if (!vm) return 0.0f;
    uint64_t* R = vm->registers;
    uint32_t pesos_id = reg_u32(R[base_reg + 0]);
    uint32_t sesgos_id = reg_u32(R[base_reg + 1]);
    uint32_t x_id = reg_u32(R[base_reg + 2]);

    JMNMemoria* mp = mlp_pick_mem(vm, pesos_id);
    JMNMemoria* ms = mlp_pick_mem(vm, sesgos_id);
    JMNMemoria* mx = mlp_pick_mem(vm, x_id);
    if (!mp || !ms || !mx) return 0.0f;

    uint32_t L0 = jmn_lista_obtener(mp, pesos_id, 0).u;
    uint32_t L1 = jmn_lista_obtener(mp, pesos_id, 1).u;
    uint32_t S0 = jmn_lista_obtener(ms, sesgos_id, 0).u;
    uint32_t S1 = jmn_lista_obtener(ms, sesgos_id, 1).u;

    JMNMemoria* mL0 = mlp_pick_mem(vm, L0);
    JMNMemoria* mL1 = mlp_pick_mem(vm, L1);
    JMNMemoria* mS0 = mlp_pick_mem(vm, S0);
    JMNMemoria* mS1 = mlp_pick_mem(vm, S1);

    int n_in = (int)jmn_lista_tamano(mL0, L0);
    int H = (int)jmn_lista_tamano(mS0, S0);
    int n_out = (int)jmn_lista_tamano(mS1, S1);

    // Forward pass
    double* z1 = (double*)malloc(sizeof(double) * H);
    double* a1 = (double*)malloc(sizeof(double) * H);
    
    for (int j = 0; j < H; j++) {
        double sum = (double)jmn_lista_obtener(mS0, S0, (uint32_t)j).f;
        for (int i = 0; i < n_in; i++) {
            uint32_t row_id = jmn_lista_obtener(mL0, L0, (uint32_t)i).u;
            JMNMemoria* mr = mlp_pick_mem(vm, row_id);
            double w = (double)jmn_lista_obtener(mr, row_id, (uint32_t)j).f;
            double x = (double)jmn_lista_obtener(mx, x_id, (uint32_t)i).f;
            sum += w * x;
        }
        z1[j] = sum;
        a1[j] = sum > 0.0 ? sum : 0.0;
    }

    double res = 0.0;
    if (n_out == 1) {
        double sum2 = (double)jmn_lista_obtener(mS1, S1, 0).f;
        for (int j = 0; j < H; j++) {
            uint32_t row_id = jmn_lista_obtener(mL1, L1, (uint32_t)j).u;
            JMNMemoria* mr = mlp_pick_mem(vm, row_id);
            double w = (double)jmn_lista_obtener(mr, row_id, 0).f;
            sum2 += w * a1[j];
        }
        res = sigmoid(sum2);
    } else {
        // Softmax not implemented for simple predict yet, return first logit
        res = 0.0; 
    }

    free(z1);
    free(a1);
    return (float)res;
}

int vm_analitica_mlp_save_native(VM* vm, uint8_t base_reg) {
    if (!vm) return 0;
    uint64_t* R = vm->registers;
    uint32_t ruta_id = reg_u32(R[base_reg + 0]);
    uint32_t pesos_id = reg_u32(R[base_reg + 1]);
    uint32_t sesgos_id = reg_u32(R[base_reg + 2]);

    char ruta[512];
    if (jmn_obtener_texto(vm->mem_neuronal, ruta_id, ruta, sizeof(ruta)) <= 0) return 0;

    FILE* f = fopen(ruta, "wb");
    if (!f) return 0;

    fwrite("JBM1", 4, 1, f);
    uint8_t ver = 1;
    fwrite(&ver, 1, 1, f);

    JMNMemoria* mp = mlp_pick_mem(vm, pesos_id);
    JMNMemoria* ms = mlp_pick_mem(vm, sesgos_id);
    if (!mp || !ms) { fclose(f); return 0; }

    uint32_t L0_id = jmn_lista_obtener(mp, pesos_id, 0).u;
    uint32_t L1_id = jmn_lista_obtener(mp, pesos_id, 1).u;
    uint32_t S0_id = jmn_lista_obtener(ms, sesgos_id, 0).u;
    uint32_t S1_id = jmn_lista_obtener(ms, sesgos_id, 1).u;

    JMNMemoria* mL0 = mlp_pick_mem(vm, L0_id);
    JMNMemoria* mL1 = mlp_pick_mem(vm, L1_id);
    JMNMemoria* mS0 = mlp_pick_mem(vm, S0_id);
    JMNMemoria* mS1 = mlp_pick_mem(vm, S1_id);

    // Metadata basica
    uint32_t n_in = jmn_lista_tamano(mL0, L0_id);
    uint32_t H = jmn_lista_tamano(mS0, S0_id);
    uint32_t n_out = jmn_lista_tamano(mS1, S1_id);

    fwrite(&n_in, 4, 1, f);
    fwrite(&H, 4, 1, f);
    fwrite(&n_out, 4, 1, f);

    // Pesos Capa 0 (n_in x H)
    for (uint32_t i = 0; i < n_in; i++) {
        uint32_t row_id = jmn_lista_obtener(mL0, L0_id, i).u;
        JMNMemoria* mr = mlp_pick_mem(vm, row_id);
        for (uint32_t j = 0; j < H; j++) {
            float w = jmn_lista_obtener(mr, row_id, j).f;
            fwrite(&w, 4, 1, f);
        }
    }

    // Pesos Capa 1 (H x n_out)
    for (uint32_t j = 0; j < H; j++) {
        uint32_t row_id = jmn_lista_obtener(mL1, L1_id, j).u;
        JMNMemoria* mr = mlp_pick_mem(vm, row_id);
        for (uint32_t k = 0; k < n_out; k++) {
            float w = jmn_lista_obtener(mr, row_id, k).f;
            fwrite(&w, 4, 1, f);
        }
    }

    // Sesgos S0 (H)
    for (uint32_t j = 0; j < H; j++) {
        float b = jmn_lista_obtener(mS0, S0_id, j).f;
        fwrite(&b, 4, 1, f);
    }

    // Sesgos S1 (n_out)
    for (uint32_t k = 0; k < n_out; k++) {
        float b = jmn_lista_obtener(mS1, S1_id, k).f;
        fwrite(&b, 4, 1, f);
    }

    fclose(f);
    return 1;
}
