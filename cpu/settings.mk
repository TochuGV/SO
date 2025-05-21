# Libraries
LIBS=utils commons pthread readline m

# Custom libraries' paths
SHARED_LIBPATHS=
STATIC_LIBPATHS=../utils

# Compiler flags
CDEBUG=-g -Wall - std= c99 -DDEBUG -fdiagnostics-color=always
CRELEASE=-O3 -Wall -std = c99 -DNDEBUG

# Source files (*.c) to be excluded from tests compilation
TEST_EXCLUDE=src/main.c
