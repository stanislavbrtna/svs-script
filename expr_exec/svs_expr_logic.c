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

uint16_t exprExecValueSkip(uint16_t index, svsVM *s) {
  // one token
  if(getTokenType(index, s) == SVS_TOKEN_CONST_NUM
    || getTokenType(index, s) == SVS_TOKEN_CONST_FLOAT
    || getTokenType(index, s) == SVS_TOKEN_CONST_STR
  ){
    return index + 1;
  }

  // var or array
  if(getTokenType(index, s) == SVS_TOKEN_VAR
     || getTokenType(index, s) == SVS_TOKEN_ARG
  ){
    index++;
    if(getTokenType(index, s) == SVS_TOKEN_LSQB) {
      index = exprExecValueSkip(index + 1, s);
      if (getTokenType(index, s) != SVS_TOKEN_RSQB) {
        errSoft((uint8_t *)"exprExecSkip ARRAY: Missing \"]\")!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)index, s);
        errSoftSetToken(index, s);
        return index;
      }
      return index + 1;
    }
    
    return index;
  }

  // NOT
  if(getTokenType(index, s) == SVS_TOKEN_NOT) {
    return exprExecValueSkip(index + 1, s);
  }

  // "calls"
  if(getTokenType(index, s) == SVS_TOKEN_SYS
  || getTokenType(index, s) == SVS_TOKEN_CALL
  || getTokenType(index, s) == SVS_TOKEN_FUNCTION_BUILTIN
  ) {
    return exprSkip(index + 1, s) + 1;
  }

  // ()
  if(getTokenType(index, s) == SVS_TOKEN_LBR) {
    return exprSkip(index, s) + 1;
  }

  return index;
}


uint16_t exprExecSkip(uint16_t index, svsVM *s) {
  // skip value
  index = exprExecValueSkip(index, s);

  while (getTokenType(index, s) != SVS_TOKEN_RBR
    && getTokenType(index, s) != SVS_TOKEN_SCOL
    && getTokenType(index, s) != SVS_TOKEN_COL
  ){
    index = exprExecValueSkip(index + 1, s);
  }
  return index;
}


void exprExecLvlLogic(uint16_t index, varRetVal *result, svsVM *s) {
  // AND OR
  varRetVal prac;
  uint16_t tokenId;

  varRetValZero(&prac);

  tokenId = index;
  exprExecDMSG("ExprExecLvlLogic (0) Begin", result->value.val_s, tokenId, s);
  exprExecLvl1(tokenId, result, s);
  if (errCheck(s)) {
    return;
  }
  tokenId = result->tokenId;

  while ((getTokenType(tokenId, s) == SVS_TOKEN_AND) || (getTokenType(tokenId, s) == SVS_TOKEN_OR)) {
    if (getTokenType(tokenId, s) == SVS_TOKEN_AND) { // and
      exprExecDMSG("ExprExecLvlLogic AND operator", result->value.val_s, tokenId, s);
      // lazy evaluation
      if (result->type == SVS_TYPE_NUM && result->value.val_s == 0) {
        result->value.val_s = 0;
        result->tokenId = exprExecSkip(result->tokenId + 1, s); // skipping the rest of expression
        exprExecDMSG("ExprExecLvlLogic (0) Exit", result->value.val_s, result->tokenId, s);
        return;
      }

      exprExecLvl1(result->tokenId + 1, &prac, s); //získáme druhý operand
      if (errCheck(s)) {
        return;
      }
      if ((result->type == SVS_TYPE_NUM) && (prac.type == SVS_TYPE_NUM)) { //ověříme typ a pokud je to num, tak
        if (result->value.val_s && prac.value.val_s) {
          result->value.val_s = 1;
        } else {
          result->value.val_s = 0;
        }
        tokenId = prac.tokenId;  //nastavíme token id co se vrátilo
        result->tokenId = prac.tokenId; //nastavíme znova
        exprExecDMSG("ExprExecLogic AND: ", result->value.val_s, tokenId, s);
      } else {
        errSoft((uint8_t *)"Can only use logic operators (AND) on num type!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      }
    }else if (getTokenType(tokenId,s) == SVS_TOKEN_OR) { // or
      exprExecDMSG("ExprExecLvlLogic OR operator", result->value.val_s, tokenId, s);
      if (result->type == SVS_TYPE_NUM && result->value.val_s == 1) {
        result->value.val_s = 1;
        result->tokenId = exprExecSkip(result->tokenId + 1, s);
        exprExecDMSG("ExprExecLvlLogic (0) Exit", result->value.val_s, result->tokenId, s);
        return;
      }

      exprExecLvl1(result->tokenId + 1, &prac, s); //získáme druhý operand
      if (errCheck(s)) {
        return;
      }
      if ((result->type == SVS_TYPE_NUM) && (prac.type == SVS_TYPE_NUM)) { //ověříme typ a pokud je to num, tak
        if (result->value.val_s || prac.value.val_s) {
          result->value.val_s = 1;
        } else {
          result->value.val_s = 0;
        }
        tokenId = prac.tokenId;  //nastavíme token id co se vrátilo
        result->tokenId = prac.tokenId; //nastavíme znova
        exprExecDMSG("ExprExecLogic OR: ", result->value.val_s, tokenId, s);
      } else {
        errSoft((uint8_t *)"Can only use logic operators (OR) on num type!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      }
    }
  }

  exprExecDMSG("ExprExecLvlLogic (0) Exit", result->value.val_s, result->tokenId, s);
}