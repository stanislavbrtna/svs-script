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

This is the second version of expression exec logic.
*/

#include "svs_expr_exec2.h"

uint8_t unsecureCommand;
uint8_t exprExecDebug;


void setExprExecDebug(uint8_t level) {
  exprExecDebug = level;
}


void exprExecDMSG(char *text, int16_t result, uint16_t tokenId, svsVM *s) {
  if ((exprExecDebug == 1) || (s->globalDebug)) {
    printf("exprExecDMSG: %s \nResult:%i\ntokenId: %u\n", text, result, tokenId);
  }
}


void exprExec(uint16_t index, varRetVal *result, svsVM *s) {
  uint16_t tokenId;
  uint16_t strBeginVal;
  uint16_t x;
  uint16_t gcSafePointPrev;
  uint8_t unsecureCommandFlag;
  varRetVal prac;

  varRetValZero(&prac);
  varRetValZero(result);
  result->tokenId = index;

  tokenId = index;

  //printf("GC:EXPR: begin %u\n", s->stringFieldLen);
  strBeginVal = s->stringFieldLen;
  gcSafePointPrev = s->gcSafePoint;
  s->gcSafePoint = s->stringFieldLen - 1;
  unsecureCommandFlag = unsecureCommand;

  exprExecDMSG("ExprExec Begin", result->value.val_s, tokenId, s);

  exprExecLvlLogic(tokenId, result, s);
  if (errCheck(s)) {
    return;
  }

  // Unsecure command is any command that might introduce more new strings in the memory
  // than just the result of the expression.
  // If there is no call, then small garbage collection is done after which the only new string
  // is the expression result, this is much quicker than full garbage collection.
  // In industry terms this would be sort of gen 0 garbage collection.
  // But by design this would also remove strings introduced by functions called inside expression.
  // So this garbage collection is only triggered if there is no function call in the expression stack.

  // For example x = "abc" + func(); function func{ b = "text"; return "def";}
  // Without this unsecure command mechanism the string in variable b would be lost.

  if (unsecureCommand == 0) {
    if ((strBeginVal < s->stringFieldLen) && (result->type == 1)) {
      for(x = 0; x < (s->stringFieldLen - strBeginVal); x++) {
        if(s->stringField[result->value.val_str + x] != 0) {
          s->stringField[strBeginVal + x] = s->stringField[result->value.val_str + x];
        } else {
          break;
        }
      }
      s->stringFieldLen = strBeginVal + x + 1;
      s->stringField[strBeginVal + x] = 0;
      result->value.val_s = strBeginVal;
    }
  }

  // gc safepoint is introduced because of function call,
	// because garbage collection inside this call might remove
	// unwanted strings from unfinished expression, unsecure command is flagged and garbage collector
	// safe point is set.

	// for example: x = "alpha" + 1 + func();
	// new string "alpha1" would be created before calling func(), garbage collector call inside func
	// would remove the "alpha1", because there is no referenece to it yet.

  s->gcSafePoint = gcSafePointPrev;

  if (unsecureCommandFlag == 0 && unsecureCommand == 1){
    unsecureCommand = 0;
  }

  exprExecDMSG("ExprExec Exit",result->value.val_u,result->tokenId, s);

  return;
}
