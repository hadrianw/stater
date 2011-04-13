TARGET = stater
SRCS := $(wildcard *.c)
EXCLUDE =

# includes and libs
INCS = 
LIBS = 

# flags
CPPFLAGS = #-D
CFLAGS   = -std=c99 -pedantic -Os -Wall ${INCS} ${CPPFLAGS} -march=native
LDFLAGS  = -s ${LIBS}

# compiler and linker
CC = cc
