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

#include "comm_exec_var_op.h"


uint8_t comm_exec_var_op(uint16_t *token, svsVM * s) {
    uint16_t x;
    varRetVal varPrac;
    volatile uint16_t currToken = *token;

    //pokud narazíme na typ VAR / if we found a variable
    x = currToken; //uložíme token indexu promněnné / we store index of the variable
    currToken++;
    if (getTokenType(currToken, s) == SVS_TOKEN_ASSIGN) { // =
      commExDMSG("commExecLoop: = statement", currToken, s);
      if (varGetType(getTokenData(x, s), s) == SVS_TYPE_ARR) {
        errSoft((uint8_t *)"commEx: Assign on array is not supported.", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
        errSoftSetToken(currToken, s);
        return 0;
      }

      currToken++;

      exprExec(currToken, &varPrac, s); //vykonáme výraz za = / executing expression after =
      if (errCheck(s)) {
        return 0;
      }
      varSetVal(getTokenData(x, s), varPrac.value, s); //nastavíme value / we set new value
      varSetType(getTokenData(x, s), varPrac.type, s); //nastavíme typ / we set new type

      if (varPrac.type == 0) {
        commExDMSG("commExecLoop: = statement: result is NUM", currToken, s);
      } else {
        commExDMSG("commExecLoop: = statement: result is STR", currToken, s);
      }

      currToken = varPrac.tokenId; //nastavíme token kde se má pokračovat / we set the token id we got from exprExec
      commExDMSG("commExecLoop: = statement: continue on token:", currToken, s);

    } else if(getTokenType(currToken, s) == SVS_TOKEN_ADD) { // ++
      currToken++;
      if (getTokenType(currToken, s) == SVS_TOKEN_ADD) {
        commExDMSG("commExecLoop: ++ statement", currToken, s);

        if (varGetType(getTokenData(x, s), s) == SVS_TYPE_UNDEF) {
          if (getUndefWarning()) {
            printf("Warning: ++ operator used on a uninitialized variable!\nThis will produce error in a future release.\n");
          }
          varSetType(getTokenData(x, s), SVS_TYPE_NUM ,s);
        }

        if (varGetType(getTokenData(x,s), s) != SVS_TYPE_NUM) {
          errSoft((uint8_t *)"commEx: Syntax error in ++: only num type can be incremented.", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
          errSoftSetToken(currToken, s);
          return 0;
        }
        varType prac;
        prac = varGetVal(getTokenData(x,s),s);
        varSetVal(getTokenData(x,s), (varType)(prac.val_s + (int32_t)1), s);

        currToken++;
      } else if(getTokenType(currToken, s) == SVS_TOKEN_ASSIGN) {
        if (varGetType(getTokenData(x, s), s) == SVS_TYPE_UNDEF) {
          errSoft((uint8_t *)"commEx: Error: \"+=\" operator used on a uninitialized variable!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
          errSoftSetToken(currToken, s);
          return 0;
        }

        currToken++;
        exprExec(currToken, &varPrac, s);
        if (errCheck(s)) {
          return 0;
        }

        // inline += on type NUM
        if (varGetType(getTokenData(x, s), s) == SVS_TYPE_NUM) {

          if (varPrac.type == SVS_TYPE_NUM) {
            varSetVal(
              getTokenData(x, s),
              (varType)((varGetVal(getTokenData(x, s), s)).val_s + (varPrac.value).val_s),
              s
            );
          } else if (varPrac.type == SVS_TYPE_FLT) {
            errSoft((uint8_t *)"commEx: Error: \"+=\" operator: Can not add FLT to NUM!", s);
            errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
            errSoftSetToken(currToken, s);
            return 0;
          } else if (varPrac.type == SVS_TYPE_STR) {
            varSetType(getTokenData(x, s), SVS_TYPE_STR, s);
            varSetVal(
              getTokenData(x, s),
              (varType)strAdd( i16toString(varGetVal(getTokenData(x, s), s), s).val_str, (varPrac.value).val_str, s),
              s
            );
          }
        // inline += on type STR
        } else if (varGetType(getTokenData(x, s), s) == SVS_TYPE_STR) {
          if (varPrac.type == SVS_TYPE_NUM) {
            varSetVal(
              getTokenData(x, s),
              (varType)strAdd((varGetVal(getTokenData(x, s), s)).val_str, i16toString(varPrac.value, s).val_str, s),
              s
            );
          } else if (varPrac.type == SVS_TYPE_FLT) {
            varSetVal(
              getTokenData(x, s),
              (varType)strAdd((varGetVal(getTokenData(x, s), s)).val_str, floatToString(varPrac.value, s).val_str, s),
              s
            );
          } else if (varPrac.type == SVS_TYPE_STR) {
            varSetVal(
              getTokenData(x, s),
              (varType)strAdd( varGetVal(getTokenData(x, s), s).val_str, (varPrac.value).val_str, s),
              s
            );
          }
        } else if (varGetType(getTokenData(x, s), s) == SVS_TYPE_FLT) {
          if (varPrac.type == SVS_TYPE_NUM) {
            errSoft((uint8_t *)"commEx: Error: \"+=\" operator: Can not add FLT to NUM!", s);
            errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
            errSoftSetToken(currToken, s);
            return 0;
          } else if (varPrac.type == SVS_TYPE_FLT) {
            varSetVal(
              getTokenData(x, s),
              (varType)((varGetVal(getTokenData(x, s), s)).val_f + (varPrac.value).val_f),
              s
            );
          } else if (varPrac.type == SVS_TYPE_STR) {
            varSetType(getTokenData(x, s), SVS_TYPE_STR, s);
            varSetVal(
              getTokenData(x, s),
              (varType)strAdd(floatToString(varGetVal(getTokenData(x, s), s), s).val_str, (varPrac.value).val_str, s),
              s
            );
          }
        } else {
          errSoft((uint8_t *)"commEx: Error: \"+=\" operator could be used only on type NUM, STR or FLT!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
          errSoftSetToken(currToken, s);
          return 0;
        }

        currToken = varPrac.tokenId;
      } else {
        errSoft((uint8_t *)"commEx: Syntax error next to + (missing \"+\" or \"=\").", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
        errSoftSetToken(currToken, s);
        return 0;
      }

    } else if(getTokenType(currToken, s) == SVS_TOKEN_SUBT) { // --, -=

      currToken++;
      if (getTokenType(currToken, s) == SVS_TOKEN_SUBT) {
        commExDMSG("commExecLoop: -- statement", currToken, s);
        if (varGetType(getTokenData(x, s), s) == SVS_TYPE_UNDEF) {
          if (getUndefWarning()) {
            printf("Warning: -- operator used on a uninitialized variable!\nThis will produce error in a future release.\n");
          }
          varSetType(getTokenData(x, s), SVS_TYPE_NUM ,s);
        }

        if (varGetType(getTokenData(x, s), s) != SVS_TYPE_NUM) {
          errSoft((uint8_t *)"commEx: Syntax error in --: only num type can be decremented.", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
          errSoftSetToken(currToken, s);
          return 0;
        }
        varType prac;
        prac = varGetVal(getTokenData(x, s), s);
        varSetVal(getTokenData(x, s), (varType)(prac.val_s - (int32_t)1), s);

        currToken++;
      } else if (getTokenType(currToken, s) == SVS_TOKEN_ASSIGN) {
        commExDMSG("commExecLoop: -= statement", currToken, s);
        if (varGetType(getTokenData(x, s), s) == SVS_TYPE_UNDEF) {
          errSoft((uint8_t *)"commEx: Error: \"-=\" operator used on a uninitialized variable!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
          errSoftSetToken(currToken, s);
          return 0;
        }

        currToken++;
        exprExec(currToken, &varPrac, s);
        if (errCheck(s)) {
          return 0;
        }

        if (varGetType(getTokenData(x, s), s) == SVS_TYPE_NUM) {
          if (varPrac.type == SVS_TYPE_NUM) {
            varSetVal(
              getTokenData(x, s),
              (varType)((varGetVal(getTokenData(x, s), s)).val_s - (varPrac.value).val_s),
              s
            );
          } else {
            errSoft((uint8_t *)"commEx: Error: \"-=\" operator only works on NUM or FLT!", s);
            errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
            errSoftSetToken(currToken, s);
            return 0;
          }
        } else if (varGetType(getTokenData(x, s), s) == SVS_TYPE_FLT) {
          if (varPrac.type == SVS_TYPE_FLT) {
            varSetVal(
              getTokenData(x, s),
              (varType)((varGetVal(getTokenData(x, s), s)).val_f - (varPrac.value).val_f),
              s
            );
          } else {
            errSoft((uint8_t *)"commEx: Error: \"-=\" operator only works on NUM or FLT!", s);
            errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
            errSoftSetToken(currToken, s);
            return 0;
          }
        } else {
          errSoft((uint8_t *)"commEx: Error: \"-=\" operator only works on NUM or FLT!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
          errSoftSetToken(currToken, s);
          return 0;
        }

        currToken = varPrac.tokenId;
      } else {
        errSoft((uint8_t *)"commEx: Syntax error next to - (missing \"-\" or \"=\").", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
        errSoftSetToken(currToken, s);
        return 0;
      }
    } else if(getTokenType(currToken, s) == SVS_TOKEN_DIV) {
      currToken++;

      if (getTokenType(currToken, s) == SVS_TOKEN_ASSIGN) {
        commExDMSG("commExecLoop: /= statement", currToken, s);
        if (varGetType(getTokenData(x, s), s) == SVS_TYPE_UNDEF) {
          errSoft((uint8_t *)"commEx: Error: \"/=\" operator used on a uninitialized variable!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
          errSoftSetToken(currToken, s);
          return 0;
        }

        currToken++;
        exprExec(currToken, &varPrac, s);
        if (errCheck(s)) {
          return 0;
        }

        if (varGetType(getTokenData(x, s), s) == SVS_TYPE_NUM) {
          if (varPrac.type == SVS_TYPE_NUM) {
            varSetVal(
              getTokenData(x, s),
              (varType)((varGetVal(getTokenData(x, s), s)).val_s / (varPrac.value).val_s),
              s
            );
          } else {
            errSoft((uint8_t *)"commEx: Error: \"/=\" operator only works on NUM or FLT!", s);
            errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
            errSoftSetToken(currToken, s);
            return 0;
          }
        } else if (varGetType(getTokenData(x, s), s) == SVS_TYPE_FLT) {
          if (varPrac.type == SVS_TYPE_FLT) {
            varSetVal(
              getTokenData(x, s),
              (varType)((varGetVal(getTokenData(x, s), s)).val_f / (varPrac.value).val_f),
              s
            );
          } else {
            errSoft((uint8_t *)"commEx: Error: \"/=\" operator only works on NUM or FLT!", s);
            errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
            errSoftSetToken(currToken, s);
            return 0;
          }
        } else {
          errSoft((uint8_t *)"commEx: Error: \"/=\" operator only works on NUM or FLT!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
          errSoftSetToken(currToken, s);
          return 0;
        }

        currToken = varPrac.tokenId;
      } else {
        errSoft((uint8_t *)"commEx: Syntax error next to / (missing \"=\").", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
        errSoftSetToken(currToken, s);
        return 0;
      }

    } else if(getTokenType(currToken, s) == SVS_TOKEN_MUL) {
      currToken++;

      if (getTokenType(currToken, s) == SVS_TOKEN_ASSIGN) {
        commExDMSG("commExecLoop: *= statement", currToken, s);
        if (varGetType(getTokenData(x, s), s) == SVS_TYPE_UNDEF) {
          errSoft((uint8_t *)"commEx: Error: \"*=\" operator used on a uninitialized variable!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
          errSoftSetToken(currToken, s);
          return 0;
        }

        currToken++;
        exprExec(currToken, &varPrac, s);
        if (errCheck(s)) {
          return 0;
        }

        if (varGetType(getTokenData(x, s), s) == SVS_TYPE_NUM) {
          if (varPrac.type == SVS_TYPE_NUM) {
            varSetVal(
              getTokenData(x, s),
              (varType)((varGetVal(getTokenData(x, s), s)).val_s * (varPrac.value).val_s),
              s
            );
          } else {
            errSoft((uint8_t *)"commEx: Error: \"/=\" operator only works on NUM or FLT!", s);
            errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
            errSoftSetToken(currToken, s);
            return 0;
          }
        } else if (varGetType(getTokenData(x, s), s) == SVS_TYPE_FLT) {
          if (varPrac.type == SVS_TYPE_FLT) {
            varSetVal(
              getTokenData(x, s),
              (varType)((varGetVal(getTokenData(x, s), s)).val_f * (varPrac.value).val_f),
              s
            );
          } else {
            errSoft((uint8_t *)"commEx: Error: \"*=\" operator only works on NUM or FLT!", s);
            errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
            errSoftSetToken(currToken, s);
            return 0;
          }
        } else {
          errSoft((uint8_t *)"commEx: Error: \"*=\" operator only works on NUM or FLT!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
          errSoftSetToken(currToken, s);
          return 0;
        }

        currToken = varPrac.tokenId;
      } else {
        errSoft((uint8_t *)"commEx: Syntax error next to * (missing \"=\").", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
        errSoftSetToken(currToken, s);
        return 0;
      }

    } else if(getTokenType(currToken, s) == SVS_TOKEN_MOD) {
      currToken++;

      if (getTokenType(currToken, s) == SVS_TOKEN_ASSIGN) {
        commExDMSG("commExecLoop: %= statement", currToken, s);
        if (varGetType(getTokenData(x, s), s) == SVS_TYPE_UNDEF) {
          errSoft((uint8_t *)"commEx: Error: \"%=\" operator used on a uninitialized variable!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
          errSoftSetToken(currToken, s);
          return 0;
        }

        currToken++;
        exprExec(currToken, &varPrac, s);
        if (errCheck(s)) {
          return 0;
        }

        if (varGetType(getTokenData(x, s), s) == SVS_TYPE_NUM) {
          if (varPrac.type == SVS_TYPE_NUM) {
            varSetVal(
              getTokenData(x, s),
              (varType)((varGetVal(getTokenData(x, s), s)).val_s % (varPrac.value).val_s),
              s
            );
          } else {
            errSoft((uint8_t *)"commEx: Error: \"%=\" operator only works on NUM!", s);
            errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
            errSoftSetToken(currToken, s);
            return 0;
          }
        } else {
          errSoft((uint8_t *)"commEx: Error: \"*=\" operator only works on NUM or FLT!", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
          errSoftSetToken(currToken, s);
          return 0;
        }

        currToken = varPrac.tokenId;
      } else {
        errSoft((uint8_t *)"commEx: Syntax error next to * (missing \"=\").", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
        errSoftSetToken(currToken, s);
        return 0;
      }

    } else if (getTokenType(currToken, s) == SVS_TOKEN_LSQB) { // []
      varType index;

      if (varGetType(getTokenData(x, s), s) != SVS_TYPE_ARR) {
        errSoft((uint8_t *)"commEx: Only array type can be indexed.", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
        errSoftSetToken(currToken, s);
        return 0;
      }

      currToken++;

      exprExec(currToken, &varPrac, s);
      if (errCheck(s)) {
        return 0;
      }

      if (varPrac.type != SVS_TYPE_NUM || varPrac.value.val_s < 0) {
        errSoft((uint8_t *)"commEx: Error next to ARRAY: Index can be positive num only.", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
        errSoftSetToken(currToken, s);
        return 0;
      }
      currToken = varPrac.tokenId;
      index = varPrac.value;

      if (getTokenType(currToken, s) != SVS_TOKEN_RSQB) {
        errSoft((uint8_t *)"commEx: Syntax error, missing ].", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
        errSoftSetToken(currToken, s);
        return 0;
      }

      currToken++;

      if (getTokenType(currToken, s) != SVS_TOKEN_ASSIGN) {
        errSoft((uint8_t *)"commEx: Syntax error, missing =.", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
        errSoftSetToken(currToken, s);
        return 0;
      }

      currToken++;

      // exec
      exprExec(currToken, &varPrac, s);
      if (errCheck(s)) {
        return 0;
      }

      currToken = varPrac.tokenId;

      if (index.val_s + varGetVal(getTokenData(x, s), s).val_s > SVS_ARRAY_LEN) {
        errSoft((uint8_t *)"commEx: Array out of range!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
        errSoftSetToken(currToken, s);
        return 0;
      } else {
        s->varArray[index.val_s + varGetVal(getTokenData(x, s), s).val_s] = varPrac.value;
        s->varArrayType[index.val_s + varGetVal(getTokenData(x, s), s).val_s] = varPrac.type;
      }

    } else { //očekáváme "=" / expecting "="
      errSoft((uint8_t *)"commEx: Syntax error next to VAR (missing \"=\", \"++\", \"--\" or \"[\").", s);
      errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
      errSoftSetToken(currToken, s);
      return 0;
    }

    *token = currToken;
    return 1;
}
