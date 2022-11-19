/*
Copyright (c) 2022 Stanislav Brtna

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

#include "svs_variables.h"

static uint32_t warncount;

void resetWarnCount() {
  warncount = 0;
}

uint8_t getUndefWarning() {
  warncount++;
  if (warncount < 10) {
    return 1;
  }

  if (warncount == 10) {
    printf("No more warnings like this will be shown in this run.\n");
  }
  return 0;
}

void varRetValZero(varRetVal *x) {
  x->value.val_s = 0;
  x->type = 0;
  x->tokenId = 0;
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
    errMsgS((uint8_t *)"varGetIdFromMask: variable not found!");
    errHalt();
    return (varType)((uint16_t)0);
  }
}

void varAddLocal(varType Id, svsVM *s) {
  if (s->varTableLen == VAR_TABLE_L) {
    errSoft((uint8_t *)"varAddLocal: varTable full!", s);
    errSoftSetParam((uint8_t *)"VarId", Id, s);
    return;
  }

  s->varTableLen++;
  s->varTable[s->varTableLen].maskId = Id.val_u;
  s->varTable[s->varTableLen].value.val_s = (int32_t)0;
  s->varTable[s->varTableLen].type = 0;
}

#else
varType varGetIdFromMask(varType Id, svsVM *s) {
  return Id;
}

void varAddLocal(varType Id, svsVM *s) {
  errSoft((uint8_t *)"varAddLocal: Local variables are disabled in this build!", s);
  errSoftSetParam((uint8_t *)"VarId", Id, s);
  return;
}

#endif

varType newArray(varType Id, uint16_t len, svsVM *s) {
  varType retval;
  (void)(Id);
  if (len > (SVS_ARRAY_LEN - s->varArrayLen)) {
    errSoft((uint8_t *)"newArray: Array field full!", s);
    return (varType)((uint16_t)0);
  }
  // first element of array is its lenght
  s->varArray[s->varArrayLen] = (varType)len;
  retval = (varType)s->varArrayLen;
  s->varArrayLen++;
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

  errMsgS((uint8_t *)"varGetType: variable not found!");
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
  errMsgS((uint8_t *)"varSetType: variable not found!");
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
  errMsgS((uint8_t *)"varGetVal: variable not found!");
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
  errMsgS((uint8_t *)"varSetVal: variable not found!");
  return 0;
}