"use strict";

const fs = require("fs");
const path = require("path");
const vscode = require("vscode");

function fileExists(targetPath) {
  try {
    return fs.existsSync(targetPath);
  } catch {
    return false;
  }
}

function dirExists(targetPath) {
  try {
    return fs.existsSync(targetPath) && fs.statSync(targetPath).isDirectory();
  } catch {
    return false;
  }
}

function resolveProjectRoot(filePath) {
  let current = path.dirname(filePath);
  while (true) {
    if (
      fileExists(path.join(current, ".vscode", "run-jasb.cjs")) ||
      dirExists(path.join(current, "stdlib")) ||
      dirExists(path.join(current, "sdk-dependiente"))
    ) {
      return current;
    }
    const parent = path.dirname(current);
    if (parent === current) break;
    current = parent;
  }

  const workspaceFolder = vscode.workspace.getWorkspaceFolder(
    vscode.Uri.file(filePath),
  );
  return workspaceFolder ? workspaceFolder.uri.fsPath : path.dirname(filePath);
}

function findBundledLauncher(projectRoot) {
  const launcher = path.join(projectRoot, ".vscode", "run-jasb.cjs");
  return fileExists(launcher) ? launcher : null;
}

function findJbc(projectRoot) {
  const rels = [
    ["sdk-dependiente", "jas-compiler-c", "bin", "jbc.exe"],
    ["sdk-dependiente", "jas-compiler-c", "bin", "jbc-next.exe"],
    ["sdk", "jas-compiler-c", "bin", "jbc.exe"],
    ["sdk", "jas-compiler-c", "bin", "jbc"],
    ["sdk-dependiente", "jas-compiler-c", "bin", "jbc"],
    ["..", "bin", "jbc.exe"],
    ["..", "bin", "jbc_new.exe"],
    ["..", "bin", "jbc-next.exe"],
    ["..", "bin", "jbc"],
  ];

  for (const parts of rels) {
    const candidate = path.join(projectRoot, ...parts);
    if (fileExists(candidate)) return candidate;
  }
  return null;
}

function quotePowerShell(value) {
  return `'${String(value).replace(/'/g, "''")}'`;
}

