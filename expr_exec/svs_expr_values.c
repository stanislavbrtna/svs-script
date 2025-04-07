/*
Copyright (c) 2024 Stanislav Brtna

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

#include "svs_expr_exec2.h"

extern uint8_t unsecureCommand;

void exprExecLvl5(uint16_t index, varRetVal *result, svsVM *s) {
  uint16_t gcSafePointPrev;
  // ( ) NUM FLT STR VAR CALL
  varRetVal prac;

  exprExecDMSG("ExprExecLvl5 Begin", result->value.val_s, index, s);

  if (getTokenType(index, s) == SVS_TOKEN_CONST_NUM) {
    result->value = getTokenData(index, s);
    result->type = 0;
    result->tokenId = index + 1;
    exprExecDMSG("ExprExecLvl5 NUM const.", result->value.val_s, result->tokenId, s);
  } else if (getTokenType(index, s) == SVS_TOKEN_CONST_FLOAT) {
    #ifdef USE_FLOAT
    result->value = getTokenData(index, s);
    result->type = 3;
    result->tokenId = index + 1;
    exprExecDMSG("ExprExecLvl5 FLT const.", result->value.val_s, result->tokenId, s);
    #endif
  } else  if (getTokenType(index, s) == SVS_TOKEN_SYS) {
    sysExec(index, &prac, s);
    result->value.val_s = prac.value.val_s;
    result->type = prac.type;
    result->tokenId = prac.tokenId;
    exprExecDMSG("ExprExecLvl5 SYS", result->value.val_s, result->tokenId, s);
  } else  if (getTokenType(index, s) == SVS_TOKEN_FUNCTION_BUILTIN) {
    processBuiltInCall(index, &prac, s);
    if (errCheck(s)) {
      return;
    }
    result->value.val_s = prac.value.val_s;
    result->type = prac.type;
    result->tokenId = prac.tokenId;
    exprExecDMSG("ExprExecLvl5 BUILTIN FUNC", result->value.val_s, result->tokenId, s);
  } else if (getTokenType(index, s) == SVS_TOKEN_CALL) { //CALL
    if ((getTokenType(index + 1, s) == 5)) {
      unsecureCommand = 1;
      gcSafePointPrev = s->gcSafePoint;
      s->gcSafePoint = s->stringFieldLen - 1;
      index = commParseCall(index, s);
      s->gcSafePoint = gcSafePointPrev;
      if (errCheck(s)) {
        return;
      }
    } else {
      errSoft((uint8_t *)"ExprExecLvl5 CALL: Missing \"(\" at start of function! ", s);
      errSoftSetParam((uint8_t *)"TokenId", (varType)index, s);
      errSoftSetToken(index, s);
      return;
    }
    result->value = (varType)s->commRetVal;
    result->type = s->commRetType;
    result->tokenId = index;
    exprExecDMSG("ExprExecLvl5 Function Call", result->value.val_s, result->tokenId, s);
  } else if (getTokenType(index, s) == SVS_TOKEN_CONST_STR) {
    result->value = getTokenData(index, s);
    result->type = SVS_TYPE_STR;
    result->tokenId = index + 1;
    exprExecDMSG("ExprExecLvl5 STR const.", result->value.val_s, result->tokenId, s);
  } else if ((getTokenType(index, s) == SVS_TOKEN_VAR) || (getTokenType(index, s) == SVS_TOKEN_ARG)) {
    if (getTokenType(index, s) == SVS_TOKEN_VAR) {
      // variable
      result->value = varGetVal(getTokenData(index, s), s);
      result->type = varGetType(getTokenData(index, s), s);
      result->tokenId = index + 1;
    } else {
      // argument
      result->value = s->commArgs.arg[(uint16_t)getTokenData(index, s).val_u + 1];
      result->type = s->commArgs.argType[(uint16_t)getTokenData(index, s).val_u + 1];
      result->tokenId = index + 1;

      if (result->type == SVS_TYPE_UNDEF) {
        if (getUndefWarning()) {
          printf("WARNING: argument \"arg%u\" on token %d was used in an expression without initialization.\n",
            getTokenData(index, s).val_u,
            result->tokenId
          );
        }
        result->type = SVS_TYPE_NUM;
      }
      exprExecDMSG("ExprExecLvl5 ARG, now internaly assumed as VAR.", result->value.val_s, result->tokenId, s);
    }

    if (result->type == 0) {
      exprExecDMSG("ExprExecLvl5 VAR type NUM", result->value.val_s, result->tokenId, s);
    } else if (result->type == 1) {
      exprExecDMSG("ExprExecLvl5 VAR type STR", result->value.val_s, result->tokenId, s);
    } else if (result->type == 3) {
      exprExecDMSG("ExprExecLvl5 VAR type FLT", result->value.val_s, result->tokenId, s);
    } else if (result->type == SVS_TYPE_ARR) {
      exprExecDMSG("ExprExecLvl5 VAR type ARRAY", result->value.val_s, result->tokenId, s);
      varType arrayIndex;
      varType arrayBase;
      arrayBase = result->value;
      index++;
      if (getTokenType(index, s) == SVS_TOKEN_LSQB) {
        exprExec(index + 1, result, s); //odskočí a vrátí v resultu doufejme konec závorky
        if (errCheck(s)) {
          return;
        }
        index = result->tokenId;
        arrayIndex = result->value;

        if (arrayIndex.val_s < 0) {
          errSoft((uint8_t *)"ExprExecLvl5 ARRAY: Negative array index!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)index, s);
          errSoftSetToken(index, s);
          return;
        }

        if (arrayIndex.val_s > s->varArray[arrayBase.val_s].val_s) {
          errSoft((uint8_t *)"ExprExecLvl5 ARRAY: Array out of bounds!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)index, s);
          errSoftSetToken(index, s);
          return;
        }

        result->value = s->varArray[1 + arrayIndex.val_s + arrayBase.val_s];
        result->type = s->varArrayType[1 + arrayIndex.val_s + arrayBase.val_s];

        if (result->type == SVS_TYPE_UNDEF) {
          exprExecDMSG("ExprExecLvl5 ARRAY element type UNDEF", result->value.val_s, result->tokenId, s);
          if (getUndefWarning()) {
            printf("WARNING: array element %u on token %d was used in an expression without initialization.\n\
This will produce errors in future releases.\n",
              arrayIndex.val_u,
              result->tokenId
            );
          }
          result->type = SVS_TYPE_NUM;
        }

        if (getTokenType(index, s) != SVS_TOKEN_RSQB) {
          errSoft((uint8_t *)"ExprExecLvl5 ARRAY: Missing \"]\")!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)index, s);
          errSoftSetToken(index, s);
          return;
        }
        result->tokenId = index + 1;
      } else {
        exprExecDMSG("ExprExecLvl5 passing ARRAY identificator", result->value.val_s, result->tokenId, s);
      }
    } else if (result->type == SVS_TYPE_UNDEF) {
      exprExecDMSG("ExprExecLvl5 VAR type UNDEF", result->value.val_s, result->tokenId, s);
      if (getUndefWarning()) {
        printf("WARNING: variable \"%s\" on token %d was used in an expression without initialization.\n\
This will produce errors in future releases.\n",
          s->varTable[getTokenData(index, s).val_u].name,
          result->tokenId
        );
      }
      result->type = SVS_TYPE_NUM;
    } else {
      errSoft((uint8_t *)"ExprExecLvl5 VAR: Unknown variable type!", s);
      errSoftSetParam((uint8_t *)"TokenId", (varType)index, s);
      errSoftSetToken(index, s);
      return;
    }
  } else if ((getTokenType(index, s) == SVS_TOKEN_NOT)) {
    exprExecDMSG("ExprExecLvl5 NOT statement", result->value.val_s, result->tokenId, s);
    exprExecLvl5(index + 1, result, s); // get next statement
    if (errCheck(s)) {
      return;
    }
    if (result->type != SVS_TYPE_NUM) {
      errSoft((uint8_t *)"ExprExecLvl5 NOT: You can only apply NOT on num type!", s);
      errSoftSetParam((uint8_t *)"TokenId", (varType)index, s);
      errSoftSetToken(index, s);
      return;
    }
    if (result->value.val_s) { // perform negation
      result->value = (varType)((int32_t)0);
    } else {
      result->value = (varType)((int32_t)1);
    }
    exprExecDMSG("ExprExecLvl5 NOT statement end", result->value.val_s, result->tokenId, s);
  } else if (((getTokenType(index, s) == SVS_TOKEN_LBR) || (getTokenType(index, s) == SVS_TOKEN_RBR))) {
    while ((getTokenType(index, s) == SVS_TOKEN_LBR) || (getTokenType(index, s) == SVS_TOKEN_RBR)) {
      if (getTokenType(index, s) == SVS_TOKEN_LBR) { // (
        exprExecDMSG("ExprExecLvl5 recursion on (", result->value.val_s, result->tokenId, s);
        exprExec(index + 1, result, s); //odskočí a vrátí v resultu doufejme konec závorky
        if (errCheck(s)) {
          return;
        }
        index = result->tokenId;
      }
      if(getTokenType(index, s) == SVS_TOKEN_RBR) { // ) když nám přišel konec závorky, posuneme tokenId returnu a skočíme pryč
        result->tokenId++;
        exprExecDMSG("ExprExecLvl5 Exit on )", result->value.val_s, index, s);
        return;
      }
    }
  } else {
    result->tokenId = index; //když to neprojde, tak necháme propadnout
  }
  exprExecDMSG("ExprExecLvl5 Exit", result->value.val_s, result->tokenId, s);
}
