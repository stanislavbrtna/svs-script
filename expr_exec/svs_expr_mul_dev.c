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

void exprExecLvl4(uint16_t index, varRetVal *result, svsVM *s) {
  //NUM * / % FLT * /
  varRetVal prac;
  uint16_t tokenId;

  varRetValZero(&prac);

  tokenId = index;
  exprExecDMSG("ExprExecLvl4 Begin", result->value.val_s, tokenId, s);
  exprExecLvl5(tokenId, result, s);
  if (errCheck(s)){
    return;
  }
  tokenId = result->tokenId;

  if (result->type == SVS_TYPE_STR) {
    exprExecDMSG("ExprExecLvl4 Exit, token type STR, nothing to do",result->value.val_s,result->tokenId, s);
    return;
  }

  while((getTokenType(tokenId, s) == 3) || (getTokenType(tokenId, s) == 4) || (getTokenType(tokenId, s) == 30)) {
    if (getTokenType(tokenId, s) == 3) { // násobení
      exprExecDMSG("ExprExecLvl4 * operator", result->value.val_s, tokenId, s);
      exprExecLvl5(result->tokenId + 1, &prac, s); //získáme druhý operand
      if (errCheck(s)) {
        return;
      }
      if ((result->type == 0) && (prac.type == 0)) { //ověříme typ a pokud je to číslo, tak
        result->value.val_s *= prac.value.val_s; //vynásobbí
        tokenId = prac.tokenId;  //nastavíme token id co se vrátilo
        result->tokenId = prac.tokenId; //nastavíme znova
      } else if ((result->type == 3) && (prac.type == 3)) { //ověříme typ a pokud je to float, tak
        #ifdef USE_FLOAT
        result->value.val_f *= prac.value.val_f; //vynásobbí
        tokenId = prac.tokenId;  //nastavíme token id co se vrátilo
        result->tokenId = prac.tokenId; //nastavíme znova
        #endif
      } else if((result->type == SVS_TYPE_ARR) || (prac.type == SVS_TYPE_ARR)) {
        errSoft((uint8_t *)"Can not multiply array identificators!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      } else {
        errSoft((uint8_t *)"Can only multiply num and num or float and float!", s);
        errSoftSetParam((uint8_t *)"TokenId",(varType)tokenId,s);
        errSoftSetToken(tokenId,s);
        return;
      }
    } else  if (getTokenType(tokenId, s) == 4) { // dělení
      exprExecDMSG("ExprExecLvl4 / operator", result->value.val_s, tokenId, s);
      exprExecLvl5(result->tokenId + 1, &prac, s); //získáme druhý operand
      if (errCheck(s)) {
        return;
      }
      if ((result->type == 0) && (prac.type == 0)) { //ověříme typ a pokud je to číslo, tak
        if (prac.value.val_s != 0) {
          result->value.val_s /= prac.value.val_s; //vydělíme
        } else {
          errSoft((uint8_t *)"Division by zero!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
          errSoftSetToken(tokenId, s);
          return;
        }
        tokenId = prac.tokenId;  //nastavíme token id co se vrátilo
        result->tokenId = prac.tokenId; //nastavíme znova
      } else if ((result->type == 3) && (prac.type == 3)) { //ověříme typ a pokud je to float, tak
        #ifdef USE_FLOAT
        if (prac.value.val_f != 0) {
          result->value.val_f /= prac.value.val_f; //vydělíme
        } else {
          errSoft((uint8_t *)"Division by zero!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
          errSoftSetToken(tokenId, s);
          return;
        }
        tokenId = prac.tokenId; //nastavíme token id co se vrátilo
        result->tokenId = prac.tokenId; //nastavíme znova
        #endif
      } else if((result->type == SVS_TYPE_ARR) || (prac.type == SVS_TYPE_ARR)) {
        errSoft((uint8_t *)"Can not divide array identificators!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      } else {
        errSoft((uint8_t *)"Can only divide num and num or float and float!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      }
    }else if (getTokenType(tokenId, s) == 30) { // %
      exprExecDMSG("ExprExecLvl4 %% operator", result->value.val_s,tokenId, s);
      exprExecLvl5(result->tokenId + 1, &prac, s); //získáme druhý operand
      if (errCheck(s)) {
        return;
      }
      if ((result->type == 0) && (prac.type == 0)) { //ověříme typ a pokud je to číslo, tak
        if (prac.value.val_s != 0) {
          result->value.val_s %= prac.value.val_s; //vydělíme
        } else {
          errSoft((uint8_t *)"Division by zero!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
          errSoftSetToken(tokenId, s);
          return;
        }
        tokenId = prac.tokenId;  //nastavíme token id co se vrátilo
        result->tokenId = prac.tokenId; //nastavíme znova
      } else if((result->type == SVS_TYPE_ARR) || (prac.type == SVS_TYPE_ARR)) {
        errSoft((uint8_t *)"Can not divide array identificators!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      } else {
        errSoft((uint8_t *)"Can not divide string and num!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      }
    }
  }
  exprExecDMSG("ExprExecLvl4 Exit", result->value.val_s, result->tokenId, s);
}
