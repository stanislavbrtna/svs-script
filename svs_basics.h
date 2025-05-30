/*
Copyright (c) 2016 Stanislav Brtna

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

This is the main include file.
*/

#ifndef SVS_BASICS
#define SVS_BASICS

#ifdef PC
 #include <stdio.h>
 #include <stdint.h>
#else
 #include <stdint.h>
 #ifdef SVS_USE_PLATFORM_SPECIFIC
 #include "platform_specific.h"
 #endif
#endif

#include "svs_limits.h"
#include "svs_types.h"
#include "svs_errors.h"

#if !defined(USE_FLOAT) && defined(SVS_USE_ADV_MATH)
#error "USE_FLOAT required for SVS_USE_ADV_MATH"
#endif

#include "tokenizer/svs_token_cache.h"
#include "variables/svs_variables.h"
#include "svs_misc.h"
#include "svs_misc_str.h"
#include "svs_sys_exec.h"
#include "svs_builtin_calls.h"
#include "tokenizer/svs_tokenizer.h"
#include "expr_exec/svs_expr_exec2.h"
#include "comm_exec/svs_comm_exec.h"
#include "svs_garbage_collector.h"
#include "svs_load.h"
#include "svs_debug.h"
#include "svs_heatmap.h"

#endif