function quoteShell(value) {
  return `'${String(value).replace(/'/g, `'\\''`)}'`;
}

function buildCommand({ launcherPath, jbcPath, filePath }) {
  if (process.platform === "win32") {
    if (launcherPath) {
      return `node ${quotePowerShell(launcherPath)} ${quotePowerShell(filePath)}`;
    }
    return `${quotePowerShell(jbcPath)} ${quotePowerShell(filePath)} -e`;
  }

  if (launcherPath) {
    return `node ${quoteShell(launcherPath)} ${quoteShell(filePath)}`;
  }
  return `${quoteShell(jbcPath)} ${quoteShell(filePath)} -e`;
}

function getTerminal() {
  const terminalName = "Jasboot";
  let terminal = vscode.window.terminals.find(
    (item) => item.name === terminalName,
  );
  if (!terminal) {
    terminal = vscode.window.createTerminal({ name: terminalName });
  }
  return terminal;
}

async function runActiveFile() {
  const editor = vscode.window.activeTextEditor;
  if (!editor) {
    vscode.window.showErrorMessage("No hay un editor activo para ejecutar.");
    return;
  }

  const filePath = editor.document.uri.fsPath;
  const ext = path.extname(filePath).toLowerCase();
  if (ext !== ".jasb" && ext !== ".jd") {
    vscode.window.showErrorMessage(
      "Abre un archivo .jasb o .jd para ejecutar con Jasboot.",
    );
    return;
  }

  if (editor.document.isDirty) {
    await editor.document.save();
  }

  const projectRoot = resolveProjectRoot(filePath);
  const launcherPath = findBundledLauncher(projectRoot);
  const jbcPath = findJbc(projectRoot);

  if (!launcherPath && !jbcPath) {
    vscode.window.showErrorMessage(
      "No se encontro un launcher Jasboot ni un compilador jbc para ejecutar el archivo activo.",
    );
    return;
  }

  if (ext === ".jd" && !launcherPath) {
    vscode.window.showErrorMessage(
      "Para ejecutar archivos .jd se necesita un launcher compatible como .vscode/run-jasb.cjs en el proyecto.",
    );
    return;
  }

  const command = buildCommand({ launcherPath, jbcPath, filePath });
  const terminal = getTerminal();
  terminal.show(true);
  terminal.sendText(command, true);
}

const cp = require("child_process");

function activate(context) {
  context.subscriptions.push(
    vscode.commands.registerCommand("jasboot.runActiveFile", runActiveFile),
  );

  const selector = { scheme: "file", language: "jasboot" };

  // Proveedor de navegación (Go to Definition)
  context.subscriptions.push(
    vscode.languages.registerDefinitionProvider(
      selector,
      new JasbootDefinitionProvider(),
    ),
  );

  // Proveedor de información al pasar el mouse (Hover)
  context.subscriptions.push(
    vscode.languages.registerHoverProvider(
      selector,
      new JasbootHoverProvider(),
    ),
  );

  // Sistema de Diagnósticos (Errores y Advertencias)
  const diagnosticCollection =
    vscode.languages.createDiagnosticCollection("jasboot");
  context.subscriptions.push(diagnosticCollection);

  // Suscribirse a cambios en el documento para actualizar diagnósticos
  context.subscriptions.push(
    vscode.workspace.onDidSaveTextDocument((doc) => {
      if (doc.languageId === "jasboot")
        updateDiagnostics(doc, diagnosticCollection);
    }),
  );
  context.subscriptions.push(
    vscode.workspace.onDidOpenTextDocument((doc) => {
      if (doc.languageId === "jasboot")
        updateDiagnostics(doc, diagnosticCollection);
    }),
  );

  // Ejecutar inicial al activar si hay un editor abierto
  if (vscode.window.activeTextEditor) {
    const doc = vscode.window.activeTextEditor.document;
    if (doc.languageId === "jasboot")
      updateDiagnostics(doc, diagnosticCollection);
  }
}

async function updateDiagnostics(document, collection) {
  const filePath = document.uri.fsPath;
  const projectRoot = resolveProjectRoot(filePath);
  const jbcPath = findJbc(projectRoot);

  if (!jbcPath) return;

  // Ejecutar compilador en modo solo-chequeo (si no hay -o, jbc compila igual pero podemos ignorar el .jbo)
  const cmd = `"${jbcPath}" "${filePath}"`;

  cp.exec(cmd, { cwd: projectRoot }, (err, stdout, stderr) => {
    const diagnostics = [];
    let output = stderr + stdout;

    // Limpiar códigos de escape ANSI antes de procesar
    output = output.replace(/\x1b\[[0-9;]*m/g, "");

    // Expresión regular para parsear errores de jbc:
    // Archivo PATH, linea L, columna C: error/aviso: MSG
    const regex =
      /Archivo\s+(.*?),\s+linea\s+(\d+),\s+columna\s+(\d+):\s+(error|aviso):\s+(.*)/g;
    let match;

    while ((match = regex.exec(output)) !== null) {
      const [, file, lineStr, colStr, severityStr, message] = match;

      // Solo mostrar errores del archivo actual para evitar ruido de módulos
      if (path.basename(file) !== path.basename(filePath)) continue;

      const line = parseInt(lineStr) - 1;
      const col = parseInt(colStr) - 1;
      const severity =
        severityStr.trim().toLowerCase() === "error"
          ? vscode.DiagnosticSeverity.Error
          : vscode.DiagnosticSeverity.Warning;

      const lineText = document.lineAt(line).text;
      const nameMatch = message.match(/`([^`]+)`/);
      let range;

      if (nameMatch) {
        const name = nameMatch[1];
        // Buscar la posición real del nombre en la línea, empezando desde la columna que da el compilador
        let startIdx = lineText.indexOf(name, col);
        if (startIdx === -1) startIdx = lineText.indexOf(name); // Fallback si col no coincide

        if (startIdx !== -1) {
          range = new vscode.Range(
            line,
            startIdx,
            line,
            startIdx + name.length,
          );
        } else {
          range = new vscode.Range(line, col, line, col + name.length);
        }
      } else {
        const pos = new vscode.Position(line, col);
        range =
          document.getWordRangeAtPosition(pos) ||
          new vscode.Range(line, col, line, col + 1);
      }

      const diagnostic = new vscode.Diagnostic(range, message, severity);

      // Si es un aviso de "no usada", aplicar transparencia y eliminar el subrayado visual
      const lowerMsg = message.toLowerCase();
      const isUnused =
        lowerMsg.includes("no usada") ||
        lowerMsg.includes("no usado") ||
        lowerMsg.includes("unused") ||
        lowerMsg.includes("no se usa") ||
        lowerMsg.includes("never used");

      if (isUnused) {
        diagnostic.tags = [vscode.DiagnosticTag.Unnecessary];
        // Al usar Hint, VS Code elimina el subrayado fuerte (squiggly)
        // y solo deja el efecto de transparencia/faded.
        diagnostic.severity = vscode.DiagnosticSeverity.Hint;
      }

      diagnostics.push(diagnostic);
    }

    // Añadir validaciones extras de la extensión (ej: prohibidos)
    const text = document.getText();
    const forbiddenWords = [
      "function",
      "while",
      "for",
      "if",
      "else",
      "class",
      "return",
    ];
    for (const word of forbiddenWords) {
      const regexForbidden = new RegExp(`\\b${word}\\b`, "g");
      let m;
      while ((match = regexForbidden.exec(text)) !== null) {
        const startPos = document.positionAt(match.index);
        const endPos = document.positionAt(match.index + word.length);
        diagnostics.push(
          new vscode.Diagnostic(
            new vscode.Range(startPos, endPos),
            `La palabra '${word}' está prohibida en Jasboot (use el equivalente en español).`,
            vscode.DiagnosticSeverity.Error,
          ),
        );
      }
    }

    collection.set(document.uri, diagnostics);
  });
}

