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

#ifndef SVS_EXPR_EXEC2_H
#define SVS_EXPR_EXEC2_H

#include "../svs_basics.h"

void varRetValZero(varRetVal *x);

void exprExecDMSG(char *text,int16_t result , uint16_t tokenId, svsVM *s);

void exprExec(uint16_t index, varRetVal *result, svsVM *s);
void exprExecLvl5(uint16_t index, varRetVal *result, svsVM *s);
void exprExecLvl4(uint16_t index, varRetVal *result, svsVM *s);
void exprExecLvl3(uint16_t index, varRetVal *result, svsVM *s);
void exprExecLvl2(uint16_t index, varRetVal *result, svsVM *s);
void exprExecLvl1(uint16_t index, varRetVal *result, svsVM *s);
void exprExecLvlLogic(uint16_t index, varRetVal *result, svsVM *s);
void setExprExecDebug(uint8_t level);

#endif
