CC = gcc
MKDIR_P ?= mkdir -p
RM = rm

# define any compile-time flags
CFLAGS = -std=c99

LIBS = -lm

DEFINES = -std=c99 -DPC -DCMDLINE -DTOKEN_CACHE_DISABLED

SRCS = $(shell find "." -name "*.c")

BUILD_DIR ?= ./build

OBJS := $(addprefix $(BUILD_DIR),$(addprefix /, $(addsuffix .o,$(basename $(SRCS)))))

all: svs test

$(BUILD_DIR)/%.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) $(DEFINES) -c $< -o $@

svs: $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o bin/svs

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

test: svs
	./bin/svs tests/autotest.svs
	./bin/svs tests/gctest.svs
	./bin/svs tests/optest.svs
	./bin/svs tests/optest_arg.svs
	./bin/svs tests/array-gc-test.svs
	./bin/svs tests/import.svs
	./bin/svs tests/snipplets.svs
	@echo "All tests PASSED"