class JasbootDefinitionProvider {
  async provideDefinition(document, position, token) {
    const range = document.getWordRangeAtPosition(position);
    if (!range) return null;

    const word = document.getText(range);
    const locations = [];

    // 1. Buscar en el archivo actual
    const text = document.getText();
    const lines = text.split(/\r?\n/);
    for (let i = 0; i < lines.length; i++) {
      const line = lines[i];
      // Patrones de declaración: funcion, clase, registro, concepto
      const patterns = [
        new RegExp(`\\b(funcion|clase|registro|concepto)\\s+${word}\\b`),
        new RegExp(
          `\\b(entero|texto|flotante|caracter|bool|lista|mapa|u32|u64|u8|byte|bytes|objeto|elemento|json|[A-Z][a-zA-Z0-9_]*)\\s+${word}\\b`,
        ),
      ];

      for (const pattern of patterns) {
        const match = line.match(pattern);
        if (match) {
          const startIdx = line.indexOf(word, match.index);
          locations.push(
            new vscode.Location(
              document.uri,
              new vscode.Range(i, startIdx, i, startIdx + word.length),
            ),
          );
        }
      }
    }

    // 2. Buscar en archivos importados (usar)
    const imports = this.parseImports(text);
    for (const imp of imports) {
      if (imp.names.includes(word)) {
        const projectRoot = resolveProjectRoot(document.uri.fsPath);
        let targetPath = path.isAbsolute(imp.path)
          ? imp.path
          : path.join(path.dirname(document.uri.fsPath), imp.path);

        if (fileExists(targetPath)) {
          const targetUri = vscode.Uri.file(targetPath);
          const targetDoc = await vscode.workspace.openTextDocument(targetUri);
          const targetText = targetDoc.getText();
          const targetLines = targetText.split(/\r?\n/);

          for (let i = 0; i < targetLines.length; i++) {
            const line = targetLines[i];
            const patterns = [
              new RegExp(`\\b(funcion|clase|registro|concepto)\\s+${word}\\b`),
              new RegExp(
                `\\b(entero|texto|flotante|caracter|bool|lista|mapa|u32|u64|u8|byte|bytes|objeto|elemento|json|[A-Z][a-zA-Z0-9_]*)\\s+${word}\\b`,
              ),
            ];
            for (const pattern of patterns) {
              const match = line.match(pattern);
              if (match) {
                const startIdx = line.indexOf(word, match.index);
                locations.push(
                  new vscode.Location(
                    targetUri,
                    new vscode.Range(i, startIdx, i, startIdx + word.length),
                  ),
                );
                break;
              }
            }
          }
        }
      }
    }

    return locations;
  }

