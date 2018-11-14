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

void exprExecDMSG(char *text, int16_t result, uint16_t tokenId) {
  if (exprExecDebug == 1) {
    printf("exprExecDMSG: %s \nResult:%i\ntokenId: %u\n", text, result, tokenId);
  }
}

void exprExecLvl5(uint16_t index, varRetVal *result, svsVM *s) {
  uint16_t gcSafePointPrev;
  // ( ) NUM FLT STR VAR CALL
  varRetVal prac;

  exprExecDMSG("ExprExecLvl5 Begin", result->value.val_s, index);

  if (getTokenType(index, s) == 0) { //NUM
    result->value = getTokenData(index, s);
    result->type = 0;
    result->tokenId = index + 1;
    exprExecDMSG("ExprExecLvl5 NUM const.", result->value.val_s, result->tokenId);
  }else  if (getTokenType(index, s) == 35) { //ARG
    result->value = s->commArgs.arg[(uint16_t)getTokenData(index, s).val_u + 1];
    result->type = s->commArgs.argType[(uint16_t)getTokenData(index, s).val_u + 1];
    result->tokenId = index + 1;
    exprExecDMSG("ExprExecLvl5 ARG (const).", result->value.val_s, result->tokenId);
  }else  if (getTokenType(index, s) == 31) { //FLT
    #ifdef USE_FLOAT
    result->value = getTokenData(index, s);
    result->type = 3;
    result->tokenId = index + 1;
    exprExecDMSG("ExprExecLvl5 FLT const.", result->value.val_s, result->tokenId);
    #endif
  }else  if (getTokenType(index, s) == 28) { //SYS
    sysExec(index, &prac, s);
    result->value.val_s = prac.value.val_s;
    result->type = prac.type;
    result->tokenId = prac.tokenId;
    exprExecDMSG("ExprExecLvl5 SYS", result->value.val_s, result->tokenId);
  }else  if (getTokenType(index, s) == 36) { // BuiltIn FUNC
    processBuiltInCall(index, &prac, s);
    if (errCheck(s)) {
      return;
    }
    result->value.val_s = prac.value.val_s;
    result->type = prac.type;
    result->tokenId = prac.tokenId;
    exprExecDMSG("ExprExecLvl5 BUILTIN FUNC", result->value.val_s, result->tokenId);
  }else if (getTokenType(index, s) == 17) { //CALL
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
    exprExecDMSG("ExprExecLvl5 Function Call", result->value.val_s, result->tokenId);
  }else if (getTokenType(index, s) == 25) { //STR
    result->value = getTokenData(index, s);
    result->type = 1;
    result->tokenId = index + 1;
    exprExecDMSG("ExprExecLvl5 STR const.", result->value.val_s, result->tokenId);
  }else if ((getTokenType(index, s) == 10)) {//VAR   &&(varGetType(tokenData[index])==0)
    result->value = varGetVal(getTokenData(index, s), s);
    result->type = varGetType(getTokenData(index, s), s);
    result->tokenId = index + 1;
    if (result->type == 0) {
      exprExecDMSG("ExprExecLvl5 VAR type NUM", result->value.val_s, result->tokenId);
    }else if(result->type == 1) {
      exprExecDMSG("ExprExecLvl5 VAR type STR", result->value.val_s, result->tokenId);
    }else if(result->type == 3) {
      exprExecDMSG("ExprExecLvl5 VAR type FLT", result->value.val_s, result->tokenId);
    }else if(result->type == SVS_TYPE_ARR) {
      exprExecDMSG("ExprExecLvl5 VAR type ARRAY", result->value.val_s, result->tokenId);
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
      } else {
        errSoft((uint8_t *)"ExprExecLvl5 ARRAY: You can not evaluate whole array (missing \"[\")!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)index, s);
        errSoftSetToken(index, s);
        return;
      }
      arrayIndex = result->value;
      result->value = s->varArray[arrayIndex.val_s + arrayBase.val_s];
      result->type = s->varArrayType[arrayIndex.val_s + arrayBase.val_s];

      if (getTokenType(index, s) != SVS_TOKEN_RSQB) {
        errSoft((uint8_t *)"ExprExecLvl5 ARRAY: Missing \"]\")!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)index, s);
        errSoftSetToken(index, s);
        return;
      }
      result->tokenId = index + 1;
    }else{
      errSoft((uint8_t *)"ExprExecLvl5 VAR: Unknown variable type!", s);
      errSoftSetParam((uint8_t *)"TokenId", (varType)index, s);
      errSoftSetToken(index, s);
      return;
    }
  }else if ((getTokenType(index, s) == 39)) {
    exprExecDMSG("ExprExecLvl5 NOT statement", result->value.val_s, result->tokenId);
    exprExecLvl5(index + 1, result, s); // get next statement
    if (errCheck(s)) {
      return;
    }
    if (result->type != 0) {
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
    exprExecDMSG("ExprExecLvl5 NOT statement end", result->value.val_s, result->tokenId);
    index = result->tokenId;
  }else if (((getTokenType(index, s) == 5) || (getTokenType(index, s) == 6))) {
    while ((getTokenType(index, s) == 5) || (getTokenType(index, s) == 6)) {
      if (getTokenType(index, s) == 5) { // (
        exprExecDMSG("ExprExecLvl5 recursion on (", result->value.val_s, result->tokenId);
        exprExec(index + 1, result, s); //odskočí a vrátí v resultu doufejme konec závorky
        if (errCheck(s)) {
          return;
        }
        index = result->tokenId;
      }
      if(getTokenType(index, s) == 6) { // ) když nám přišel konec závorky, posuneme tokenId returnu a skočíme pryč
        result->tokenId++;
        exprExecDMSG("ExprExecLvl5 Exit on )", result->value.val_s,index);
        return;
      }
    }
  } else {
    result->tokenId = index; //když to neprojde, tak necháme propadnout
  }
  exprExecDMSG("ExprExecLvl5 Exit", result->value.val_s, result->tokenId);
}

void exprExecLvl4(uint16_t index, varRetVal *result, svsVM *s) {
  //NUM * / % FLT * /
  varRetVal prac;
  uint16_t tokenId;

  varRetValZero(&prac);

  tokenId = index;
  exprExecDMSG("ExprExecLvl4 Begin", result->value.val_s, tokenId);
  exprExecLvl5(tokenId, result, s);
  if (errCheck(s)){
    return;
  }
  tokenId = result->tokenId;

  if (result->type == SVS_TYPE_STR) {
    exprExecDMSG("ExprExecLvl4 Exit, token type STR, nothing to do",result->value.val_s,result->tokenId);
    return;
  }

  while((getTokenType(tokenId, s) == 3) || (getTokenType(tokenId, s) == 4) || (getTokenType(tokenId, s) == 30)) {
    if (getTokenType(tokenId, s) == 3) { // násobení
      exprExecDMSG("ExprExecLvl4 * operator", result->value.val_s, tokenId);
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
      } else {
        errSoft((uint8_t *)"Can only multiply num and num or float and float!",s);
        errSoftSetParam((uint8_t *)"TokenId",(varType)tokenId,s);
        errSoftSetToken(tokenId,s);
        return;
      }
    } else  if (getTokenType(tokenId, s) == 4) { // dělení
      exprExecDMSG("ExprExecLvl4 / operator", result->value.val_s, tokenId);
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
      } else {
        errSoft((uint8_t *)"Can not divide string and num or float and num!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      }
    }else if (getTokenType(tokenId, s) == 30) { // %
      exprExecDMSG("ExprExecLvl4 %% operator", result->value.val_s,tokenId);
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
      } else {
        errSoft((uint8_t *)"Can not divide string and num!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      }
    }
  }
  exprExecDMSG("ExprExecLvl4 Exit", result->value.val_s, result->tokenId);

}


void exprExecLvl3(uint16_t index, varRetVal *result, svsVM *s) {
  //NUM + - FLT + -
  varRetVal prac;
  uint16_t tokenId;

  varRetValZero(&prac);

  tokenId = index;
  exprExecDMSG("ExprExecLvl3 Begin", result->value.val_s,tokenId);
  exprExecLvl4(tokenId, result, s);
  if (errCheck(s)) {
    return;
  }
  tokenId = result->tokenId;

  if (result->type == SVS_TYPE_STR) {
    exprExecDMSG("ExprExecLvl3 Exit, token type STR, nothing to do",result->value.val_s,result->tokenId);
    return;
  }

  while ((getTokenType(tokenId, s) == SVS_TOKEN_ADD) || (getTokenType(tokenId, s) == SVS_TOKEN_SUBT)) {
    if (getTokenType(tokenId,s) == SVS_TOKEN_ADD) {
      exprExecDMSG("ExprExecLvl3 + (NUM) operator", result->value.val_s, tokenId);
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
        exprExecDMSG("ExprExecLvl3 + (NUM) operator: Exit", result->value.val_s, tokenId);
        result->tokenId = tokenId;
        return;
      } else {
        errSoft((uint8_t *)"Can not add float and num!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      }
    } else  if (getTokenType(tokenId,s) == SVS_TOKEN_SUBT) {
      exprExecDMSG("ExprExecLvl3 - operator", result->value.val_s, tokenId);
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
      } else {
        errSoft((uint8_t *)"Can only subtract num with num or float with float!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      }
    }
  }

  exprExecDMSG("ExprExecLvl3 Exit", result->value.val_s, result->tokenId);
}


void exprExecLvl2(uint16_t index, varRetVal *result, svsVM *s) {
  //STR +
  varRetVal prac;
  uint16_t tokenId;

  varRetValZero(&prac);

  tokenId = index;
  exprExecDMSG("ExprExecLvl2 Begin", result->value.val_s, tokenId);
  exprExecLvl3(tokenId, result, s);
  if (errCheck(s)) {
    return;
  }
  tokenId = result->tokenId;

  while (getTokenType(tokenId, s) == SVS_TOKEN_ADD) {
    if (getTokenType(tokenId, s) == SVS_TOKEN_ADD) {
      exprExecDMSG("ExprExecLvl2 + (STR) operator",result->value.val_s,tokenId);
      exprExecLvl3(result->tokenId + 1, &prac, s);
      if (errCheck(s)) {
        return;
      }
      if ((result->type == SVS_TYPE_STR) && (prac.type == SVS_TYPE_STR)) {
        result->value.val_str = strAdd(result->value.val_str, prac.value.val_str, s);
        tokenId = prac.tokenId;
        result->tokenId = prac.tokenId;
        exprExecDMSG("ExprExecLvl2 STR: STR + STR", result->value.val_s, tokenId);
      } else if ((result->type == SVS_TYPE_NUM) && (prac.type == SVS_TYPE_STR)) {
        result->value.val_str = strAdd(i16toString(result->value, s).val_str, prac.value.val_str, s);
        result->type = SVS_TYPE_STR;
        tokenId = prac.tokenId;
        result->tokenId = prac.tokenId;
        exprExecDMSG("ExprExecLvl2 STR: NUM + STR", result->value.val_s, tokenId);
      } else if ((result->type == SVS_TYPE_STR) && (prac.type == SVS_TYPE_NUM)) {
        result->value.val_str = strAdd(result->value.val_str, i16toString(prac.value, s).val_str, s);
        result->type = SVS_TYPE_STR;
        tokenId = prac.tokenId;
        result->tokenId = prac.tokenId;
        exprExecDMSG("ExprExecLvl2 STR: STR + NUM", result->value.val_s, tokenId);
      }else if ((result->type == SVS_TYPE_STR) && (prac.type == SVS_TYPE_FLT)) {
        #ifdef USE_FLOAT
        result->value.val_str = strAdd(result->value.val_str, floatToString(prac.value, s).val_str,s);
        result->type = SVS_TYPE_STR;
        tokenId = prac.tokenId;
        result->tokenId = prac.tokenId;
        exprExecDMSG("ExprExecLvl2 STR: STR + NUM", result->value.val_s, tokenId);
        #endif
      }else if ((result->type == SVS_TYPE_FLT) && (prac.type == SVS_TYPE_STR)) {
      #ifdef USE_FLOAT
        result->value.val_str = strAdd(floatToString(result->value, s).val_str, prac.value.val_str, s);
        result->type = SVS_TYPE_STR;
        tokenId = prac.tokenId;
        result->tokenId = prac.tokenId;
        exprExecDMSG("ExprExecLvl2 STR: NUM + STR", result->value.val_s, tokenId);
       #endif
      }
    }
  }

  exprExecDMSG("ExprExecLvl2 Exit", result->value.val_s, result->tokenId);
}

//expressionExec
void exprExecLvl1(uint16_t index, varRetVal *result, svsVM *s) {
  //== != <= >= < > --hotovo
  uint16_t tokenId;
  varRetVal prac;
  varRetValZero(&prac);

  result->tokenId = index;
  tokenId = index;

  exprExecDMSG("ExprExecLvl1 Begin", result->value.val_s, tokenId);

  exprExecLvl2(tokenId, result, s);
  if (errCheck(s)) {
    return;
  }
  tokenId = result->tokenId;

  while ((getTokenType(tokenId, s) >= SVS_TOKEN_EQUALS) && (getTokenType(tokenId, s) <= 23)) {
    if (getTokenType(tokenId,s) == SVS_TOKEN_EQUALS) { //==
      tokenId++;
      exprExecDMSG("ExprExecLvl1: == ", prac.value.val_s, tokenId);
      exprExecLvl2(tokenId, &prac, s);
      if (errCheck(s)) {
        return;
      }
      if (result->type == prac.type) { //typová kontrola/type check
        if (result->type == SVS_TYPE_NUM) {
          if (result->value.val_s == prac.value.val_s) { //porovnání dvou čísel / comparing num types
            result->value.val_s = 1;
            result->type = 0;
            exprExecDMSG("ExprExecLvl1: == (NUM) TRUE", prac.value.val_s,tokenId);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: == (NUM) FALSE", prac.value.val_s,tokenId);
            result->tokenId = prac.tokenId;
          }
        }else if (result->type == SVS_TYPE_STR) {
          if (strCmp(s->stringField + result->value.val_str, s->stringField + prac.value.val_str) == 1) {
            result->value.val_s = 1;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: == (STR) TRUE", prac.value.val_s, tokenId);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: == (STR) FALSE", prac.value.val_s, tokenId);
            result->tokenId = prac.tokenId;
          }
        }else if (result->type == SVS_TYPE_FLT) {
        #ifdef USE_FLOAT
          if (svsFloatCompare(result->value.val_f, prac.value.val_f)) { //porovnání dvou floatů / comparing float types
            result->value.val_s = 1;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: == (FLT) TRUE", prac.value.val_s, tokenId);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: == (FLT) FALSE", prac.value.val_s, tokenId);
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
      exprExecDMSG("ExprExecLvl1: != ", prac.value.val_s, tokenId);
      exprExecLvl2(tokenId, &prac, s);
      if (errCheck(s)) {
        return;
      }
      if (result->type == prac.type) { //typová kontrola / type check
        if (result->type == SVS_TYPE_NUM) {
          if (result->value.val_s != prac.value.val_s) { //porovnání dvou čísel
            result->value.val_s = 1;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: != (NUM) TRUE", prac.value.val_str, tokenId);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: != (NUM) FALSE", prac.value.val_str, tokenId);
            result->tokenId = prac.tokenId;
          }
        }else if (result->type == SVS_TYPE_FLT) {
        #ifdef USE_FLOAT
          if (!svsFloatCompare(result->value.val_f, prac.value.val_f)) { //porovnání dvou floatů
            result->value.val_s = 1;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: != (FLT) TRUE", prac.value.val_str, tokenId);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: != (FLT) FALSE", prac.value.val_str, tokenId);
            result->tokenId = prac.tokenId;
          }
        #endif
        } else if (result->type == SVS_TYPE_STR) {
          if (strCmp(s->stringField + result->value.val_str, s->stringField + prac.value.val_str) != 1) {
            result->value.val_s = 1;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: != (STR) TRUE", prac.value.val_s, tokenId);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: != (STR) FALSE", prac.value.val_s, tokenId);
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
      exprExecDMSG("ExprExecLvl1: < ", prac.value.val_s, tokenId);
      exprExecLvl2(tokenId, &prac, s);
      if (errCheck(s)) {
        return;
      }
      if (result->type == prac.type) {
        if (result->type == SVS_TYPE_NUM) {
          if (result->value.val_s < prac.value.val_s) { //<
            result->value.val_s = 1;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: < (NUM) TRUE", prac.value.val_s, tokenId);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: < (NUM) FALSE", prac.value.val_s, tokenId);
            result->tokenId = prac.tokenId;
          }
        }else if (result->type == SVS_TYPE_FLT) {
        #ifdef USE_FLOAT
          if (result->value.val_f < prac.value.val_f) { //<
            result->value.val_s = 1;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: < (FLT) TRUE", prac.value.val_s, tokenId);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: < (FLT) FALSE", prac.value.val_s, tokenId);
            result->tokenId = prac.tokenId;
          }
        #endif
        } else {
          errSoft((uint8_t *)"Can not use < operator on strings!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
          errSoftSetToken(tokenId,s);
          return;
        }
      } else {
        errSoft((uint8_t *)"Can not compare (<) string and num!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      }
    }else if (getTokenType(tokenId, s) == SVS_TOKEN_GREATER_THAN) { //>
      tokenId++;
      exprExecDMSG("ExprExecLvl1: > ", prac.value.val_s, tokenId);
      exprExecLvl2(tokenId, &prac, s);
      if (errCheck(s)) {
        return;
      }
      if (result->type == prac.type) {
        if (result->type == SVS_TYPE_NUM) {
          if (result->value.val_s > prac.value.val_s) { //<
            result->value.val_s = 1;
            result->type = 0;
            exprExecDMSG("ExprExecLvl1: > (NUM) TRUE", prac.value.val_s, tokenId);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = 0;
            exprExecDMSG("ExprExecLvl1: > (NUM) FALSE", prac.value.val_s, tokenId);
            result->tokenId = prac.tokenId;
          }
        }else if (result->type == 3) {
        #ifdef USE_FLOAT
          if (result->value.val_f > prac.value.val_f) { //<
            result->value.val_s = 1;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: > (FLT) TRUE", prac.value.val_s, tokenId);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: > (FLT) FALSE", prac.value.val_s, tokenId);
            result->tokenId = prac.tokenId;
          }
        #endif
        } else {
          errSoft((uint8_t *)"Can not use > operator on strings!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
          errSoftSetToken(tokenId, s);
          return;
        }
      } else {
        errSoft((uint8_t *)"Can only compare (>) same types", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      }

    } else if (getTokenType(tokenId, s) == SVS_TOKEN_LESS_OR_EQ) { //<=
      tokenId++;
      exprExecDMSG("ExprExecLvl1: <= ", prac.value.val_s, tokenId);
      exprExecLvl2(tokenId, &prac, s);
      if (errCheck(s)) {
        return;
      }
      if (result->type == prac.type) { //typová kontrola
        if (result->type == SVS_TYPE_NUM) {
          if (result->value.val_s <= prac.value.val_s) { //<=
            result->value.val_s = 1;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: <= (NUM) TRUE", prac.value.val_s, tokenId);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: <= (NUM) FALSE", prac.value.val_s, tokenId);
            result->tokenId = prac.tokenId;
          }
        } else if (result->type == SVS_TYPE_FLT) {
        #ifdef USE_FLOAT
          if (result->value.val_f <= prac.value.val_f) { //<=
            result->value.val_s = 1;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: <= (FLT) TRUE", prac.value.val_s, tokenId);
            result->tokenId = prac.tokenId;
          }else{
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: <= (FLT) FALSE", prac.value.val_s, tokenId);
            result->tokenId = prac.tokenId;
          }
        #endif
        } else {
          errSoft((uint8_t *)"Can not use <= operator on strings!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
          errSoftSetToken(tokenId, s);
          return;
        }
      } else {
        errSoft((uint8_t *)"Can only compare (<=) same types", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      }
    } else if (getTokenType(tokenId, s) == SVS_TOKEN_GREATER_OR_EQ) { //>=
      tokenId++;
      exprExecDMSG("ExprExecLvl1: >= ", prac.value.val_s, tokenId);
      exprExecLvl2(tokenId, &prac, s);
      if (errCheck(s)) {
        return;
      }
      if (result->type == prac.type) { //typová kontrola
        if (result->type == SVS_TYPE_NUM) {
          if (result->value.val_s >= prac.value.val_s) { //>=
            result->value.val_s = 1;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: >= (NUM) TRUE", prac.value.val_s, tokenId);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: >= (NUM) FALSE", prac.value.val_s, tokenId);
            result->tokenId = prac.tokenId;
          }
        }else if (result->type == SVS_TYPE_FLT) {
        #ifdef USE_FLOAT
          if (result->value.val_f >= prac.value.val_f) { //>=
            result->value.val_s = 1;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: >= (FLT) TRUE", prac.value.val_s, tokenId);
            result->tokenId = prac.tokenId;
          } else {
            result->value.val_s = 0;
            result->type = SVS_TYPE_NUM;
            exprExecDMSG("ExprExecLvl1: >= (FLT) FALSE", prac.value.val_s, tokenId);
            result->tokenId = prac.tokenId;
          }
        #endif
        } else {
          errSoft((uint8_t *)"Can not use >= operator on strings!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
          errSoftSetToken(tokenId, s);
          return;
        }
      } else {
        errSoft((uint8_t *)"Can only compare (>=) same types", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      }
    }
  }

  exprExecDMSG("ExprExecLvl1 Exit", result->value.val_u, result->tokenId);
  return;
}

void exprExecLvlLogic(uint16_t index, varRetVal *result, svsVM *s) {
  // AND OR
  varRetVal prac;
  uint16_t tokenId;

  varRetValZero(&prac);

  tokenId = index;
  exprExecDMSG("ExprExecLvlLogic (0) Begin", result->value.val_s, tokenId);
  exprExecLvl1(tokenId, result, s);
  if (errCheck(s)) {
    return;
  }
  tokenId = result->tokenId;

  while ((getTokenType(tokenId, s) == SVS_TOKEN_AND) || (getTokenType(tokenId, s) == SVS_TOKEN_OR)) {
    if (getTokenType(tokenId, s) == SVS_TOKEN_AND) { // and
      exprExecDMSG("ExprExecLvlLogic AND operator", result->value.val_s, tokenId);
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
        exprExecDMSG("ExprExecLogic AND: ", result->value.val_s, tokenId);
      } else {
        errSoft((uint8_t *)"Can only use logic operators (AND) on num type!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      }
    }else  if (getTokenType(tokenId,s) == SVS_TOKEN_OR) { // or
      exprExecDMSG("ExprExecLvlLogic OR operator", result->value.val_s, tokenId);
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
        exprExecDMSG("ExprExecLogic OR: ", result->value.val_s, tokenId);
      } else {
        errSoft((uint8_t *)"Can only use logic operators (OR) on num type!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)tokenId, s);
        errSoftSetToken(tokenId, s);
        return;
      }
    }
  }

  exprExecDMSG("ExprExecLvlLogic (0) Exit", result->value.val_s, result->tokenId);
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

  exprExecDMSG("ExprExec Begin", result->value.val_s, tokenId);

  exprExecLvlLogic(tokenId, result, s);
  if (errCheck(s)) {
    return;
  }
  tokenId = result->tokenId;

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

  exprExecDMSG("ExprExec Exit",result->value.val_u,result->tokenId);

  return;
}
