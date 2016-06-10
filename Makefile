include config.mk

SRCS = stater.c

all: stater

stater: $(SRCS:.c=.o)
	@echo CC -o $@
	@$(CC) -o $@ $+ $(LDFLAGS)

%.o: %.c config.mk
	@echo CC -c $<
	@$(CC) -c $< $(CFLAGS)

clean:
	@echo cleaning
	@rm -f stater *.o

.PHONY: run all clean
