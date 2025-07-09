/*
Copyright (c) 2018 Stanislav Brtna

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

#include "svs_builtin_calls.h"
#ifdef SVS_USE_ADV_MATH
#include <math.h>
#endif

#ifdef SVS_RND_FUNCTION
  uint32_t SVS_RND_FUNCTION;
#endif

uint16_t callToken; // used to precisely locate errors

svsBuiltInCallsTableType svsBuiltInCallsTable[] = {
  {"num", NUM},
  {"float", FLOAT},
  {"print", PRINT},
  {"isnum", ISNUM},
  {"typeof", TYPEOF},
  // strings
  {"getcp", GETCP},
  {"len", LEN},
  {"rlen", RLEN},
  {"substr", SUBSTR},
  {"instr", INSTR},
  {"lower", LOWER},
  {"upper", UPPER},
  {"charval", CHARVAL},
  // advanced math
  {"sin", SIN},
  {"cos", COS},
  {"tan", TAN},
  {"atan", ATAN},
  {"atan2", ATAN2},
  {"log", LOG},
  {"exp", EXP},
  {"pow", POW},
  {"pi", PI},
  {"sqrt", SQRT},
  {"ver", VER},
  {"rnd", RND},
  // system calls
  {"call", CALL},
  {"dbg", DBG},
  {"gcprof", GCP},
  {"gc", GC},
  {"end", 0}
};


uint16_t getBuiltInCallId(uint8_t * str) {
  uint16_t x = 0;
  while (strCmp((uint8_t *)"end", (uint8_t *)svsBuiltInCallsTable[x].name) != 1) {
    if (strCmp(str, (uint8_t *)(svsBuiltInCallsTable[x].name))) {
      return svsBuiltInCallsTable[x].id;
    }
    x++;
  }
  return 0;
}


uint16_t processBuiltInCall(uint16_t index, varRetVal *result, svsVM *s) {
   uint16_t callId;
   uint16_t x = 1;
   varRetVal pracVar;
   varType args[FUNCTION_ARGS_MAX];
   uint8_t argType[FUNCTION_ARGS_MAX];

   callId = getTokenData(index, s).val_u;
   index++;

   //tady načteme nové
   if ((getTokenType(index, s) == 5)) {
    index++;
    if (getTokenType(index, s) != 6) { //pokud nemáme prázdnou závorku
      exprExec(index, &pracVar, s);
      if (errCheck(s)) {
        return 0;
      }
      args[x] = pracVar.value;
      argType[x] = pracVar.type;
      index = pracVar.tokenId;
      x++;
    }

    while((getTokenType(index, s) == 33)) { //argumenty odděleny čárkou
      if (x == FUNCTION_ARGS_MAX + 1) {
        errSoft((uint8_t *)"processBuiltInCall: too many arguments in function call!", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)index, s);
        errSoftSetToken(index, s);
        return 0;
      }
      index++;
      exprExec(index, &pracVar, s);
      if (errCheck(s)) {
        return 0;
      }
      args[x] = pracVar.value;
      argType[x] = pracVar.type;
      index = pracVar.tokenId;
      x++;
    }

    if (getTokenType(index, s) != 6) {
      errSoft((uint8_t *)"processBuiltInCall: Syntax error at end of function call. (missing \")\")", s);
      errSoftSetParam((uint8_t *)"TokenId", (varType)index, s);
      errSoftSetToken(index, s);
      return 0;
    }

    index++;
    varRetValZero(&pracVar);

    //Exec
    callToken = index;
    execBuiltInCall(callId, args, argType, x - 1, &pracVar, s);

    // Check
    if (errCheck(s)) {
      return 0;
    }

    // Return
    result->value = pracVar.value;
    result->type = pracVar.type;
    result->tokenId = index;
    return 1;

  } else {
    errSoft((uint8_t *)"processBuiltInCall: Syntax error at the begin of function call. (missing \"(\")", s);
    errSoftSetParam((uint8_t *)"TokenId", (varType)index, s);
    errSoftSetToken(index, s);
    return 0;
  }

}

static void simpleError(uint8_t * text, svsVM *s) {
  errSoft(text, s);
  errSoftSetParam((uint8_t *)"TokenId", (varType)callToken, s);
  errSoftSetToken(callToken, s);
}

uint16_t execBuiltInCall(builtinCallEnum callId, varType *args,  uint8_t * argType, uint16_t count, varRetVal *result, svsVM *s) {
  // num
  if (callId == NUM) {
    varType prac;
    uint8_t negative = 0;
    uint8_t hex = 0;
    uint16_t x = 0;
    prac.val_s = 0;

    if (count != 1) {
      simpleError((uint8_t *)"num(): wrong argument count!", s);
      return 0;
    }

    if (argType[1] == 1) { // from string
      prac.val_s = 0;

      while((!isNumber(s->stringField[args[1].val_str + x]))&&(s->stringField[args[1].val_str + x] != 0)) {
        x++;
      }

      if(s->stringField[args[1].val_str + x] == '-') {
        negative = 1;
        x++;
      }

      while((s->stringField[args[1].val_str+x] != 0)&&(s->stringField[args[1].val_str+x] != '.')) {
        
        if (s->stringField[args[1].val_str+x] == 'x') {
          hex = 1;
        }
        if (hex == 0) {
          if((s->stringField[args[1].val_str+x] >= '0') && (s->stringField[args[1].val_str+x]<='9')) {
            prac.val_s *= 10;
            prac.val_s += s->stringField[args[1].val_str + x] - 48;
          }
        } else {
          if((s->stringField[args[1].val_str+x] >= '0') && (s->stringField[args[1].val_str+x]<='9')) {
            prac.val_s *= 16;
            prac.val_s += s->stringField[args[1].val_str + x] - 48;
          }

          if((s->stringField[args[1].val_str+x] >= 'a') && (s->stringField[args[1].val_str+x]<='f')) {
            prac.val_s *= 16;
            prac.val_s += s->stringField[args[1].val_str + x] - 'a' + 10;
          }

          if((s->stringField[args[1].val_str+x] >= 'A') && (s->stringField[args[1].val_str+x]<='F')) {
            prac.val_s *= 16;
            prac.val_s += s->stringField[args[1].val_str + x] - 'A' + 10;
          }
        }
        
        
        x++;
      }

      if (negative) {
        prac.val_s *= -1;
      }

      result->value = prac;
      result->type = 0;
      return 1;
    }

    if (argType[1] == 3) { // from float
      result->value = (varType)((int32_t) args[1].val_f);
      result->type = 0;
      return 1;
    }

    if (argType[1] == 0) { // from num
      result->value = args[1];
      result->type = 0;
      return 1;
    }

    simpleError((uint8_t *)"num(): wrong type of argument!", s);
    return 0;
  }

  // flt(num) # takes num or str returns float
  if (callId == FLOAT) {

    if (count != 1) {
      simpleError((uint8_t *)"flt(): wrong argument count!", s);
      return 0;
    }

    if (argType[1] == 0) { // from num
      result->value = (varType)((float)args[1].val_s);
      result->type = 3;
      return 1;
    }

    if (argType[1] == 1) { // from str
      float fltPrac = 0;
      uint8_t negative = 0;
      uint16_t x = 0;
      uint16_t float_dp = 1;

      fltPrac = 0;

      while((!isNumber(s->stringField[args[1].val_str + x])) \
            && (s->stringField[args[1].val_str + x] != 0) \
            && (s->stringField[args[1].val_str + x] != '-')) {
        x++;
      }

      if(s->stringField[args[1].val_str + x] == '-') {
        negative = 1;
        x++;
      }

      while((s->stringField[args[1].val_str + x] != 0) && isNumber(s->stringField[args[1].val_str + x])) {
        fltPrac *= 10;
        fltPrac += (float)(s->stringField[args[1].val_str + x] - 48);
        x++;
      }

      // 1.234
      if (s->stringField[args[1].val_str + x] == '.') {
        x++;
        while((s->stringField[args[1].val_str + x] != 0) && isNumber(s->stringField[args[1].val_str + x])) {
            fltPrac += ((float)(s->stringField[args[1].val_str + x] - 48)/(float)((exp_helper(10, float_dp))));
            float_dp++;
            x++;
        }

        if (negative == 1) {
          fltPrac *= -1;
        }

        result->value = (varType)(fltPrac);
        result->type = 3;
        return 1;
      }

      // 1,256.789 or 1,236
      if (s->stringField[args[1].val_str + x] == ',') {
        x++;
        while((s->stringField[args[1].val_str + x] != 0)) {

          if (isNumber(s->stringField[args[1].val_str + x])) {
            fltPrac += ((float)(s->stringField[args[1].val_str + x] - 48)/(float)((exp_helper(10, float_dp))));
            float_dp++;
            x++;
          } else if ((s->stringField[args[1].val_str + x] == '.') && float_dp == 4) {
            fltPrac *= 1000;
            float_dp = 1;
            x++;
          } else {
            break;
          }
        }
      }

      if (negative == 1) {
        fltPrac *= -1;
      }

      result->value = (varType)(fltPrac);
      result->type = 3;
      return 1;
    }

    if (argType[1] == 3) { // from float
      result->value = args[1];
      result->type = 3;
      return 1;
    }

    simpleError((uint8_t *)"flt(): wrong type of argument!", s);
    return 0;
  }

  // print
  if (callId == PRINT) {
    if (count != 1 && count != 2) {
      simpleError((uint8_t *)"print(): wrong argument count!", s);
      return 0;
    }

    if ((count == 2) && (argType[2] != SVS_TYPE_STR)) {
      simpleError((uint8_t *)"print(): wrong type of argument 2!", s);
      return 0;
    }

    if (argType[1] == SVS_TYPE_STR) {
      if (count == 1) {
        printf("%s\n", (uint8_t *)(s->stringField + args[1].val_str));
      } else {
        printf(
                "%s%s",
                (uint8_t *)(s->stringField + args[1].val_str),
                (uint8_t *)(s->stringField + args[2].val_str)
              );
      }
      result->value = (varType)((int32_t)0);
      result->type = 0;
      return 1;
    }

    simpleError((uint8_t *)"print(): wrong type of argument!", s);
    return 0;
  }

  // isnum(str|num)
  if (callId == ISNUM) {

    if (count != 1) {
      simpleError((uint8_t *)"isnum(): wrong argument count!", s);
      return 0;
    }

    if (argType[1] == 1) {
      int32_t isNum = 1;
      int32_t isFlt = 0;
      uint8_t c;
      uint16_t x = 0;

      while (s->stringField[args[1].val_str + x] != 0) {
        c = s->stringField[args[1].val_str + x];
        // when char is not one that occurs in numbers/floats
        if (
            (!isNumber(c))
            && (c != ' ')
            && (c != '.')
            && (c != '-')
            && (c != ',')
        ){
          // then break
          isNum = 0;
          break;
        }

        // float detect
        if (c == '.' || c == ',') {
          isFlt = 1;
        }

        x++;
      }

      // when num return is 1, when float return is 2, otherwise zero
      result->value = (varType)(isNum + (isFlt * isNum));
      result->type = 0;
      return 1;
    }

    if (argType[1] == 0){
      result->value = (varType)((int32_t)1);
      result->type = 0;
      return 1;
    }

    if (argType[1] == 3){
      result->value = (varType)((int32_t)2);
      result->type = 0;
      return 1;
    }

    simpleError((uint8_t *)"isnum(): wrong type of argument!", s);
    return 0;
  }

  // typeof(var)
  if (callId == TYPEOF) {

    if (count != 1) {
      simpleError((uint8_t *)"typeof(): wrong argument count!", s);
      return 0;
    }

    result->value = (varType)((int32_t)argType[1]);
    result->type = 0;
    return 1;
  }

  // getcp
  if (callId == GETCP) {
    int32_t len = 1; // we start indexing strings from 1
    int32_t x = 0;
    uint8_t tmpChar[5];

    tmpChar[0] = 0;

    if (count != 2) {
      simpleError((uint8_t *)"getcp(): wrong argument count!", s);
      return 0;
    }

    if (argType[1] != 1){
      simpleError((uint8_t *)"getcp(): wrong type of argument 1!", s);
      return 0;
    }

    if (argType[2] != 0){
      simpleError((uint8_t *)"getcp(): wrong type of argument 2!", s);
      return 0;
    }

    while (s->stringField[args[1].val_str + x] != 0) {
      // success
      if (len == args[2].val_s) {

        if ((s->stringField[args[1].val_str + x] >= 0xC3) \
              && (s->stringField[args[1].val_str + x] <= 0xC5)) {
          tmpChar[0] = s->stringField[args[1].val_str + x];
          tmpChar[1] = s->stringField[args[1].val_str + x + 1];
          tmpChar[2] = 0;
          break;
        }
        
        if ((x > 1) && (s->stringField[args[1].val_str + x - 1] >= 0xC3) \
              && (s->stringField[args[1].val_str + x - 1] <= 0xC5)) {
          tmpChar[0] = s->stringField[args[1].val_str + x - 1];
          tmpChar[1] = s->stringField[args[1].val_str + x];
          tmpChar[2] = 0;
          break;
        }

        if (
          s->stringField[args[1].val_str + x] == 0xF0 &&
          s->stringField[args[1].val_str + x + 1] == 0x9F
        ){
          tmpChar[0] = s->stringField[args[1].val_str + x];
          tmpChar[1] = s->stringField[args[1].val_str + x + 1];
          tmpChar[2] = s->stringField[args[1].val_str + x + 2];
          tmpChar[3] = s->stringField[args[1].val_str + x + 3];
          tmpChar[4] = 0;
          break;
        }

        tmpChar[0] = s->stringField[args[1].val_str + x];
        tmpChar[1] = 0;
        break;
      }

      if (
        s->stringField[args[1].val_str + x] >= 0xC3 &&
        s->stringField[args[1].val_str + x] <= 0xC5
      ){
        x++;
      } else if (
        s->stringField[args[1].val_str + x] >= 0xF0 &&
        s->stringField[args[1].val_str + x + 1] == 0x9F
      ){
        x += 3;
      }

      len++;
      x++;
    }

    result->value = (varType)(strNew(tmpChar, s));

    if (errCheck(s)) {
      errSoftSetParam((uint8_t *)"TokenId", (varType)callToken, s);
      errSoftSetToken(callToken, s);
      return 0;
    }
    result->type = 1;
    return 1;
  }

  // len
  if (callId == LEN) {
    uint16_t len = 0;
    uint16_t x = 0;

    if (count != 1) {
      simpleError((uint8_t *)"len(): wrong argument count!", s);
      return 0;
    }

    if ((argType[1] != SVS_TYPE_STR) && (argType[1] != SVS_TYPE_ARR)) {
      simpleError((uint8_t *)"len(): wrong type of argument!", s);
      return 0;
    }

    if (argType[1] == SVS_TYPE_STR) {
      while (s->stringField[args[1].val_str + x] != 0) {
        // cz chars
        if ((s->stringField[args[1].val_str + x] >= 0xC3) \
            && (s->stringField[args[1].val_str + x] <= 0xC5)) {
          x++;
        }

        // emoji
        if ((s->stringField[args[1].val_str + x] == 0xF0) \
            && (s->stringField[args[1].val_str + x + 1] == 0x9F)) {
          x += 3;
        }

        len++;
        x++;
      }
      // to get just the characters, without end of string
      result->value = (varType)((int32_t)len);
      result->type = 0;
      return 1;
    } else {
      result->value = s->varArray[args[1].val_s];
      result->type = 0;
      return 1;
    }
  }


  // real len
  if (callId == RLEN) {
    int32_t x = 0;

    if (count != 1) {
      simpleError((uint8_t *)"rlen(): wrong argument count!", s);
      return 0;
    }

    if (argType[1] != SVS_TYPE_STR) {
      simpleError((uint8_t *)"rlen(): wrong type of argument!", s);
      return 0;
    }

    if (argType[1] == SVS_TYPE_STR) {
      while (s->stringField[args[1].val_str + x] != 0) {
        x++;
      }
      // to get just the characters, without end of string
      result->value = (varType)((int32_t)x);
      result->type = 0;
      return 1;
    }
  }
  
  // str2 = substr(str, begin, end);
  if (callId == SUBSTR) {
    int32_t x = 0;
    int32_t len = 1;

    if (count != 3) {
      simpleError((uint8_t *)"substr(): wrong argument count!", s);
      return 0;
    }

    if (argType[1] != SVS_TYPE_STR
        || argType[2] != SVS_TYPE_NUM
        || argType[3] != SVS_TYPE_NUM) {
      simpleError((uint8_t *)"substr(): wrong type of argument!", s);
      return 0;
    }

    if (args[2].val_s <= 0) {
      args[2].val_s = 0;
    }

    if (args[3].val_s <= 0) {
      args[3].val_s = 0;
    }

    strNewStreamInit(s);

    while (s->stringField[args[1].val_str + x] != 0) {

      if (len >= ((int32_t)args[2].val_u) && (len <= args[3].val_s)) {
        if(strNewStreamPush(s->stringField[args[1].val_str + x], s)) {
          simpleError((uint8_t *)"substr(): out of string memory!", s);
          return 0;
        }

        if ((s->stringField[args[1].val_str + x] >= 0xC3) \
            && (s->stringField[args[1].val_str + x] <= 0xC5)) {
          x++;
          if(strNewStreamPush(s->stringField[args[1].val_str + x], s)) {
            simpleError((uint8_t *)"substr(): out of string memory!", s);
            return 0;
          }
        }
        
        if ((s->stringField[args[1].val_str + x] == 0xF0) \
            && (s->stringField[args[1].val_str + x + 1] == 0x9F)) {
          x++;
          if(strNewStreamPush(s->stringField[args[1].val_str + x], s)) {
            simpleError((uint8_t *)"substr(): out of string memory!", s);
            return 0;
          }
          x++;
          if(strNewStreamPush(s->stringField[args[1].val_str + x], s)) {
            simpleError((uint8_t *)"substr(): out of string memory!", s);
            return 0;
          }
          x++;
          if(strNewStreamPush(s->stringField[args[1].val_str + x], s)) {
            simpleError((uint8_t *)"substr(): out of string memory!", s);
            return 0;
          }
        }

      } else {
        if ((s->stringField[args[1].val_str + x] >= 0xC3) \
            && (s->stringField[args[1].val_str + x] <= 0xC5)) {
          x++;
        }
      }
      
      len++;
      x++;

      if (len > args[3].val_s) {
        break;
      }
    }
    
    // to get just the characters, without end of string
    result->value = (varType)(strNewStreamEnd(s));
    result->type = SVS_TYPE_STR;
    return 1;
  }

  // contains = instr(str, sub_str);
  if (callId == INSTR) {
    uint16_t res = 0;
    uint16_t x = 0;
    uint16_t y = 0;

    if (count != 2) {
      simpleError((uint8_t *)"instr(): wrong argument count!", s);
      return 0;
    }

    if (argType[1] != SVS_TYPE_STR
        || argType[2] != SVS_TYPE_STR) {
      simpleError((uint8_t *)"instr(): wrong type of argument!", s);
      return 0;
    }

    while (s->stringField[args[1].val_str + x] != 0) {
      if (s->stringField[args[1].val_str + x] == s->stringField[args[2].val_str]) {
        uint8_t breaked = 0;
        y = 0;
        res = x;

        while (s->stringField[args[2].val_str + y] != 0 && s->stringField[args[1].val_str + x] != 0) {
          if (s->stringField[args[1].val_str + x] != s->stringField[args[2].val_str + y]) {
            breaked = 1;
            break;
          }
          x++;
          y++;
        }
        // out of str but not out of substr
        if (s->stringField[args[1].val_str + x] == 0 && s->stringField[args[2].val_str + y] != 0) {
          break;
        }

        if (!breaked) {
          result->value = (varType)(uint32_t)(1 + res);
          result->type = SVS_TYPE_NUM;
          return 1;
        }
      }
      x++;
    }
    // to get just the characters, without end of string
    result->value = (varType)((uint32_t)0);
    result->type = SVS_TYPE_NUM;
    return 1;
  }

  // str2 = lower(str);
  if (callId == LOWER) {
    uint16_t x = 0;

    if (count != 1) {
      simpleError((uint8_t *)"lower(): wrong argument count!", s);
      return 0;
    }

    if (argType[1] != SVS_TYPE_STR) {
      simpleError((uint8_t *)"lower(): wrong type of argument!", s);
      return 0;
    }

    strNewStreamInit(s);

    while (s->stringField[args[1].val_str + x] != 0) {
        if (s->stringField[args[1].val_str + x] >= 'A' && s->stringField[args[1].val_str + x] <= 'Z') {
          strNewStreamPush((s->stringField[args[1].val_str + x]) + 32, s);
        } else {
          strNewStreamPush((s->stringField[args[1].val_str + x]), s);
        }
      x++;
    }

    result->value = (varType)(strNewStreamEnd(s));
    result->type = SVS_TYPE_STR;
    return 1;
  }

  // num = charval(str); str = charval(num);
  // num = charval(str, byte);
  if (callId == CHARVAL) {
    if (argType[1] == SVS_TYPE_STR) {
      if (count != 1 && count != 2) {
        simpleError((uint8_t *)"charval(): wrong argument count!", s);
        return 0;
      }

      if(count == 2) {
        if(argType[2] != SVS_TYPE_NUM) {
          simpleError((uint8_t *)"charval(): wrong type of argument!", s);
          return 0;
        }

        uint8_t charLen = 0;

        if(
          s->stringField[args[1].val_str] >= 0xC3 &&
          s->stringField[args[1].val_str + 1] <= 0xC5
        ){
          charLen = 2;
        }

        if(
          s->stringField[args[1].val_str] == 0xF0 &&
          s->stringField[args[1].val_str + 1] == 0x9F
        ){
          charLen = 4;
        }

        if(args[2].val_s < charLen && args[2].val_s >= 0) {
          result->value = (varType) ((uint32_t)s->stringField[args[1].val_str + args[2].val_s]);
          result->type = SVS_TYPE_NUM;
          return 1;
        } else {
          result->value = (varType) (int32_t)-1;
          result->type = SVS_TYPE_NUM;
          return 1;
        }
      }
      
      result->value = (varType) ((uint32_t)s->stringField[args[1].val_str]);
      result->type = SVS_TYPE_NUM;
      return 1;
    }

    if (argType[1] == SVS_TYPE_NUM) {
      if (count != 1) {
        simpleError((uint8_t *)"charval(): wrong argument count!", s);
        return 0;
      }  
      strNewStreamInit(s);
      if (args[1].val_s < 256 && args[1].val_s > 0) {
        strNewStreamPush((uint8_t)args[1].val_s, s);
      }
      result->value = (varType)(strNewStreamEnd(s));
      result->type = SVS_TYPE_STR;
      return 1;
    }

    simpleError((uint8_t *)"charval(): wrong argument type!", s);
    return 0;
  }

  // str2 = upper(str);
  if (callId == UPPER) {
    uint16_t x = 0;

    if (count != 1) {
      simpleError((uint8_t *)"upper(): wrong argument count!", s);
      return 0;
    }

    if (argType[1] != SVS_TYPE_STR) {
      simpleError((uint8_t *)"upper(): wrong type of argument!", s);
      return 0;
    }

    strNewStreamInit(s);

    while (s->stringField[args[1].val_str + x] != 0) {
        if (s->stringField[args[1].val_str + x] >= 'a' && s->stringField[args[1].val_str + x] <= 'z') {
          strNewStreamPush((s->stringField[args[1].val_str + x]) - 32, s);
        } else {
          strNewStreamPush((s->stringField[args[1].val_str + x]), s);
        }
      x++;
    }

    result->value = (varType)(strNewStreamEnd(s));
    result->type = SVS_TYPE_STR;
    return 1;
  }

#ifdef SVS_USE_ADV_MATH
  // flt = sin([flt]rad)
  if (callId == SIN) {
    if (count != 1) {
      simpleError((uint8_t *)"sin(): wrong argument count!", s);
      return 0;
    }

    if (argType[1] != SVS_TYPE_FLT) {
      simpleError((uint8_t *)"sin(): wrong type of argument!", s);
      return 0;
    }

    result->value = (varType)(sinf(args[1].val_f));
    result->type = SVS_TYPE_FLT;
    return 1;
  }

  // flt = cos([flt]rad)
  if (callId == COS) {
    if (count != 1) {
      simpleError((uint8_t *)"cos(): wrong argument count!", s);
      return 0;
    }

    if (argType[1] != SVS_TYPE_FLT) {
      simpleError((uint8_t *)"cos(): wrong type of argument!", s);
      return 0;
    }

    result->value = (varType)(cosf(args[1].val_f));
    result->type = SVS_TYPE_FLT;
    return 1;
  }

  // flt = tan([flt]rad)
  if (callId == TAN) {
    if (count != 1) {
      simpleError((uint8_t *)"tan(): wrong argument count!", s);
      return 0;
    }

    if (argType[1] != SVS_TYPE_FLT) {
      simpleError((uint8_t *)"tan(): wrong type of argument!", s);
      return 0;
    }

    result->value = (varType)(tanf(args[1].val_f));
    result->type = SVS_TYPE_FLT;
    return 1;
  }

  // flt = atan([flt]rad)
  if (callId == ATAN) {
    if (count != 1) {
      simpleError((uint8_t *)"atan(): wrong argument count!", s);
      return 0;
    }

    if (argType[1] != SVS_TYPE_FLT) {
      simpleError((uint8_t *)"atan(): wrong type of argument!", s);
      return 0;
    }

    result->value = (varType)(atanf(args[1].val_f));
    result->type = SVS_TYPE_FLT;
    return 1;
  }

  // flt = atan2([flt]rad)
  if (callId == ATAN2) {
    if (count != 2) {
      simpleError((uint8_t *)"atan2(): wrong argument count!", s);
      return 0;
    }

    if (argType[1] != SVS_TYPE_FLT) {
      simpleError((uint8_t *)"atan2(): wrong type of argument! (1)", s);
      return 0;
    }

    if (argType[2] != SVS_TYPE_FLT) {
      simpleError((uint8_t *)"atan2(): wrong type of argument! (2)", s);
      return 0;
    }

    result->value = (varType)(atan2f(args[1].val_f, args[2].val_f));
    result->type = SVS_TYPE_FLT;
    return 1;
  }

  // flt = log([flt])
  if (callId == LOG) {
    if (count != 1) {
      simpleError((uint8_t *)"log(): wrong argument count!", s);
      return 1;
    }

    if (argType[1] != SVS_TYPE_FLT) {
      simpleError((uint8_t *)"log(): wrong type of argument!", s);
      return 1;
    }

    result->value = (varType)(logf(args[1].val_f));
    result->type = SVS_TYPE_FLT;
    return 1;
  }

  // flt = exp([flt])
  if (callId == EXP) {
    if (count != 1) {
      simpleError((uint8_t *)"exp(): wrong argument count!", s);
      return 0;
    }

    if (argType[1] != SVS_TYPE_FLT) {
      simpleError((uint8_t *)"exp(): wrong type of argument!", s);
      return 0;
    }

    result->value = (varType)(expf(args[1].val_f));
    result->type = SVS_TYPE_FLT;
    return 1;
  }

  // flt = pow([flt]base, [flt]exp)
  if (callId == POW) {
    if (count != 2) {
      simpleError((uint8_t *)"pow(): wrong argument count!", s);
      return 0;
    }

    if (argType[1] != SVS_TYPE_FLT || argType[2] != SVS_TYPE_FLT) {
      simpleError((uint8_t *)"pow(): wrong type of argument!", s);
      return 0;
    }

    result->value = (varType)(powf(args[1].val_f, args[2].val_f));
    result->type = SVS_TYPE_FLT;
    return 1;
  }

  // flt = pi()
  if (callId == PI) {
    if (count != 0) {
      simpleError((uint8_t *)"pi(): wrong argument count!", s);
      return 0;
    }

    result->value = (varType)((float)3.14159265);
    result->type = SVS_TYPE_FLT;
    return 1;
  }

  // flt = sqrt([flt])
  if (callId == SQRT) {
    if (count != 1) {
      simpleError((uint8_t *)"sqrt(): wrong argument count!", s);
      return 0;
    }

    if (argType[1] != SVS_TYPE_FLT) {
      simpleError((uint8_t *)"sqrt(): wrong type of argument!", s);
      return 0;
    }

    result->value = (varType)(sqrtf(args[1].val_f));
    result->type = SVS_TYPE_FLT;
    return 1;
  }
#else
  if (callId == SIN || callId == COS ||
      callId == TAN || callId == ATAN ||
      callId == LOG || callId == EXP ||
      callId == POW || callId == PI ||
      callId == SQRT
    ) {
    simpleError((uint8_t *)"execBuiltInCall(): SVS_USE_ADV_MATH is disabled in this build!", s);
    return 0;
  }
#endif

  // [num] ver()
  if (callId == VER) {
    /* // kept here for future use
    if (count != 0) {
      simpleError("ver(): wrong argument count!", s);
      return 0;
    }
    */

    result->value = (varType)((uint16_t)SVS_VERSION_NUM);
    result->type = SVS_TYPE_NUM;
    return 1;
  }

  // [num] rnd()
  if (callId == RND) {
    if (count != 0) {
      simpleError((uint8_t *)"rnd(): wrong argument count!", s);
      return 0;
    }
#ifdef SVS_RND_FUNCTION
    result->value = (varType)(SVS_RND_FUNCTION);
    result->type = SVS_TYPE_NUM;
    return 1;
#else
    simpleError((uint8_t *)"rnd(): Hook not presented!", s);
    return 0;
#endif
  }

  // call([str] function_name, arguments 0 - 8);
  if (callId == CALL) {
    if (count == 0) {
      simpleError((uint8_t *)"call(): wrong argument count!", s);
      return 0;
    }

    if (argType[1] != SVS_TYPE_STR) {
      simpleError((uint8_t *)"call(): function name must be passed as string!", s);
      return 0;
    }

    varRetVal pracVar;
    comExArgs pracArgs;
    comExArgs myArgs;
    uint16_t usedUpOld = 0;
    uint8_t retFlagOld = s->commRetFlag;

    commArgNull(&myArgs);

    for(uint16_t i = 2; i < count + 1; i++) {
      myArgs.arg[i - 1] = args[i];
      myArgs.argType[i - 1] = argType[i];
    }

    myArgs.usedup = count;

    // storing current arg field
    commArgCopy(&(s->commArgs), &pracArgs);
    commArgNull(&(s->commArgs));
    commArgCopy(&myArgs, &(s->commArgs));
    varRetValZero(&pracVar);

    //commExec
    commExec((uint8_t *)(s->stringField + args[1].val_str), s);

    if (errCheck(s)) {
      return 0;
    }

    // get results    
    if (s->commRetFlag) {
      result->value = s->commRetVal;
      result->type = s->commRetType;
    } else {
      result->value = (varType)((uint32_t)0);
      result->type = SVS_TYPE_NUM;
    }

    // put args back
    commArgCopy(&pracArgs, &s->commArgs);
    s->commArgs.usedup = usedUpOld;
    s->commRetFlag = retFlagOld;
    return 1;
  }

  // dbg([num] level)
  if (callId == DBG) {
    if (count != 1) {
      simpleError((uint8_t *)"dbg(): wrong argument count!", s);
      return 0;
    }

    if (argType[1] != SVS_TYPE_NUM) {
      simpleError((uint8_t *)"dbg(): wrong type of argument!", s);
      return 0;
    }

    if (args[1].val_s == 1) {
      s->globalDebug = args[1].val_s;
    }
    
    if (args[1].val_s == -1) {
      svsPrintUsedUp(s);
    }

    if (args[1].val_s == -2) {
      varTablePrint(s);
    }

    if (args[1].val_s == -3) {
      strTablePrint(s);
    }

    if (args[1].val_s == -4) {
      gcCheckField(s);
    }

    result->value = (varType)((uint32_t)0);
    result->type = SVS_TYPE_NUM;
    return 1;
  }

  // gcp([num] enable)
  if (callId == GCP) {
    if (count != 1) {
      simpleError((uint8_t *)"gcp(): wrong argument count!", s);
      return 0;
    }

    if (argType[1] != SVS_TYPE_NUM) {
      simpleError((uint8_t *)"gcp(): wrong type of argument!", s);
      return 0;
    }

    s->profilerEnabled = args[1].val_u;

    result->value = (varType)((uint32_t)0);
    result->type = SVS_TYPE_NUM;
    return 1;
  }

  // gc([num] num_to_free)
  if (callId == GC) {
    if (count != 1) {
      simpleError((uint8_t *)"gc(): wrong argument count!", s);
      return 0;
    }

    if (argType[1] != SVS_TYPE_NUM) {
      simpleError((uint8_t *)"gc(): wrong type of argument!", s);
      return 0;
    }

    garbageCollect(args[1].val_s,s);

    result->value = (varType)((uint32_t)0);
    result->type = SVS_TYPE_NUM;
    return 1;
  }

  simpleError((uint8_t *)"execBuiltInCall: Unknown builtin call!", s);
  return 0;
}
