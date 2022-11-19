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

#ifndef _SVS_VARIABLES_H
#define _SVS_VARIABLES_H
#include "../svs_basics.h"

uint8_t varExists(uint8_t *name, svsVM *s);
VARTYPE varGetId(uint8_t *name, svsVM *s);
uint8_t varGetType(VARTYPE id, svsVM *s);
uint8_t varSetType(VARTYPE id, uint8_t type, svsVM *s);

VARTYPE varGetVal(VARTYPE id, svsVM *s);
uint8_t varSetVal(VARTYPE id, VARTYPE val, svsVM *s);

// arrays
varType newArray(varType Id, uint16_t len, svsVM *s);

// undef warning
uint8_t getUndefWarning();
void resetWarnCount();

#ifdef LOCAL_VARIABLES_ENABLED
void varAddLocal(varType Id, svsVM *s);
#endif

#endif