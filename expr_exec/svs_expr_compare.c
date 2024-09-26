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

void exprExecLvl1(uint16_t index, varRetVal *result, svsVM *s) {
  //== != <= >= < > --hotovo
  uint16_t tokenId;
  varRetVal prac;
  varRetValZero(&prac);

  result->tokenId = index;
  tokenId = index;

  exprExecDMSG("ExprExecLvl1 Begin", result->value.val_s, tokenId, s);

  exprExecLvl2(tokenId, result, s);
  if (errCheck(s)) {
    return;
  }
  tokenId = result->tokenId;

  while ((getTokenType(tokenId, s) >= SVS_TOKEN_EQUALS) && (getTokenType(tokenId, s) <= 23)) {
    if (getTokenType(tokenId,s) == SVS_TOKEN_EQUALS) { //==
      tokenId++;
      exprExecDMSG("ExprExecLvl1: == ", prac.value.val_s, tokenId, s);
      exprExecLvl2(tokenId, &prac, s);
      if (errCheck(s)) {
        return;
      }
      if (result->type == prac.type) { //typová kontrola/type check
        if (result->type == SVS_TYPE_NUM) {
          if (result->value.val_s == prac.value.val_s) { //porovnání dvou čísel / comparing num types
            result->value.val_s = 1;
            result->type = 0;
            exprExecDMSG("ExprExecLvl1: == (NUM) TRUE", prac.value.val_s,tokenId, s);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: == (NUM) FALSE", prac.value.val_s,tokenId, s);
            result->tokenId = prac.tokenId;
          }
        }else if (result->type == SVS_TYPE_STR) {
          if (strCmp(s->stringField + result->value.val_str, s->stringField + prac.value.val_str) == 1) {
            result->value.val_s = 1;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: == (STR) TRUE", prac.value.val_s, tokenId, s);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: == (STR) FALSE", prac.value.val_s, tokenId, s);
            result->tokenId = prac.tokenId;
          }
        }else if (result->type == SVS_TYPE_FLT) {
        #ifdef USE_FLOAT
          if (svsFloatCompare(result->value.val_f, prac.value.val_f)) { //porovnání dvou floatů / comparing float types
            result->value.val_s = 1;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: == (FLT) TRUE", prac.value.val_s, tokenId, s);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: == (FLT) FALSE", prac.value.val_s, tokenId, s);
            result->tokenId = prac.tokenId;
          }
        #endif
        }
      } else {
        errSoft((uint8_t *)"Can only compare (==) same types!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      }
    }else if (getTokenType(tokenId, s) == SVS_TOKEN_NOTEQUALS) { //!=
      tokenId++;
      exprExecDMSG("ExprExecLvl1: != ", prac.value.val_s, tokenId, s);
      exprExecLvl2(tokenId, &prac, s);
      if (errCheck(s)) {
        return;
      }
      if (result->type == prac.type) { //typová kontrola / type check
        if (result->type == SVS_TYPE_NUM) {
          if (result->value.val_s != prac.value.val_s) { //porovnání dvou čísel
            result->value.val_s = 1;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: != (NUM) TRUE", prac.value.val_str, tokenId, s);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: != (NUM) FALSE", prac.value.val_str, tokenId, s);
            result->tokenId = prac.tokenId;
          }
        }else if (result->type == SVS_TYPE_FLT) {
        #ifdef USE_FLOAT
          if (!svsFloatCompare(result->value.val_f, prac.value.val_f)) { //porovnání dvou floatů
            result->value.val_s = 1;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: != (FLT) TRUE", prac.value.val_str, tokenId, s);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: != (FLT) FALSE", prac.value.val_str, tokenId, s);
            result->tokenId = prac.tokenId;
          }
        #endif
        } else if (result->type == SVS_TYPE_STR) {
          if (strCmp(s->stringField + result->value.val_str, s->stringField + prac.value.val_str) != 1) {
            result->value.val_s = 1;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: != (STR) TRUE", prac.value.val_s, tokenId, s);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: != (STR) FALSE", prac.value.val_s, tokenId, s);
            result->tokenId = prac.tokenId;
          }
        }
      } else {
        errSoft((uint8_t *)"Can only compare (!=) same types", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      }
    }else if (getTokenType(tokenId, s) == SVS_TOKEN_LESS_THAN) { //<
      tokenId++;
      exprExecDMSG("ExprExecLvl1: < ", prac.value.val_s, tokenId, s);
      exprExecLvl2(tokenId, &prac, s);
      if (errCheck(s)) {
        return;
      }
      if (result->type == prac.type) {
        if (result->type == SVS_TYPE_NUM) {
          if (result->value.val_s < prac.value.val_s) { //<
            result->value.val_s = 1;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: < (NUM) TRUE", prac.value.val_s, tokenId, s);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: < (NUM) FALSE", prac.value.val_s, tokenId, s);
            result->tokenId = prac.tokenId;
          }
        }else if (result->type == SVS_TYPE_FLT) {
        #ifdef USE_FLOAT
          if (result->value.val_f < prac.value.val_f) { //<
            result->value.val_s = 1;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: < (FLT) TRUE", prac.value.val_s, tokenId, s);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: < (FLT) FALSE", prac.value.val_s, tokenId, s);
            result->tokenId = prac.tokenId;
          }
        #endif
        } else if((result->type == SVS_TYPE_ARR) || (prac.type == SVS_TYPE_ARR)) {
          errSoft((uint8_t *)"Can not use < on array identificators!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
          errSoftSetToken(tokenId, s);
          return;
        } else {
          errSoft((uint8_t *)"Can not use < operator on strings!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
          errSoftSetToken(tokenId,s);
          return;
        }
      } else if((result->type == SVS_TYPE_ARR) || (prac.type == SVS_TYPE_ARR)) {
          errSoft((uint8_t *)"Can not use < on array identificators!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
          errSoftSetToken(tokenId, s);
          return;
        } else {
        errSoft((uint8_t *)"Can not compare (<) string and num!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      }
    }else if (getTokenType(tokenId, s) == SVS_TOKEN_GREATER_THAN) { //>
      tokenId++;
      exprExecDMSG("ExprExecLvl1: > ", prac.value.val_s, tokenId, s);
      exprExecLvl2(tokenId, &prac, s);
      if (errCheck(s)) {
        return;
      }
      if (result->type == prac.type) {
        if (result->type == SVS_TYPE_NUM) {
          if (result->value.val_s > prac.value.val_s) { //<
            result->value.val_s = 1;
            result->type = 0;
            exprExecDMSG("ExprExecLvl1: > (NUM) TRUE", prac.value.val_s, tokenId, s);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = 0;
            exprExecDMSG("ExprExecLvl1: > (NUM) FALSE", prac.value.val_s, tokenId, s);
            result->tokenId = prac.tokenId;
          }
        }else if (result->type == 3) {
        #ifdef USE_FLOAT
          if (result->value.val_f > prac.value.val_f) { //<
            result->value.val_s = 1;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: > (FLT) TRUE", prac.value.val_s, tokenId, s);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: > (FLT) FALSE", prac.value.val_s, tokenId, s);
            result->tokenId = prac.tokenId;
          }
        #endif
        } else if((result->type == SVS_TYPE_ARR) || (prac.type == SVS_TYPE_ARR)) {
          errSoft((uint8_t *)"Can not use > on array identificators!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
          errSoftSetToken(tokenId, s);
          return;
        } else {
          errSoft((uint8_t *)"Can not use > operator on strings!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
          errSoftSetToken(tokenId, s);
          return;
        }
      } else if((result->type == SVS_TYPE_ARR) || (prac.type == SVS_TYPE_ARR)) {
          errSoft((uint8_t *)"Can not use > on array identificators!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
          errSoftSetToken(tokenId, s);
          return;
        } else {
        errSoft((uint8_t *)"Can only compare (>) same types", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      }

    } else if (getTokenType(tokenId, s) == SVS_TOKEN_LESS_OR_EQ) { //<=
      tokenId++;
      exprExecDMSG("ExprExecLvl1: <= ", prac.value.val_s, tokenId, s);
      exprExecLvl2(tokenId, &prac, s);
      if (errCheck(s)) {
        return;
      }
      if (result->type == prac.type) { //typová kontrola
        if (result->type == SVS_TYPE_NUM) {
          if (result->value.val_s <= prac.value.val_s) { //<=
            result->value.val_s = 1;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: <= (NUM) TRUE", prac.value.val_s, tokenId, s);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: <= (NUM) FALSE", prac.value.val_s, tokenId, s);
            result->tokenId = prac.tokenId;
          }
        } else if (result->type == SVS_TYPE_FLT) {
        #ifdef USE_FLOAT
          if (result->value.val_f <= prac.value.val_f) { //<=
            result->value.val_s = 1;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: <= (FLT) TRUE", prac.value.val_s, tokenId, s);
            result->tokenId = prac.tokenId;
          }else{
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: <= (FLT) FALSE", prac.value.val_s, tokenId, s);
            result->tokenId = prac.tokenId;
          }
        #endif
        } else if((result->type == SVS_TYPE_ARR) || (prac.type == SVS_TYPE_ARR)) {
          errSoft((uint8_t *)"Can not use <= on array identificators!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
          errSoftSetToken(tokenId, s);
          return;
        } else {
          errSoft((uint8_t *)"Can not use <= operator on strings!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
          errSoftSetToken(tokenId, s);
          return;
        }
      } else if((result->type == SVS_TYPE_ARR) || (prac.type == SVS_TYPE_ARR)) {
          errSoft((uint8_t *)"Can not use <= on array identificators!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
          errSoftSetToken(tokenId, s);
          return;
        } else {
        errSoft((uint8_t *)"Can only compare (<=) same types", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      }
    } else if (getTokenType(tokenId, s) == SVS_TOKEN_GREATER_OR_EQ) { //>=
      tokenId++;
      exprExecDMSG("ExprExecLvl1: >= ", prac.value.val_s, tokenId, s);
      exprExecLvl2(tokenId, &prac, s);
      if (errCheck(s)) {
        return;
      }
      if (result->type == prac.type) { //typová kontrola
        if (result->type == SVS_TYPE_NUM) {
          if (result->value.val_s >= prac.value.val_s) { //>=
            result->value.val_s = 1;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: >= (NUM) TRUE", prac.value.val_s, tokenId, s);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: >= (NUM) FALSE", prac.value.val_s, tokenId, s);
            result->tokenId = prac.tokenId;
          }
        }else if (result->type == SVS_TYPE_FLT) {
        #ifdef USE_FLOAT
          if (result->value.val_f >= prac.value.val_f) { //>=
            result->value.val_s = 1;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: >= (FLT) TRUE", prac.value.val_s, tokenId, s);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: >= (FLT) FALSE", prac.value.val_s, tokenId, s);
            result->tokenId = prac.tokenId;
          }
        #endif
        } else if((result->type == SVS_TYPE_ARR) || (prac.type == SVS_TYPE_ARR)) {
          errSoft((uint8_t *)"Can not use >= on array identificators!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
          errSoftSetToken(tokenId, s);
          return;
        } else {
          errSoft((uint8_t *)"Can not use >= operator on strings!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
          errSoftSetToken(tokenId, s);
          return;
        }
      } else if((result->type == SVS_TYPE_ARR) || (prac.type == SVS_TYPE_ARR)) {
          errSoft((uint8_t *)"Can not use >= on array identificators!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
          errSoftSetToken(tokenId, s);
          return;
        } else {
        errSoft((uint8_t *)"Can only compare (>=) same types", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      }
    }
  }

  exprExecDMSG("ExprExecLvl1 Exit", result->value.val_u, result->tokenId, s);
  return;
}
