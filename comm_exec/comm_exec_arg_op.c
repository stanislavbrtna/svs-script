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


uint8_t comm_exec_arg_op(uint16_t *token, svsVM * s) {
    uint16_t x;
    varRetVal varPrac;
    volatile uint16_t currToken = *token;

    varType arg_value;
    uint8_t arg_type;

    x = currToken; //uložíme token indexu promněnné / we store index of the variable

    arg_value = s->commArgs.arg[(uint16_t)getTokenData(x, s).val_u + 1];
    arg_type = s->commArgs.argType[(uint16_t)getTokenData(x, s).val_u + 1];

    currToken++;
    if (getTokenType(currToken, s) == SVS_TOKEN_ASSIGN) { // =
      commExDMSG("commExecLoop: ARG = statement", currToken, s);
      if (arg_type == SVS_TYPE_ARR) {
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

      s->commArgs.arg[(uint16_t)getTokenData(x, s).val_u + 1] = varPrac.value; // set value
      s->commArgs.argType[(uint16_t)getTokenData(x, s).val_u + 1] = varPrac.type; // set type

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

        if (arg_type == SVS_TYPE_UNDEF) {
          if (getUndefWarning()) {
            printf("Warning: ++ operator used on a uninitialized argument!\n");
          }
          s->commArgs.argType[(uint16_t)getTokenData(x, s).val_u + 1] = SVS_TYPE_NUM;
        }

        if (arg_type != SVS_TYPE_NUM) {
          errSoft((uint8_t *)"commEx: Syntax error in ++: only num type can be incremented.", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
          errSoftSetToken(currToken, s);
          return 0;
        }
        s->commArgs.arg[(uint16_t)getTokenData(x, s).val_u + 1] = (varType)(arg_value.val_s + (int32_t)1);

        currToken++;
      } else if(getTokenType(currToken, s) == SVS_TOKEN_ASSIGN) {
        if (arg_type == SVS_TYPE_UNDEF) {
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
        if (arg_type == SVS_TYPE_NUM) {

          if (varPrac.type == SVS_TYPE_NUM) {
            s->commArgs.arg[(uint16_t)getTokenData(x, s).val_u + 1] = (varType)((arg_value).val_s + (varPrac.value).val_s);
          } else if (varPrac.type == SVS_TYPE_FLT) {
            errSoft((uint8_t *)"commEx: Error: \"+=\" operator: Can not add FLT to NUM!", s);
            errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
            errSoftSetToken(currToken, s);
            return 0;
          } else if (varPrac.type == SVS_TYPE_STR) {
            s->commArgs.argType[(uint16_t)getTokenData(x, s).val_u + 1] = SVS_TYPE_STR;
            s->commArgs.arg[(uint16_t)getTokenData(x, s).val_u + 1] = (varType)strAdd( i16toString(arg_value, s).val_str, (varPrac.value).val_str, s);
          }
        // inline += on type STR
        } else if (arg_type == SVS_TYPE_STR) {
          if (varPrac.type == SVS_TYPE_NUM) {
            s->commArgs.arg[(uint16_t)getTokenData(x, s).val_u + 1] = (varType)strAdd((arg_value).val_str, i16toString(varPrac.value, s).val_str, s);
          } else if (varPrac.type == SVS_TYPE_FLT) {
            s->commArgs.arg[(uint16_t)getTokenData(x, s).val_u + 1] = (varType)strAdd((arg_value).val_str, floatToString(varPrac.value, s).val_str, s);
          } else if (varPrac.type == SVS_TYPE_STR) {
            s->commArgs.arg[(uint16_t)getTokenData(x, s).val_u + 1] = (varType)strAdd( arg_value.val_str, (varPrac.value).val_str, s);
          }
        } else if (arg_type == SVS_TYPE_FLT) {
          if (varPrac.type == SVS_TYPE_NUM) {
            errSoft((uint8_t *)"commEx: Error: \"+=\" operator: Can not add FLT to NUM!", s);
            errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
            errSoftSetToken(currToken, s);
            return 0;
          } else if (varPrac.type == SVS_TYPE_FLT) {
            s->commArgs.arg[(uint16_t)getTokenData(x, s).val_u + 1] = (varType)((arg_value).val_f + (varPrac.value).val_f);
          } else if (varPrac.type == SVS_TYPE_STR) {
            s->commArgs.argType[(uint16_t)getTokenData(x, s).val_u + 1] = SVS_TYPE_STR;
            s->commArgs.arg[(uint16_t)getTokenData(x, s).val_u + 1] = (varType)strAdd(floatToString(arg_value, s).val_str, (varPrac.value).val_str, s);
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
        if (arg_type == SVS_TYPE_UNDEF) {
          if (getUndefWarning()) {
            printf("Warning: -- operator used on a uninitialized variable!\nThis will produce error in a future release.\n");
          }
          s->commArgs.argType[(uint16_t)getTokenData(x, s).val_u + 1] = SVS_TYPE_NUM;
        }

        if (arg_type != SVS_TYPE_NUM) {
          errSoft((uint8_t *)"commEx: Syntax error in --: only num type can be decremented.", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
          errSoftSetToken(currToken, s);
          return 0;
        }
        varType prac;
        prac = arg_value;
        s->commArgs.arg[(uint16_t)getTokenData(x, s).val_u + 1] = (varType)(prac.val_s - (int32_t)1);

        currToken++;
      } else if (getTokenType(currToken, s) == SVS_TOKEN_ASSIGN) {
        commExDMSG("commExecLoop: -= statement", currToken, s);
        if (arg_type == SVS_TYPE_UNDEF) {
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

        if (arg_type == SVS_TYPE_NUM) {
          if (varPrac.type == SVS_TYPE_NUM) {
            s->commArgs.arg[(uint16_t)getTokenData(x, s).val_u + 1] = (varType)((arg_value).val_s - (varPrac.value).val_s);
          } else {
            errSoft((uint8_t *)"commEx: Error: \"-=\" operator only works on NUM or FLT!", s);
            errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
            errSoftSetToken(currToken, s);
            return 0;
          }
        } else if (arg_type == SVS_TYPE_FLT) {
          if (varPrac.type == SVS_TYPE_FLT) {
            s->commArgs.arg[(uint16_t)getTokenData(x, s).val_u + 1] = (varType)((arg_value).val_f - (varPrac.value).val_f);
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
        if (arg_type == SVS_TYPE_UNDEF) {
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

        if (arg_type == SVS_TYPE_NUM) {
          if (varPrac.type == SVS_TYPE_NUM) {
            s->commArgs.arg[(uint16_t)getTokenData(x, s).val_u + 1] = (varType)((arg_value).val_s / (varPrac.value).val_s);
          } else {
            errSoft((uint8_t *)"commEx: Error: \"/=\" operator only works on NUM or FLT!", s);
            errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
            errSoftSetToken(currToken, s);
            return 0;
          }
        } else if (arg_type == SVS_TYPE_FLT) {
          if (varPrac.type == SVS_TYPE_FLT) {
            s->commArgs.arg[(uint16_t)getTokenData(x, s).val_u + 1] = (varType)((arg_value).val_f / (varPrac.value).val_f);
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
        if (arg_type == SVS_TYPE_UNDEF) {
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

        if (arg_type == SVS_TYPE_NUM) {
          if (varPrac.type == SVS_TYPE_NUM) {
            s->commArgs.arg[(uint16_t)getTokenData(x, s).val_u + 1] = (varType)((arg_value).val_s * (varPrac.value).val_s);
          } else {
            errSoft((uint8_t *)"commEx: Error: \"/=\" operator only works on NUM or FLT!", s);
            errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
            errSoftSetToken(currToken, s);
            return 0;
          }
        } else if (arg_type == SVS_TYPE_FLT) {
          if (varPrac.type == SVS_TYPE_FLT) {
            s->commArgs.arg[(uint16_t)getTokenData(x, s).val_u + 1] = (varType)((arg_value).val_f * (varPrac.value).val_f);
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
        if (arg_type == SVS_TYPE_UNDEF) {
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

        if (arg_type == SVS_TYPE_NUM) {
          if (varPrac.type == SVS_TYPE_NUM) {
            s->commArgs.arg[(uint16_t)getTokenData(x, s).val_u + 1] = (varType)((arg_value).val_s % (varPrac.value).val_s);
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
      varType array_index;

      if (arg_type != SVS_TYPE_ARR) {
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
      array_index = varPrac.value;

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

      if (arraySet(arg_value, array_index, varPrac.value, varPrac.type, s)) {
        errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
        errSoftSetToken(currToken, s);
        return 0;
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
