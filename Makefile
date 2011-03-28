include config.mk

SRCS := $(filter-out $(EXCLUDE),$(SRCS))

run: $(TARGET)
	./$(TARGET)

all: $(TARGET)

$(TARGET): $(SRCS:.c=.o)
	@echo CC -o $@
	@$(CC) -o $@ $+ $(LDFLAGS)

%.o: %.c config.mk
	@echo CC -c $<
	@$(CC) -c $< $(CFLAGS)

clean:
	@echo cleaning
	@rm -f $(TARGET) *.o *.d

.PHONY: run all clean
