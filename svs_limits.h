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

This file contains the main defines and limits of SVS.
*/

#ifndef SVS_LIMITS_H
#define SVS_LIMITS_H

/*============================= Versions =====================================*/

#define SVS_VERSION  "1.7.1"
#define SVS_VERSION_NUM 1710

/*============================= Local Config =================================*/

#ifndef TOKEN_CACHE_DISABLED
	#define TOKEN_CACHE_DISABLED 0
#endif

#if TOKEN_CACHE_DISABLED == 0
	#define TOKEN_LENGTH 2048 // size of token cache
	#define TOKEN_CACHE_STEP 128 // size of step for chaching the swap file
#else
  #ifndef TOKEN_LENGTH
	  #define TOKEN_LENGTH 35000 // token chache size for disabled swap file
  #endif
#endif

#ifndef SVS_ARRAY_LEN
  #define SVS_ARRAY_LEN 400
#endif

#ifndef NAME_LENGTH
  #define NAME_LENGTH 15 // max name length (functions&variables)
#endif

#ifndef SVS_FILE_NAME_L
  #define SVS_FILE_NAME_L 128
#endif

#ifndef FUNCTION_TABLE_L
  #define FUNCTION_TABLE_L 50 // počet funkcí v jednom souboru / max functions in one file
#endif

#ifndef VAR_TABLE_L
  #define VAR_TABLE_L 200 // počet promněnných / max number of variables
#endif

#define SVS_USE_SEPARATE_STRING_FIELD // when defined, string field must be stored outside of the svm struct

#ifndef STRING_FIELD_L
  #define STRING_FIELD_L 30000 // počet znaků stringové pamněti / max number of characters in string memory
#endif

#ifndef GC_THRESHOLD
  #define GC_THRESHOLD 25000 // threshold for garbage collection of strings
#endif

#ifndef SYSCALL_TABLE_L
  #define SYSCALL_TABLE_L 100  // kolik různých sys volání může obsahovat jeden vm / max number of different syscalls in one file
#endif

#ifndef SYSCALL_WRAPPERS
  #define SYSCALL_WRAPPERS 5 // maximální počet wrapperů / max number of syscall wrappers
#endif

#ifndef FUNCTION_ARGS_MAX
  #define FUNCTION_ARGS_MAX 10 // max number of arguments, do not change
#endif

#ifndef SVS_FILENAME_L
  #define SVS_FILENAME_L 128
#endif

#define LOCAL_VARIABLES_ENABLED // enable local statement
#define USE_FLOAT // enable floating point math
#define SVS_USE_ADV_MATH // enable sin, cos, tan, arctg, log, exp, pwr, pi, sqrt, float must be enabled

#ifdef CMDLINE
#define SVS_RND_FUNCTION rand()
#endif
//#define DBG_DISABLED // enable additional debug
//#define CACHE_SIMPLE // disable advanced caching

//#define SVS_DBG_HEATMAP

#endif
