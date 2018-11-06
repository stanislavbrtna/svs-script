CC = gcc

# define any compile-time flags
CFLAGS = -std=c99

DEFINES = -std=c99 -lm -DPC -DCMDLINE -DTOKEN_CACHE_DISABLED

SRCS = svs_pc.c svs_misc.c svs_comm_exec.c svs_token_cache.c svs_errors.c svs_tokenizer.c svs_expr_exec2.c svs_garbage_collector.c svs_sys_exec.c svs_sys_wrapper_pc.c svs_load.c svs_misc_str.c svs_builtin_calls.c svs_debug.c

all: svs test

svs:
	$(CC) $(CFLAGS) $(SRCS) $(DEFINES) -DLANG_CZ -o bin/svs

test: svs
	./bin/svs tests/autotest.svs
	./bin/svs tests/gctest.svs
	./bin/svs tests/optest.svs
