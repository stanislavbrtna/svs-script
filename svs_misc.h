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

Misc. functions used in SVS.
*/

#ifndef SVS_MISC_H
#define SVS_MISC_H

#include "svs_basics.h"
uint8_t syscallExists(uint8_t *name, svsVM *s);
uint8_t syscallGetId(uint8_t *name, svsVM *s);

uint8_t functionExists(uint8_t *name, svsVM *s);
uint8_t functionGetId(uint8_t *name, svsVM *s);
uint8_t varExists(uint8_t *name, svsVM *s);
VARTYPE varGetId(uint8_t *name, svsVM *s);
uint8_t varGetType(VARTYPE id, svsVM *s);
uint8_t varSetType(VARTYPE id, uint8_t type, svsVM *s);

VARTYPE varGetVal(VARTYPE id, svsVM *s);
uint8_t varSetVal(VARTYPE id, VARTYPE val, svsVM *s);

varType newArray(varType Id, uint16_t len, svsVM *s);


uint8_t isNumber(uint8_t x);
uint8_t isRegChar(uint8_t x);


void varRetValZero(varRetVal *x);
void svsReset(svsVM *s);
void svsSetName( uint8_t * name, svsVM *s);
void svsSetFileName( uint8_t * name, svsVM *s);

void svsInfo(svsVM *s);

//get svsVM return value
int32_t getRetValInt(svsVM *s);
uint8_t * getRetValStr(svsVM *s);
float getRetValFlt(svsVM *s);
float exp_helper(uint16_t a, uint16_t ex);

#ifdef USE_FLOAT
uint8_t svsFloatCompare(float a, float b);
#endif

#ifdef LOCAL_VARIABLES_ENABLED
void varAddLocal(varType Id, svsVM *s);
#endif

// undef warning
uint8_t getUndefWarning();
void resetWarnCount();
#endif
