#!/bin/bash
gcc svs_pc.c svs_misc.c svs_comm_exec.c svs_token_cache.c svs_errors.c svs_tokenizer.c svs_expr_exec2.c svs_garbage_collector.c svs_sys_exec.c svs_sys_wrapper_pc.c svs_load.c svs_misc_str.c -std=c99 -DPC -DCMDLINE -DTOKEN_CACHE_DISABLED -o bin/svs

if [ $? -eq 0 ]; then
	./bin/svs tests/autotest.svs
	if [ $? -ne 0 ]; then
  	  echo "AUTOTEST FAILED"
	fi
fi
