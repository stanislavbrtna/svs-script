/*
Copyright (c) 2018 Stanislav Brtna

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
#ifndef SVS_MISC_STR_H
#define SVS_MISC_STR_H

#include "svs_basics.h"

uint16_t strNew(uint8_t *index, svsVM *s);
uint8_t *strNewP(uint8_t *index, svsVM *s);
uint16_t strAdd(uint16_t index1, uint16_t index2, svsVM *s);
uint16_t strInsert(uint16_t index1, uint16_t index2, uint16_t pos, svsVM *s);

void strNewStreamInit(svsVM *s);
uint8_t strNewStreamPush(uint8_t c,svsVM *s);
uint16_t strNewStreamEnd(svsVM *s);

uint8_t strCmp(uint8_t *a, uint8_t *b);

uint16_t strLenId(uint16_t id, svsVM *s);

VARTYPE i16toString(VARTYPE num, svsVM *s);
varType floatToString(varType num, svsVM *s);
#endif
