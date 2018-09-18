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

#include "svs_misc.h"

void svsReset(svsVM *s){
  uint32_t x;
  s->funcTableLen = 0;
  s->varTableLen = 0;
  s->stringFieldLen = 0;
  s->stringConstMax = 0;
  s->gcSafePoint = 0;

#ifdef PC
  s->vmCache = 0;
#endif
  s->tokenMax = 1;
  s->vmCacheUsed = 0;
  s->cacheStart = 0;

  s->handbrake = 0;
  s->syscallTableLen = 0;

  commArgNull(&s->commArgs);

  s->commFlag = 0;
  s->commRetVal.val_u = 0;
  s->commRetType = 0;
  s->commRetFlag = 0;
  s->progLine = 0;
  s->globalDebug = 0;

  s->err = 0;
  s->errToken = 0;
  s->errDbgUsed = 0;

  s->profilerEnabled = 0;
  setCommExDebug(0);
  setExprExecDebug(0);

  for (x = 0; x < VAR_TABLE_L; x++) {
    s->varTable[x].value.val_u = 0;
    s->varTable[x].maskId = 0;
  }

  s->varArrayLen = 1;
  for (x = 0; x < SVS_ARRAY_LEN; x++) {
    s->varArray[x].val_u = 0;
  }

  for (x = 0; x < SVS_ARRAY_LEN; x++) {
    s->varArrayType[x] = 0;
  }

  svsSetName("def.b", s);
  svsSetFileName("def.svs", s);
}

void svsInfo(svsVM *s) {
  printf("\nS! script v. %s\n", SVS_VERSION);
  #ifdef PC
  printf("Platform: PC\n");
  #else
  printf("Platform: uC\n");
  #endif
  printf("Limits:\n");
  printf("TOKEN_LENGTH %u\n", TOKEN_LENGTH);
  printf("NAME_LENGTH  %u\n", NAME_LENGTH);
  printf("FUNCTION_TABLE_L %u\n", FUNCTION_TABLE_L);
  printf("VAR_TABLE_L %u\n", VAR_TABLE_L);
  printf("STRING_FIELD_L %u (%u used)\n", STRING_FIELD_L, s->stringFieldLen);
  printf("SYSCALL_TABLE_L %u\n", SYSCALL_TABLE_L);
  printf("SYSCALL_WRAPPERS %u\n", SYSCALL_WRAPPERS);
  printf("SVS_ARRAY_LEN %u\n", SVS_ARRAY_LEN);
  printf("Compiled: %s %s\n(c) Standa\n\nEnjoy!\n", __DATE__, __TIME__);
}

void svsSetName(uint8_t * name, svsVM *s) {
  uint16_t x;
  for(x = 0; x < NAME_LENGTH; x++) {
    s->vmName[x] = name[x];
  }
}

void svsSetFileName(uint8_t * name, svsVM *s) {
  uint16_t x;
  for(x = 0; x < FILE_NAME_L; x++) {
    s->fName[x] = name[x];
  }
}

void varRetValZero(varRetVal *x) {
  x->value.val_s = 0;
  x->type = 0;
  x->tokenId = 0;
}

uint8_t syscallExists(uint8_t *name, svsVM *s) {
  uint16_t x = 0;
  if (s->syscallTableLen != 0) {
    for(x = 1; x <= s->syscallTableLen; x++) {
      if (strCmp(name, s->syscallTable[x].sysCallName)) {
        return 1;
      }
    }
  }
  return 0;
}

uint8_t syscallGetId(uint8_t *name, svsVM *s) {
  uint16_t x = 0;
  if (s->syscallTableLen != 0) {
    for (x = 1; x <= s->syscallTableLen; x++) {
      if (strCmp(name, s->syscallTable[x].sysCallName)){
        return x;
      }
    }
  }
  return 0;
}

uint8_t functionExists(uint8_t *name, svsVM *s) {
  uint16_t x = 0;
  if (s->funcTableLen != 0) {
    for (x = 1; x <= s->funcTableLen; x++) {
      if (strCmp(name, s->funcTable[x].fString)) {
        return 1;
      }
    }
  }
  return 0;
}

uint8_t functionGetId(uint8_t *name, svsVM *s) {
  uint16_t x = 0;
  if (s->funcTableLen != 0) {
    for (x = 1; x <= s->funcTableLen; x++) {
      if (strCmp(name, s->funcTable[x].fString)) {
        return x;
      }
    }
  }
  return 0;
}

uint8_t varExists(uint8_t *name, svsVM *s) {
  uint16_t x = 0;
  if (s->varTableLen != 0) {
    for (x = 1; x <= s->varTableLen; x++) {
      if (strCmp(name, s->varTable[x].name)) {
        return 1;
      }
    }
  }
  return 0;
}

