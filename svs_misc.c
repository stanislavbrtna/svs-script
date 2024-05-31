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

void svsReset(svsVM *s) {
  uint32_t x;
  s->funcTableLen = 0;
  s->varTableLen = 0;
  s->stringFieldLen = 0;
  s->stringConstMax = 0;
  s->gcSafePoint = 0;

#ifndef SVS_USE_SEPARATE_STRING_FIELD
  s->stringFieldMax = STRING_FIELD_L + 1;
#endif


#ifdef PC
  s->vmCache = 0;
#endif
  s->tokenMax      = 1;
  s->vmCacheUsed   = 0;
  s->cacheStart    = 0;
  s->tokenCacheMax = 1;

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
    s->varTable[x].type = SVS_TYPE_UNDEF;
  }

  s->varArrayLen = 1;

  for (x = 0; x < SVS_ARRAY_LEN; x++) {
    s->varArray[x].val_u = 0;
    s->varArrayType[x] = SVS_TYPE_UNDEF;
  }

  svsSetName((uint8_t *)"def.b", s);
  svsSetFileName((uint8_t *)"def.svs", s);
  // TODO: remove this in v.1.4
  resetWarnCount();
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
  for(x = 0; x < SVS_FILE_NAME_L; x++) {
    s->vmName[x] = name[x];
  }
}

void svsSetFileName(uint8_t * name, svsVM *s) {
  uint16_t x;
  for(x = 0; x < SVS_FILE_NAME_L; x++) {
    s->fName[x] = name[x];
  }
}

void svsSetStringField(uint8_t * strirngField, uint32_t len, svsVM *s) {
#ifdef SVS_USE_SEPARATE_STRING_FIELD
  s->stringField    = strirngField;
  s->stringFieldMax = len; 
#else
  printf("svsSetStringField: SVS_USE_SEPARATE_STRING_FIELD not enambled in this build!\n");
#endif
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
      if (strCmp(name, s->funcTable[x].name)) {
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
      if (strCmp(name, s->funcTable[x].name)) {
        return x;
      }
    }
  }
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

#ifdef USE_FLOAT
uint8_t svsFloatCompare(float a, float b) {
	float diff = 0.000001; //00
	float tmp;

	tmp = a - b;

	if (tmp < 0) {
		tmp*=-1.0;
	}
	//printf("fltcmp: a:%f b:%f, tmp:%f, diff:%f ", a, b, tmp, diff);
	if (tmp <= diff) {
		//printf("EQ\n");
		return 1;
	} else {
		//printf("NOT EQ\n");
		return 0;
	}
}
#endif
