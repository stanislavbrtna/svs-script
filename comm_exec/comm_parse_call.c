/*
Copyright (c) 2019 Stanislav Brtna

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

This is the main command execution logic.
*/

#include "svs_comm_exec.h"


void commArgNull(comExArgs *target) {
  uint8_t x;

  for(x = 0; x < FUNCTION_ARGS_MAX + 1; x++) {
    target->arg[x] = (varType)((uint16_t)0);
    target->argType[x] = SVS_TYPE_UNDEF;
  }

  target->usedup = 0;
}


void commArgCopy(comExArgs* source, comExArgs* target){
  uint8_t x;

  for(x = 0; x < FUNCTION_ARGS_MAX + 1; x++) {
    target->arg[x] = source->arg[x];
    target->argType[x] = source->argType[x];
  }

  target->usedup = source->usedup;
}


uint16_t commParseCall(uint16_t index, svsVM *s) {
  uint8_t *callName;
  uint16_t x = 1;
  varRetVal pracVar;
  comExArgs pracArgs; // to store current arguments while new function is executed
  comExArgs pracArgs2; // to temporarily store new arguments
  uint16_t usedUp = 0;
  uint16_t usedUpOld = 0;

  callName = s->stringField + getTokenData(index, s).val_u;

  index++;

  commArgNull(&pracArgs2);

   //tady načteme nové
   if ((getTokenType(index, s) == 5)) {
    index++;
    if (getTokenType(index, s) != 6) { //pokud nemáme prázdnou závorku
      exprExec(index, &pracVar, s);
      if (errCheck(s)) {
        return 0;
      }
      pracArgs2.arg[x] = pracVar.value;
      pracArgs2.argType[x] = pracVar.type;
      index = pracVar.tokenId;
      x++;
    }

    while((getTokenType(index, s) == 33)) { //argumenty odděleny čárkou
      if (x == FUNCTION_ARGS_MAX + 1) {
        errSoft((uint8_t *)"commParseCall: too many arguments in function call!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)index, s);
        errSoftSetToken(index, s);
        return 0;
      }
      index++;
      exprExec(index, &pracVar, s);
      pracArgs2.arg[x] = pracVar.value;
      pracArgs2.argType[x] = pracVar.type;
      index = pracVar.tokenId;
      x++;
    }

    if (getTokenType(index, s) != 6) {
      errSoft((uint8_t *)"commParseCall: Syntax error at end of function call. (missing \")\")", s);
      errSoftSetParam((uint8_t *)"TokenId", (varType)index, s);
      errSoftSetToken(index, s);
      return 0;
    }

    usedUp = x - 1;
    usedUpOld = s->commArgs.usedup;

    //tady bude třeba uložit staré argumenty
    commArgCopy(&s->commArgs, &pracArgs);
    commArgNull(&s->commArgs);

    commArgCopy(&pracArgs2, &s->commArgs);

    s->commArgs.usedup = usedUp;
    index++;
    varRetValZero(&pracVar);

    //commExec

    commExec(callName, s);
    if (errCheck(s)) {
      return 0;
    }
    // ret val:       s->commRetVal;
    // ret val type:  s->commRetType;

    //shození return flagu
    s->commRetFlag = 0;

    //navrácení argumentů zpět
    commArgCopy(&pracArgs, &s->commArgs);
    s->commArgs.usedup = usedUpOld;

    return index;

  } else {
    errSoft((uint8_t *)"commParseCall: Syntax error at the begin of function call. (missing \"(\")", s);
    errSoftSetParam((uint8_t *)"TokenId", (varType)index, s);
    errSoftSetToken(index, s);
    return 0;
  }
}