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

#include "svs_sys_exec.h"

uint8_t sysExecDebug;

void setSysExecDebug (uint8_t level) {
  sysExecDebug = level;
}

//syscall wrappers array
uint8_t (*sysWrapper[SYSCALL_WRAPPERS]) (varRetVal *result, argStruct *argS, svsVM *s);

//tokenizer constants array
svsConstType *sysConsts[SYSCALL_WRAPPERS];

volatile uint8_t sysWrapperNum;
volatile uint8_t sysConstsNum;

void addSysConsts(svsConstType *consts) {
  if (sysConstsNum < SYSCALL_WRAPPERS) {
    sysConsts[sysConstsNum] = consts;
    sysConstsNum++;
  } else {
    errMsgS("addSysWrapper: Maximum number of syscall const arrays exceeded!");
  }
}

void addSysWrapper(uint8_t (*arg) (varRetVal *result, argStruct *argS, svsVM *s)) {
  if (sysWrapperNum < SYSCALL_WRAPPERS) {
    sysWrapper[sysWrapperNum] = arg;
    sysWrapperNum++;
  } else {
    errMsgS("addSysWrapper: Maximum number of syscall wrappers exceeded!");
  }
}

uint8_t sysFuncMatch(varType id, char *t, svsVM *s) {
  if (strCmp((uint8_t *)t, s->syscallTable[id.val_u].sysCallName)) {
    return 1;
  } else {
    return 0;
  }
}

uint8_t sysExecTypeCheck(argStruct *argS, uint8_t *argType, uint8_t argCnt, svsVM *s ) {
  uint8_t x;
  if (argS->usedup == argCnt) {
    for (x = 1; x <= argCnt; x++) {
      if ((argS->argType[x] != argType[x]) && (argType[x] != 3)) {
        errSoft("sysExecTypeCheck: Wrong type of argument for SYS function!", s);
        errSoftSetParam(s->syscallTable[(uint16_t)(argS->callId.val_u)].sysCallName, (varType)((uint16_t)0), s);
        errSoftSetParam("Argument number", (varType)((uint16_t)x), s);
        return 1;
      }
    }
  } else {
    errSoft("sysExecTypeCheck: Wrong number of arguments for SYS function!", s);
    errSoftSetParam(s->syscallTable[argS->callId.val_u].sysCallName, (varType)((uint16_t)0), s);
    errSoftSetParam("Expected", (varType)((uint16_t)argCnt), s);
    errSoftSetParam("Got", (varType)((uint16_t)argS->usedup), s);
    return 1;
  }
  return 0;
}

void sysExec(uint16_t index, varRetVal *result, svsVM *s) {
  /*
   * Is called from command execute (commEx) or expression execute (exprEx)
   * exexutes "sys" command and returns result.
   *
   * Zavolá se z commEx a nebo exprEx, hodí se jí index tokenu asi hned závorky za sys tokenem,
   * ona to vykoná a vrátí do resultu id
   *
  */

  varRetVal pracVar;
  argStruct argS;
  uint16_t x;
  uint8_t retval = 0;
  errStruct err;

  argS.callId = getTokenData(index, s);

  index++;

  x = 1;

  if ((getTokenType(index, s) == 5)) {
    index++;
    if (getTokenType(index, s) != 6) { // if the brackets are not empty
      exprExec(index, &pracVar, s); // execute expression
      if (errCheck(s)) {
        return;
      }
      argS.arg[x] = pracVar.value; // set it's value as an argument
      argS.argType[x] = pracVar.type;
      index = pracVar.tokenId; // store token index
      x++;
    }

    // arguments are separated with coma or semicolon(legacy)
    while((getTokenType(index, s) == 9) || (getTokenType(index, s) == 33)) {
      if (x == FUNCTION_ARGS_MAX + 1) {
        errSoft("sysExec: too many arguments in sys call!", s);
        errSoftSetParam(s->syscallTable[argS.callId.val_u].sysCallName, (varType)((uint16_t)0), s);
        errSoftSetParam("TokenId", (varType)index, s);
        errSoftSetToken(index, s);
        return;
      }
      index++;
      exprExec(index, &pracVar, s);
      if (errCheck(s)) { // check for error inside expression
        return;
      }
      argS.arg[x] = pracVar.value;
      argS.argType[x] = pracVar.type;
      index = pracVar.tokenId;
      x++;
    }

    if (getTokenType(index,s) != 6) {
      errSoft("sysExec: Syntax error at end of sys call. (missing \")\")", s);
      errSoftSetParam(s->syscallTable[argS.callId.val_u].sysCallName, (varType)((uint16_t)0), s);
      errSoftSetParam("TokenId", (varType)index, s);
      errSoftSetToken(index, s);
      return;
    }

    argS.usedup = x - 1;

    index++;
    varRetValZero(&pracVar);

    // run the sys wrappers
    for (x = 0; x < sysWrapperNum; x++) {
      retval = (*sysWrapper[x]) (&pracVar, &argS, s);
      if (retval == 1) {
        break;
      }
    }

    // if all wrappers failed
    if (retval == 0) {
      if (errCheck(s) == 0) {
        errSoft("sysExec: Unknown sys call!", s);
        errSoftSetParam(s->syscallTable[argS.callId.val_u].sysCallName, (varType)((uint16_t)0), s);
        errSoftSetParam("TokenId", (varType)index, s);
        errSoftSetToken(index, s);
        return;
      } else {
        // there was error inside wrapper (typecheck etc..)
        return;
      }
    } else {
      result->value = pracVar.value;
      result->type = pracVar.type;
      result->tokenId = index;
    }
  } else {
    errSoft("sysExec: Syntax error at the begin of sys call. (missing \"(\")", s);
    errSoftSetParam(s->syscallTable[argS.callId.val_u].sysCallName, (varType)((uint16_t)0), s);
    errSoftSetParam("TokenId", (varType)index, s);
    errSoftSetToken(index, s);
    return;
  }
}