#ifdef LOCAL_VARIABLES_ENABLED
varType varGetIdFromMask(varType Id, svsVM *s) {
  uint16_t x = 0;
  if (s->varTableLen != 0) {
    for (x = s->varTableLen; x >= 1; x--) { // projdem vartable pozpátku a hledáme maskId shodné s Id
      //printf("comparing %u and %u\n",s->varTable[x].maskId, Id.val_u);
      if (s->varTable[x].maskId == Id.val_u) {
        //printf("Local variable found: id=%u\n",x);
        return (varType)x;
      }
    }
    //printf("Local variable NOT found: id=%u\n",Id.val_u);
    return (varType)Id; //pokud nenajdeme, tak vrátíme id
  } else {
    errMsgS("varGetIdFromMask: variable not found!");
    errHalt();
    return (varType)((uint16_t)0);
  }
}

void varAddLocal(varType Id, svsVM *s) {
  if (s->varTableLen == VAR_TABLE_L) {
    errSoft("varAddLocal: varTable full!", s);
    errSoftSetParam("VarId", Id, s);
    return;
  }

  s->varTableLen++;
  s->varTable[s->varTableLen].maskId = Id.val_u;
  s->varTable[s->varTableLen].value.val_s = (int32_t)0;
  s->varTable[s->varTableLen].type = 0;
  //printf("Local var added:\n");
}

#else
varType varGetIdFromMask(varType Id, svsVM *s) {
  return Id;
}

void varAddLocal(varType Id, svsVM *s) {
  errSoft("varAddLocal: Local variables are disabled in this build!", s);
  errSoftSetParam("VarId", Id, s);
  return;
}

#endif

varType newArray(varType Id, uint16_t len, svsVM *s) {
  varType retval;
  if (len > (SVS_ARRAY_LEN - s->varArrayLen)) {
    errSoft("newArray: Array field full!", s);
    return (varType)((uint16_t)0);
  }
  retval = (varType)s->varArrayLen;
  s->varArrayLen += len;

  return retval;
}

VARTYPE varGetId(uint8_t *name, svsVM *s) {
  uint16_t x = 0;
  if (s->varTableLen != 0) {
    for(x = 1; x <= s->varTableLen; x++) {
      if (strCmp(name, s->varTable[x].name)) {
        return (varType)x;
      }
    }
  }
  return (varType)((uint16_t)0);
}

uint8_t varGetType(VARTYPE id, svsVM *s) {
  #ifdef LOCAL_VARIABLES_ENABLED
  id = varGetIdFromMask(id, s);
  #endif
  if (id.val_u <= s->varTableLen) {
    return s->varTable[id.val_u].type;
  }

  errMsgS("varGetType: variable not found!");
  errHalt();
  return 0;
}

uint8_t varSetType(VARTYPE id, uint8_t type, svsVM *s) {
  #ifdef LOCAL_VARIABLES_ENABLED
  id = varGetIdFromMask(id, s);
  #endif
  if (id.val_u <= s->varTableLen) {
    s->varTable[id.val_u].type = type;
    return 0;
  }
  errMsgS("varSetType: variable not found!");
  errHalt();
  return 1;
}

VARTYPE varGetVal(VARTYPE id, svsVM *s) {
  #ifdef LOCAL_VARIABLES_ENABLED
  id = varGetIdFromMask(id, s);
  #endif
  if (id.val_u <= s->varTableLen) {
    //printf("varGetVal id:%u value:%u \n",id,varTable[id].value);
    return (varType)s->varTable[id.val_u].value;
  }
  errMsgS("varGetVal: variable not found!");
  errHalt();
  return (varType)((uint16_t) 0);
}

uint8_t varSetVal(VARTYPE id, VARTYPE val, svsVM *s) {
  #ifdef LOCAL_VARIABLES_ENABLED
  id = varGetIdFromMask(id, s);
  #endif
  if (id.val_u <= s->varTableLen) {
    s->varTable[id.val_u].value.val_s = val.val_s;
    return 1;
  }
  errMsgS("varSetVal: variable not found!");
  return 0;
}

uint8_t isNumber(uint8_t x) {
  if  ((x >= '0') && (x <= '9')) {
    return 1;
  } else {
    return 0;
  }
}

uint8_t isRegChar(uint8_t x) {
  if (((x >= 65) && (x <= 90)) || ((x >= 97) && (x <= 122)) || (x == '_')) {
    return 1;
  } else {
    return 0;
  }
}

int32_t getRetValInt(svsVM *s){
  return s->commRetVal.val_s;
}

uint8_t * getRetValStr(svsVM *s){
  return (uint8_t *)(s->stringField+(s->commRetVal.val_u));
}

float getRetValFlt(svsVM *s){
  return s->commRetVal.val_f;
}

float exp_helper(uint16_t a, uint16_t ex) {
  uint16_t x;
  float val = (float)a;
  for(x = 1; x < ex; x++) {
    val *= (float)a;
  }
  return val;
}
