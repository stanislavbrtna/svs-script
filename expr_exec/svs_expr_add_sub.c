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

void exprExecLvl3(uint16_t index, varRetVal *result, svsVM *s) {
  //NUM + - FLT + -
  varRetVal prac;
  uint16_t tokenId;

  varRetValZero(&prac);

  tokenId = index;
  exprExecDMSG("ExprExecLvl3 Begin", result->value.val_s,tokenId, s);
  exprExecLvl4(tokenId, result, s);
  if (errCheck(s)) {
    return;
  }
  tokenId = result->tokenId;

  if (result->type == SVS_TYPE_STR) {
    exprExecDMSG("ExprExecLvl3 Exit, token type STR, nothing to do",result->value.val_s,result->tokenId, s);
    return;
  }

  while ((getTokenType(tokenId, s) == SVS_TOKEN_ADD) || (getTokenType(tokenId, s) == SVS_TOKEN_SUBT)) {
    if (getTokenType(tokenId,s) == SVS_TOKEN_ADD) {
      exprExecDMSG("ExprExecLvl3 + (NUM) operator", result->value.val_s, tokenId, s);
      exprExecLvl4(result->tokenId + 1, &prac, s); // get second operand
      if (errCheck(s)) {
        return;
      }
      if ((result->type == SVS_TYPE_NUM) && (prac.type == SVS_TYPE_NUM)) {
        result->value.val_s += prac.value.val_s;
        tokenId = prac.tokenId;
        result->tokenId = prac.tokenId;
      } else if ((result->type == SVS_TYPE_FLT) && (prac.type == SVS_TYPE_FLT)) {
#ifdef USE_FLOAT
        result->value.val_f += prac.value.val_f;
        tokenId = prac.tokenId;
        result->tokenId = prac.tokenId;
#endif
      } else if((result->type == SVS_TYPE_STR) || (prac.type == SVS_TYPE_STR)) {
        exprExecDMSG("ExprExecLvl3 + (NUM) operator: Exit", result->value.val_s, tokenId, s);
        result->tokenId = tokenId;
        return;
      } else if((result->type == SVS_TYPE_ARR) || (prac.type == SVS_TYPE_ARR)) {
        errSoft((uint8_t *)"Can not add array identificators!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      } else {
        errSoft((uint8_t *)"Can not add float and num!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      }
    } else  if (getTokenType(tokenId,s) == SVS_TOKEN_SUBT) {
      exprExecDMSG("ExprExecLvl3 - operator", result->value.val_s, tokenId, s);
      exprExecLvl4(result->tokenId + 1, &prac, s);
      if (errCheck(s)) {
        return;
      }
      if ((result->type == SVS_TYPE_NUM) && (prac.type == SVS_TYPE_NUM)) {
        result->value.val_s -= prac.value.val_s;
        tokenId = prac.tokenId;
        result->tokenId = prac.tokenId;
      } else if ((result->type == SVS_TYPE_FLT) && (prac.type == SVS_TYPE_FLT)) {
      #ifdef USE_FLOAT
        result->value.val_f -= prac.value.val_f;
        tokenId = prac.tokenId;
        result->tokenId = prac.tokenId;
      #endif
      }else if ( (result->type == SVS_TYPE_NUM)
                 && (prac.type == SVS_TYPE_FLT) && (result->value.val_s == 0)) { // solves (-1.1)
      #ifdef USE_FLOAT
        result->value.val_f = prac.value.val_f * -1;
        result->type = 3;
        tokenId = prac.tokenId;
        result->tokenId = prac.tokenId;
      #endif
      } else if((result->type == SVS_TYPE_ARR) || (prac.type == SVS_TYPE_ARR)) {
        errSoft((uint8_t *)"Can not subtract array identificators!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      } else {
        errSoft((uint8_t *)"Can only subtract num with num or float with float!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      }
    }
  }

  exprExecDMSG("ExprExecLvl3 Exit", result->value.val_s, result->tokenId, s);
}


void exprExecLvl2(uint16_t index, varRetVal *result, svsVM *s) {
  //STR +
  varRetVal prac;
  uint16_t tokenId;

  varRetValZero(&prac);

  tokenId = index;
  exprExecDMSG("ExprExecLvl2 Begin", result->value.val_s, tokenId, s);
  exprExecLvl3(tokenId, result, s);
  if (errCheck(s)) {
    return;
  }
  tokenId = result->tokenId;

  while (getTokenType(tokenId, s) == SVS_TOKEN_ADD) {
    if (getTokenType(tokenId, s) == SVS_TOKEN_ADD) {
      exprExecDMSG("ExprExecLvl2 + (STR) operator",result->value.val_s,tokenId, s);
      exprExecLvl3(result->tokenId + 1, &prac, s);
      if (errCheck(s)) {
        return;
      }
      if ((result->type == SVS_TYPE_STR) && (prac.type == SVS_TYPE_STR)) {
        result->value.val_str = strAdd(result->value.val_str, prac.value.val_str, s);
        tokenId = prac.tokenId;
        result->tokenId = prac.tokenId;
        exprExecDMSG("ExprExecLvl2 STR: STR + STR", result->value.val_s, tokenId, s);
      } else if ((result->type == SVS_TYPE_NUM) && (prac.type == SVS_TYPE_STR)) {
        result->value.val_str = strAdd(i16toString(result->value, s).val_str, prac.value.val_str, s);
        result->type = SVS_TYPE_STR;
        tokenId = prac.tokenId;
        result->tokenId = prac.tokenId;
        exprExecDMSG("ExprExecLvl2 STR: NUM + STR", result->value.val_s, tokenId, s);
      } else if ((result->type == SVS_TYPE_STR) && (prac.type == SVS_TYPE_NUM)) {
        result->value.val_str = strAdd(result->value.val_str, i16toString(prac.value, s).val_str, s);
        result->type = SVS_TYPE_STR;
        tokenId = prac.tokenId;
        result->tokenId = prac.tokenId;
        exprExecDMSG("ExprExecLvl2 STR: STR + NUM", result->value.val_s, tokenId, s);
      }else if ((result->type == SVS_TYPE_STR) && (prac.type == SVS_TYPE_FLT)) {
        #ifdef USE_FLOAT
        result->value.val_str = strAdd(result->value.val_str, floatToString(prac.value, s).val_str,s);
        result->type = SVS_TYPE_STR;
        tokenId = prac.tokenId;
        result->tokenId = prac.tokenId;
        exprExecDMSG("ExprExecLvl2 STR: STR + NUM", result->value.val_s, tokenId, s);
        #endif
      }else if ((result->type == SVS_TYPE_FLT) && (prac.type == SVS_TYPE_STR)) {
      #ifdef USE_FLOAT
        result->value.val_str = strAdd(floatToString(result->value, s).val_str, prac.value.val_str, s);
        result->type = SVS_TYPE_STR;
        tokenId = prac.tokenId;
        result->tokenId = prac.tokenId;
        exprExecDMSG("ExprExecLvl2 STR: NUM + STR", result->value.val_s, tokenId, s);
       #endif
      } else {
        errSoft((uint8_t *)"Wrong type of argument for an add \"+\" operation.", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      }
    }
  }

  exprExecDMSG("ExprExecLvl2 Exit", result->value.val_s, result->tokenId, s);
}
