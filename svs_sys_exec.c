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

// syscall wrappers array
uint8_t (*sysWrapper[SYSCALL_WRAPPERS]) (varRetVal *result, argStruct *argS, svsVM *s);

// sys wrapper names
uint8_t *sysWrapperName[SYSCALL_WRAPPERS];

// tokenizer constants array
svsConstType *sysConsts[SYSCALL_WRAPPERS];

volatile uint8_t sysWrapperNum;
volatile uint8_t sysConstsNum;


void setSysExecDebug (uint8_t level) {
  sysExecDebug = level;
}


void addSysConsts(svsConstType *consts) {
  if (sysConstsNum < SYSCALL_WRAPPERS) {
    sysConsts[sysConstsNum] = consts;
    sysConstsNum++;
  } else {
    errMsgS((uint8_t *)"addSysWrapper: Maximum number of syscall const arrays exceeded!");
  }
}


void addSysWrapper(uint8_t (*arg) (varRetVal *result, argStruct *argS, svsVM *s), uint8_t *wrapperName) {
  if (sysWrapperNum < SYSCALL_WRAPPERS) {
    sysWrapper[sysWrapperNum] = arg;
    sysWrapperName[sysWrapperNum] = wrapperName;
    sysWrapperNum++;
  } else {
    errMsgS((uint8_t *)"addSysWrapper: Maximum number of syscall wrappers exceeded!");
  }
}


uint16_t getSysWrapperId(uint8_t *name) {
  for (uint16_t x = 0; x < sysWrapperNum; x++) {
    if (strCmp(name, sysWrapperName[x])){
      return x + 1;
    }
  }
  return 0;
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
        errSoft((uint8_t *)"sysExecTypeCheck: Wrong type of argument for SYS function!", s);
        errSoftSetParam(s->syscallTable[(uint16_t)(argS->callId.val_u)].sysCallName, (varType)((uint16_t)0), s);
        errSoftSetParam((uint8_t *)"Argument number", (varType)((uint16_t)x), s);
        return 1;
      }
    }
  } else {
    errSoft((uint8_t *)"sysExecTypeCheck: Wrong number of arguments for SYS function!", s);
    errSoftSetParam(s->syscallTable[argS->callId.val_u].sysCallName, (varType)((uint16_t)0), s);
    errSoftSetParam((uint8_t *)"Expected", (varType)((uint16_t)argCnt), s);
    errSoftSetParam((uint8_t *)"Got", (varType)((uint16_t)argS->usedup), s);
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
  uint16_t preselected_wrapper = 0;

  x = 1;

  // starts on the first sys token, get if there is a second one with a wrapper name
  if ((getTokenType(index + 1, s) == SVS_TOKEN_SYS)) {
    // wrapper pre-selection
    preselected_wrapper = (getTokenData(index, s)).val_u + 1;
    index++;
  }

  // get call id and move to the bracket (hopefully)
  argS.callId = getTokenData(index, s);
  index++;

  if ((getTokenType(index, s) == SVS_TOKEN_LBR)) {
    index++;
    if (getTokenType(index, s) != SVS_TOKEN_RBR) { // if the brackets are not empty
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
    while((getTokenType(index, s) == SVS_TOKEN_SCOL) || (getTokenType(index, s) == SVS_TOKEN_COL)) {
      if (x == FUNCTION_ARGS_MAX + 1) {
        errSoft((uint8_t *)"sysExec: too many arguments in sys call!", s);
        errSoftSetParam(s->syscallTable[argS.callId.val_u].sysCallName, (varType)((uint16_t)0), s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)index, s);
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

    if (getTokenType(index,s) != SVS_TOKEN_RBR) {
      errSoft((uint8_t *)"sysExec: Syntax error at end of sys call. (missing \")\")", s);
      errSoftSetParam(s->syscallTable[argS.callId.val_u].sysCallName, (varType)((uint16_t)0), s);
      errSoftSetParam((uint8_t *)"TokenId", (varType)index, s);
      errSoftSetToken(index, s);
      return;
    }

    argS.usedup = x - 1;

    index++;
    varRetValZero(&pracVar);

    if (preselected_wrapper == 0) {
    // run the sys wrappers
      for (x = 0; x < sysWrapperNum; x++) {
        retval = (*sysWrapper[x]) (&pracVar, &argS, s);
        if (retval == 1) {
          break;
        }
      }
    } else {
      retval = (*sysWrapper[preselected_wrapper - 1]) (&pracVar, &argS, s);
    }

    // if all wrappers failed
    if (retval == 0) {
      if (errCheck(s) == 0) {
        errSoft((uint8_t *)"sysExec: Unknown sys call!", s);
        errSoftSetParam(s->syscallTable[argS.callId.val_u].sysCallName, (varType)((uint16_t)0), s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)index, s);
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
    errSoft((uint8_t *)"sysExec: Syntax error at the begin of sys call. (missing \"(\")", s);
    errSoftSetParam(s->syscallTable[argS.callId.val_u].sysCallName, (varType)((uint16_t)0), s);
    errSoftSetParam((uint8_t *)"TokenId", (varType)index, s);
    errSoftSetToken(index, s);
    return;
  }
}
