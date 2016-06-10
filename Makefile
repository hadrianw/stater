include config.mk

SRC = stater.c
OBJ = ${SRC:.c=.o}

all: stater

stater: $(SRC:.c=.o)
	@echo CC -o $@
	@$(CC) -o $@ $+ $(LDFLAGS)

.c.o:
	@echo CC -c $<
	@$(CC) -c $< $(CFLAGS)

$(OBJ): config.h config.mk

config.h:
	cp config.def.h $@

clean:
	@echo cleaning
	@rm -f stater *.o

.PHONY: all clean