  parseImports(text) {
    const imports = [];
    const regex = /usar\s*\{([^}]+)\}\s*de\s*"([^"]+)"/g;
    let match;
    while ((match = regex.exec(text)) !== null) {
      const names = match[1].split(",").map((s) => s.trim());
      imports.push({ names, path: match[2] });
    }
    return imports;
  }
}

class JasbootHoverProvider {
  async provideHover(document, position, token) {
    const range = document.getWordRangeAtPosition(position);
    if (!range) return null;

    const word = document.getText(range);

    // 1. Buscar declaración
    const definitionProvider = new JasbootDefinitionProvider();
    const locations = await definitionProvider.provideDefinition(
      document,
      position,
      token,
    );

    if (locations && locations.length > 0) {
      const loc = locations[0];
      const targetDoc = await vscode.workspace.openTextDocument(loc.uri);
      const line = targetDoc.lineAt(loc.range.start.line).text.trim();

      let markdown = new vscode.MarkdownString();

      // Si la línea es una declaración de variable con tipo, resolver el tipo
      // Ejemplo: "mapa mi_mapa2<MAPAS> = {" -> el tipo es MAPAS
      const typeMatch =
        line.match(/<([A-Z][a-zA-Z0-9_]*)>/) ||
        line.match(/^([A-Z][a-zA-Z0-9_]*)\s+/);
      const typeName = typeMatch ? typeMatch[1] : null;

      if (typeName && typeName !== word) {
        markdown.appendMarkdown(
          `**Variable:** \`${word}\`\n\n**Tipo:** \`${typeName}\`\n`,
        );
        // Buscar la definición del tipo (Clase o Registro)
        const typeLocations = await this.findTypeDefinition(document, typeName);
        if (typeLocations.length > 0) {
          const typeLoc = typeLocations[0];
          const typeDoc = await vscode.workspace.openTextDocument(typeLoc.uri);
          const structure = await this.extractStructure(
            typeDoc,
            typeLoc.range.start.line,
          );
          markdown.appendMarkdown(`\n**Estructura de ${typeName}:**\n`);
          markdown.appendCodeblock(structure, "jasboot");
        }
      } else {
        markdown.appendCodeblock(line, "jasboot");
        if (line.startsWith("registro") || line.startsWith("clase")) {
          const structure = await this.extractStructure(
            targetDoc,
            loc.range.start.line,
          );
          markdown.appendMarkdown("\n**Campos/Métodos:**\n");
          markdown.appendCodeblock(structure, "jasboot");
        }
      }

      return new vscode.Hover(markdown);
    }

    // 2. Buscar si es una función del sistema
    const systemFunctions = [
      "imprimir",
      "imprimir_sin_salto",
      "ingresar_texto",
      "limpiar_consola",
      "str_a_entero",
      "str_a_flotante",
      "lista_tamano",
      "lista_obtener",
      "mapa_crear",
      "mapa_poner",
      "mapa_obtener",
      "fs_leer_linea",
      "ahora",
      "codigo_caracter",
      "caracter_a_texto",
      "byte_a_caracter",
    ];

    if (systemFunctions.includes(word)) {
      return new vscode.Hover(
        new vscode.MarkdownString(
          `**Función del Sistema:** \`${word}\` (Built-in)`,
        ),
      );
    }

    return null;
  }

  async findTypeDefinition(document, typeName) {
    const text = document.getText();
    const lines = text.split(/\r?\n/);
    for (let i = 0; i < lines.length; i++) {
      if (lines[i].match(new RegExp(`\\b(clase|registro)\\s+${typeName}\\b`))) {
        return [
          new vscode.Location(
            document.uri,
            new vscode.Range(i, 0, i, typeName.length),
          ),
        ];
      }
    }
    // Buscar en imports... (reutilizar lógica de DefinitionProvider si es necesario)
    return [];
  }

  async extractStructure(doc, startLine) {
    let fields = [];
    const firstLine = doc.lineAt(startLine).text.trim();
    const endTag = firstLine.startsWith("registro")
      ? "fin_registro"
      : "fin_clase";
    for (let i = startLine + 1; i < doc.lineCount; i++) {
      const l = doc.lineAt(i).text.trim();
      if (l === endTag) break;
      if (l && !l.startsWith("#")) fields.push(l);
    }
    return fields.join("\n");
  }
}

function deactivate() {}

module.exports = {
  activate,
  deactivate,
};
