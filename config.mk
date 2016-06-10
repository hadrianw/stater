# flags
CPPFLAGS = #-D
CFLAGS = -std=c99 -pedantic -Os -Wall ${CPPFLAGS} -march=native
LDFLAGS = -s

# compiler and linker
CC = cc
