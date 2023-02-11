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
*/

#ifndef SVS_SYS_EXEC_H
#define SVS_SYS_EXEC_H

#include "svs_basics.h"

typedef struct {
  VARTYPE callId;                        // id of a sys call
  VARTYPE arg[FUNCTION_ARGS_MAX+1];      // argument values
  uint8_t argType[FUNCTION_ARGS_MAX+1];  // argument types
  uint8_t usedup;                        // count of used up arguments
} argStruct;

typedef struct {
  const char* name;
  int32_t value;
} svsConstType;

extern svsConstType * sysConsts[SYSCALL_WRAPPERS];
extern volatile uint8_t sysConstsNum;

uint8_t sysExecTypeCheck(argStruct *argS, uint8_t *argType, uint8_t argCnt, svsVM *s );
void addSysWrapper(uint8_t (*arg) (varRetVal *result, argStruct *argS, svsVM *s), uint8_t *wrapperName);
uint8_t sysFuncMatch(varType id, char *t, svsVM *s);
void sysExec(uint16_t index, varRetVal *result, svsVM *s);
void addSysConsts(svsConstType * consts);
uint16_t getSysWrapperId(uint8_t *name);

#endif
