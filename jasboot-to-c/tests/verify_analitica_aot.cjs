"use strict";
/**
 * 1) Transpila (jbc-to-c) todos los .jasb bajo stdlib/analitica-neuronal y borra .c/.exe generados junto al fuente.
 * 2) Opcional: paridad VM vs AOT sobre tests/analitica-neuronal/200–202 (mismo mecanismo que verify_vm_aot.cjs).
 *
 * Uso (desde la raíz del repo jasboot):
 *   node sdk-dependiente/jasboot-to-c/tests/verify_analitica_aot.cjs
 *   node sdk-dependiente/jasboot-to-c/tests/verify_analitica_aot.cjs --no-parity   # solo barrido transpile
 */
const { spawnSync } = require("child_process");
const fs = require("fs");
const path = require("path");

const workspaceRoot = path.resolve(__dirname, "../../..");
const childEnv = { ...process.env, JASBOOT_REPO_ROOT: workspaceRoot };

function findJbcToC() {
  if (process.env.JASBOOT_JBC_TO_C) {
    const p = path.isAbsolute(process.env.JASBOOT_JBC_TO_C)
      ? process.env.JASBOOT_JBC_TO_C
      : path.join(workspaceRoot, process.env.JASBOOT_JBC_TO_C);
    if (fs.existsSync(p)) return p;
  }
  const local = path.join(
    __dirname,
    "..",
    process.platform === "win32" ? "jbc-to-c.exe" : "jbc-to-c",
  );
  if (fs.existsSync(local)) return local;
  return null;
}

function collectJasbUnder(dir) {
  const out = [];
  if (!fs.existsSync(dir)) return out;
  const stack = [dir];
  while (stack.length) {
    const d = stack.pop();
    let ents;
    try {
      ents = fs.readdirSync(d, { withFileTypes: true });
    } catch {
      continue;
    }
    for (const e of ents) {
      const p = path.join(d, e.name);
      if (e.isDirectory()) stack.push(p);
      else if (e.isFile() && e.name.endsWith(".jasb")) out.push(p);
    }
  }
  return out.sort();
}

function cleanupTranspileArtifacts(jasbPath) {
  const base = jasbPath.replace(/\.jasb$/i, "");
  for (const ext of [".c", ".exe"]) {
    const f = base + ext;
    try {
      if (fs.existsSync(f)) fs.unlinkSync(f);
    } catch {
      /* ignore */
    }
  }
  if (process.platform !== "win32") {
    try {
      if (fs.existsSync(base)) fs.unlinkSync(base);
    } catch {
      /* ignore */
    }
  }
}

function transpileAll(jbcToC) {
  const root = path.join(workspaceRoot, "stdlib", "analitica-neuronal");
  const files = collectJasbUnder(root);
  let fail = 0;
  const errors = [];
  for (const jasb of files) {
    const rel = path.relative(workspaceRoot, jasb);
    const r = spawnSync(jbcToC, [jasb], {
      cwd: workspaceRoot,
      env: childEnv,
      encoding: "utf8",
      windowsHide: true,
    });
    const ok = r.status === 0;
    if (!ok) {
      fail++;
      errors.push({
        rel,
        err: (r.stderr || r.stdout || "").trim() || `exit ${r.status}`,
      });
    }
    cleanupTranspileArtifacts(jasb);
    process.stdout.write(ok ? "." : "X");
  }
  process.stdout.write("\n");
  return { total: files.length, fail, errors };
}

function runParitySubset() {
  const verify = path.join(__dirname, "verify_vm_aot.cjs");
  const parity = [
    path.join(__dirname, "analitica-neuronal", "200_aot_analitica_math.jasb"),
    path.join(__dirname, "analitica-neuronal", "201_aot_analitica_normalizacion.jasb"),
    path.join(__dirname, "analitica-neuronal", "202_aot_analitica_metricas_regresion.jasb"),
    path.join(__dirname, "analitica-neuronal", "203_aot_nativo_mlp_entrenar.jasb"),
  ].map((p) => path.resolve(p));
  const r = spawnSync(process.execPath, [verify, ...parity], {
    cwd: workspaceRoot,
    env: childEnv,
    encoding: "utf8",
    stdio: "inherit",
  });
  return r.status === 0;
}

function main() {
  const noParity = process.argv.includes("--no-parity");
  const jbcToC = findJbcToC();
  if (!jbcToC) {
    console.error(
      "No se encuentra jbc-to-c (compile Makefile en jasboot-to-c o defina JASBOOT_JBC_TO_C).",
    );
    process.exit(1);
  }

  console.log(
    "Barrido transpile: stdlib/analitica-neuronal/**/*.jasb (se eliminan .c/.exe junto al .jasb tras cada paso)",
  );
  const { total, fail, errors } = transpileAll(jbcToC);
  console.log(`Transpile: ${total - fail}/${total} OK`);
  if (fail > 0) {
    console.error(`Fallaron ${fail} archivo(s):`);
    for (const e of errors) console.error(" -", e.rel, "\n", e.err.slice(0, 2000));
    process.exit(1);
  }

  if (!noParity) {
    console.log("\nParidad VM/AOT (200–202 analítica)…");
    if (!runParitySubset()) process.exit(1);
  }

  console.log("Listo: analítica neuronal — transpile completo" + (noParity ? "" : " y paridad 200–202 OK."));
  process.exit(0);
}

main();
