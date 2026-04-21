#!/bin/bash
# Script de compilación completa de la VM de Jasboot
# Incluye integración con JMN Core y flags correctos para búsqueda introspectiva

set -e  # Salir si hay error

echo "╔════════════════════════════════════════════════════════════╗"
echo "║   COMPILACIÓN COMPLETA DE JASBOOT VM CON JMN CORE         ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Directorios
VM_DIR="."
JMN_DIR="../jasboot-jmn-core"
BUILD_DIR="build"
BIN_DIR="bin"

# Flags de compilación
CFLAGS="-std=c11 -Wall -Wextra -O3"
INCLUDES="-Isrc -I${JMN_DIR}/src -I${JMN_DIR}/src/memoria_neuronal"
DEFINES="-DJASBOOT_LANG_INTEGRATION"
LDFLAGS="-lm"

# Windows específico
if [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
    LDFLAGS="${LDFLAGS} -lws2_32"
fi

echo -e "${BLUE}[1/5] Preparando directorios...${NC}"
mkdir -p ${BUILD_DIR}
mkdir -p ${BIN_DIR}
echo -e "${GREEN}✓ Directorios creados${NC}"
echo ""

# ═══════════════════════════════════════════════════════════════
# COMPILAR JMN CORE
# ═══════════════════════════════════════════════════════════════
echo -e "${BLUE}[2/5] Compilando JMN Core...${NC}"

JMN_SOURCES=(
    "src/memoria_neuronal/memoria_neuronal_core.c"
    "src/memoria_neuronal/memoria_neuronal_nodos.c"
    "src/memoria_neuronal/memoria_neuronal_conexiones.c"
    "src/memoria_neuronal/memoria_neuronal_busqueda.c"
    "src/memoria_neuronal/memoria_neuronal_texto_fix.c"
    "src/memoria_neuronal/memoria_neuronal_io.c"
    "src/memoria_neuronal/memoria_neuronal_estructuras.c"
    "src/memoria_neuronal/memoria_neuronal_cognitivo.c"
    "src/memoria_neuronal/memoria_neuronal_utilidades.c"
    "src/platform_compat.c"
)

JMN_OBJECTS=()
for src in "${JMN_SOURCES[@]}"; do
    obj_name=$(basename "$src" .c).o
    obj_path="${BUILD_DIR}/jmn_${obj_name}"

    echo -e "  ${YELLOW}→${NC} Compilando $(basename $src)..."
    gcc ${CFLAGS} ${INCLUDES} ${DEFINES} -c "${JMN_DIR}/${src}" -o "${obj_path}"

    JMN_OBJECTS+=("${obj_path}")
done

echo -e "${GREEN}✓ JMN Core compilado (${#JMN_OBJECTS[@]} archivos)${NC}"
echo ""

# ═══════════════════════════════════════════════════════════════
# COMPILAR VM
# ═══════════════════════════════════════════════════════════════
echo -e "${BLUE}[3/5] Compilando VM de Jasboot...${NC}"

VM_SOURCES=(
    "src/vm.c"
    "src/ir_format.c"
    "src/ir_vm.c"
    "src/reader_ir.c"
    "src/cognitive_stubs.c"
)

VM_OBJECTS=()
for src in "${VM_SOURCES[@]}"; do
    obj_name=$(basename "$src" .c).o
    obj_path="${BUILD_DIR}/${obj_name}"

    echo -e "  ${YELLOW}→${NC} Compilando $(basename $src)..."
    gcc ${CFLAGS} ${INCLUDES} ${DEFINES} -c "${src}" -o "${obj_path}"

    VM_OBJECTS+=("${obj_path}")
done

echo -e "${GREEN}✓ VM compilada (${#VM_OBJECTS[@]} archivos)${NC}"
echo ""

# ═══════════════════════════════════════════════════════════════
# ENLAZAR TODO
# ═══════════════════════════════════════════════════════════════
echo -e "${BLUE}[4/5] Enlazando ejecutable...${NC}"

ALL_OBJECTS=("${VM_OBJECTS[@]}" "${JMN_OBJECTS[@]}")
EXECUTABLE="${BIN_DIR}/jasboot-ir-vm.exe"

echo -e "  ${YELLOW}→${NC} Enlazando ${#ALL_OBJECTS[@]} archivos objeto..."
gcc ${CFLAGS} -o "${EXECUTABLE}" "${ALL_OBJECTS[@]}" ${LDFLAGS}

echo -e "${GREEN}✓ Ejecutable creado: ${EXECUTABLE}${NC}"
echo ""

# ═══════════════════════════════════════════════════════════════
# VERIFICACIÓN
# ═══════════════════════════════════════════════════════════════
echo -e "${BLUE}[5/5] Verificando compilación...${NC}"

# Verificar tamaño
SIZE=$(du -h "${EXECUTABLE}" | cut -f1)
echo -e "  ${GREEN}✓${NC} Tamaño del ejecutable: ${SIZE}"

# Verificar símbolos de búsqueda introspectiva
echo -e "  ${YELLOW}→${NC} Verificando símbolos de búsqueda introspectiva..."

SYMBOLS_FOUND=0
if strings "${EXECUTABLE}" 2>/dev/null | grep -q "jmn_buscar_introspectiva"; then
    echo -e "    ${GREEN}✓${NC} jmn_buscar_introspectiva"
    SYMBOLS_FOUND=$((SYMBOLS_FOUND + 1))
fi

if strings "${EXECUTABLE}" 2>/dev/null | grep -q "jmn_buscar_introspectiva_lista"; then
    echo -e "    ${GREEN}✓${NC} jmn_buscar_introspectiva_lista"
    SYMBOLS_FOUND=$((SYMBOLS_FOUND + 1))
fi

if strings "${EXECUTABLE}" 2>/dev/null | grep -q "jmn_buscar_introspectiva_cs"; then
    echo -e "    ${GREEN}✓${NC} jmn_buscar_introspectiva_cs"
    SYMBOLS_FOUND=$((SYMBOLS_FOUND + 1))
fi

if strings "${EXECUTABLE}" 2>/dev/null | grep -q "jmn_buscar_introspectiva_detallada"; then
    echo -e "    ${GREEN}✓${NC} jmn_buscar_introspectiva_detallada"
    SYMBOLS_FOUND=$((SYMBOLS_FOUND + 1))
fi

# Verificar mensajes de debug
if strings "${EXECUTABLE}" 2>/dev/null | grep -q "JMN BUSQUEDA"; then
    echo -e "    ${GREEN}✓${NC} Mensajes de debug JMN presentes"
    SYMBOLS_FOUND=$((SYMBOLS_FOUND + 1))
fi

echo ""

if [ $SYMBOLS_FOUND -ge 4 ]; then
    echo -e "${GREEN}╔════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║          ✓ COMPILACIÓN EXITOSA                            ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    echo -e "Ejecutable: ${GREEN}${EXECUTABLE}${NC}"
    echo ""
    echo -e "Para probar:"
    echo -e "  ${YELLOW}JASBOOT_DEBUG=1 ${EXECUTABLE} test.jbo${NC}"
    echo ""
    echo -e "Funciones disponibles en Jasboot:"
    echo -e "  • ${BLUE}buscar_en_memoria(termino)${NC}"
    echo -e "  • ${BLUE}buscar_en_memoria_lista(termino, max)${NC}"
    echo -e "  • ${BLUE}buscar_en_memoria_cs(termino, case_sensitive)${NC}"
    echo -e "  • ${BLUE}buscar_en_memoria_detallada(termino, max, cs)${NC}"
    echo ""
else
    echo -e "${RED}╔════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${RED}║          ⚠ ADVERTENCIA                                    ║${NC}"
    echo -e "${RED}╚════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    echo -e "${YELLOW}Solo se encontraron ${SYMBOLS_FOUND}/5 símbolos esperados.${NC}"
    echo -e "${YELLOW}La VM puede no tener todas las funciones de búsqueda.${NC}"
    echo ""
fi

# Crear copia de respaldo
cp "${EXECUTABLE}" "${BIN_DIR}/jasboot-ir-vm-backup.exe" 2>/dev/null || true

exit 0
