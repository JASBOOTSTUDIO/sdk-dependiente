# Makefile para Jasboot Package Manager (JPM)
# Copyright (c) 2024 Jasboot Team

# =============================================================================
# CONFIGURACIÓN
# =============================================================================

# Compilador
CC = gcc

# Nombre del ejecutable
TARGET = jpm
ifeq ($(OS),Windows_NT)
    TARGET := $(TARGET).exe
endif

# Directorios
SRC_DIR = src
INC_DIR = include
OBJ_DIR = build
BIN_DIR = bin

# Archivos fuente
SOURCES = $(SRC_DIR)/jpm_main.c \
          $(SRC_DIR)/jpm_core.c \
          $(SRC_DIR)/jpm_json.c \
          $(SRC_DIR)/jpm_metadata.c \
          $(SRC_DIR)/jpm_commands.c \
          $(SRC_DIR)/jpm_pack.c \
          $(SRC_DIR)/jpm_install.c \
          $(SRC_DIR)/jpm_lock.c

# Archivos objeto
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Flags del compilador
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -I$(INC_DIR)

# Flags de optimización (Release vs Debug)
ifdef DEBUG
    CFLAGS += -g -O0 -DDEBUG
    BUILD_TYPE = debug
else
    CFLAGS += -O2 -DNDEBUG
    BUILD_TYPE = release
endif

# Flags del linker
LDFLAGS =

# Bibliotecas
LIBS =

# Detección de plataforma
ifeq ($(OS),Windows_NT)
    PLATFORM = Windows
    RM = del /Q /F
    RMDIR = rmdir /S /Q
    MKDIR = mkdir
    SEP = \\
    # En Windows no necesitamos flags adicionales
else
    PLATFORM = Unix
    RM = rm -f
    RMDIR = rm -rf
    MKDIR = mkdir -p
    SEP = /
    # En Unix/Linux necesitamos -lm para matemáticas
    LIBS += -lm
endif

# =============================================================================
# REGLAS PRINCIPALES
# =============================================================================

.PHONY: all clean install uninstall debug release help

# Regla por defecto
all: $(BIN_DIR)/$(TARGET)

# Compilar en modo debug
debug:
	@$(MAKE) DEBUG=1 all

# Compilar en modo release
release:
	@$(MAKE) all

# Crear ejecutable
$(BIN_DIR)/$(TARGET): $(OBJECTS) | $(BIN_DIR)
	@echo [JPM] Linking $(TARGET) ($(BUILD_TYPE))...
	@$(CC) $(OBJECTS) $(LDFLAGS) $(LIBS) -o $@
	@echo [JPM] Build successful: $@

# Compilar archivos fuente a objetos
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo [JPM] Compiling $<...
	@$(CC) $(CFLAGS) -c $< -o $@

# Crear directorios si no existen
$(OBJ_DIR):
	@echo [JPM] Creating build directory...
ifeq ($(OS),Windows_NT)
	@if not exist "$(OBJ_DIR)" $(MKDIR) $(OBJ_DIR)
else
	@$(MKDIR) $(OBJ_DIR)
endif

$(BIN_DIR):
	@echo [JPM] Creating bin directory...
ifeq ($(OS),Windows_NT)
	@if not exist "$(BIN_DIR)" $(MKDIR) $(BIN_DIR)
else
	@$(MKDIR) $(BIN_DIR)
endif

# =============================================================================
# REGLAS DE LIMPIEZA
# =============================================================================

# Limpiar archivos compilados
clean:
	@echo [JPM] Cleaning build files...
ifeq ($(OS),Windows_NT)
	@if exist "$(OBJ_DIR)" $(RMDIR) $(OBJ_DIR)
	@if exist "$(BIN_DIR)" $(RMDIR) $(BIN_DIR)
else
	@$(RMDIR) $(OBJ_DIR) $(BIN_DIR)
endif
	@echo [JPM] Clean complete

# Limpiar todo (incluyendo ejecutables)
distclean: clean
	@echo [JPM] Removing all generated files...
ifeq ($(OS),Windows_NT)
	@if exist "*.exe" $(RM) *.exe
	@if exist "*.o" $(RM) *.o
else
	@$(RM) *.exe *.o
endif

# =============================================================================
# INSTALACIÓN
# =============================================================================

# Directorio de instalación
PREFIX = /usr/local
INSTALL_DIR = $(PREFIX)/bin

# Instalar JPM en el sistema
install: $(BIN_DIR)/$(TARGET)
	@echo [JPM] Installing to $(INSTALL_DIR)...
ifeq ($(OS),Windows_NT)
	@echo Installation on Windows requires manual copying
	@echo Copy $(BIN_DIR)$(SEP)$(TARGET) to your PATH
else
	@install -d $(INSTALL_DIR)
	@install -m 755 $(BIN_DIR)/$(TARGET) $(INSTALL_DIR)/$(TARGET)
	@echo [JPM] Installation complete
endif

# Desinstalar JPM del sistema
uninstall:
	@echo [JPM] Uninstalling from $(INSTALL_DIR)...
ifeq ($(OS),Windows_NT)
	@echo Uninstallation on Windows requires manual removal
else
	@$(RM) $(INSTALL_DIR)/$(TARGET)
	@echo [JPM] Uninstallation complete
endif

# =============================================================================
# REGLAS DE UTILIDAD
# =============================================================================

# Mostrar información del build
info:
	@echo ========================================
	@echo Jasboot Package Manager (JPM) Build Info
	@echo ========================================
	@echo Platform:      $(PLATFORM)
	@echo Compiler:      $(CC)
	@echo Target:        $(TARGET)
	@echo Build Type:    $(BUILD_TYPE)
	@echo Source Dir:    $(SRC_DIR)
	@echo Object Dir:    $(OBJ_DIR)
	@echo Binary Dir:    $(BIN_DIR)
	@echo Flags:         $(CFLAGS)
	@echo Libraries:     $(LIBS)
	@echo ========================================

# Ejecutar tests básicos
test: $(BIN_DIR)/$(TARGET)
	@echo [JPM] Running basic tests...
	@$(BIN_DIR)/$(TARGET) --version || echo "(version command not implemented yet)"
	@echo [JPM] Tests complete

# Mostrar ayuda
help:
	@echo ========================================
	@echo Jasboot Package Manager (JPM) - Makefile
	@echo ========================================
	@echo Available targets:
	@echo   all         - Build JPM (default)
	@echo   debug       - Build with debug symbols
	@echo   release     - Build optimized release
	@echo   clean       - Remove build files
	@echo   distclean   - Remove all generated files
	@echo   install     - Install JPM to system
	@echo   uninstall   - Remove JPM from system
	@echo   info        - Show build information
	@echo   test        - Run basic tests
	@echo   help        - Show this help message
	@echo.
	@echo Examples:
	@echo   make              - Build release version
	@echo   make debug        - Build debug version
	@echo   make clean all    - Clean and rebuild
	@echo   make install      - Install to system
	@echo ========================================

# =============================================================================
# DEPENDENCIAS
# =============================================================================

# Dependencias automáticas de headers
$(OBJ_DIR)/jpm_main.o: $(INC_DIR)/jpm.h
$(OBJ_DIR)/jpm_core.o: $(INC_DIR)/jpm.h
$(OBJ_DIR)/jpm_json.o: $(INC_DIR)/jpm.h
$(OBJ_DIR)/jpm_metadata.o: $(INC_DIR)/jpm.h
$(OBJ_DIR)/jpm_commands.o: $(INC_DIR)/jpm.h
$(OBJ_DIR)/jpm_pack.o: $(INC_DIR)/jpm.h
$(OBJ_DIR)/jpm_install.o: $(INC_DIR)/jpm.h
$(OBJ_DIR)/jpm_lock.o: $(INC_DIR)/jpm.h

# =============================================================================
# RECONSTRUCCIÓN AUTOMÁTICA
# =============================================================================

# Reconstruir si el Makefile cambia
$(OBJECTS): Makefile
