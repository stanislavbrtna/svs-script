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

This is the main command execution logic.
*/

#include "svs_comm_exec.h"

uint8_t commExDebug;

void setCommExDebug (uint8_t level) {
  commExDebug = level;
}

void commExDMSG(char *text, uint16_t tokenId, svsVM *s) {
  if ((commExDebug == 1) || (s->globalDebug)) {
    printf("commExDMSG: %s \ntokenId: %u\n", text, tokenId);
  }
}

uint16_t exprSkip(uint16_t index, svsVM *s) {
  uint16_t count = 0;

  while(1) {
    if (getTokenType(index, s) == SVS_TOKEN_LBR) {
      count += 1 ;
    }

    if (getTokenType(index, s) == SVS_TOKEN_RBR) {
      if (count > 0) {
      count -= 1;
      } else {
        errSoft((uint8_t *)"commSkip: Bracket sanity error.", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)index, s);
        errSoftSetToken(index, s);
        return 0;
      }
      if (count == 0) {
        commExDMSG("commSkip expression skip end, (skipped)", index, s);
        break;
      }
    }
    index++;
  }

  return index; // ends on last bracket
}


// přeskakuje while smyčku, případně jednu z větví ifu
// used for skipping code after if/else/while, if the equation say so
uint16_t commSkip(uint16_t index, svsVM *s) {
  uint16_t x = 0;
  uint16_t count = 0;

  x = index;
  commExDMSG("commSkip start", index, s);

  // skip if/else
  if (getTokenType(x, s) == SVS_TOKEN_IF) {
    x = exprSkip(x, s);
    x++;
    x = commSkip(x, s);
    if (getTokenType(x + 1, s) != SVS_TOKEN_ELSE) {
      return x;
    } else {
      x++;
      x = commSkip(x, s);
      return x;
    }
  }

  // skip for and while
  if (getTokenType(x, s) == SVS_TOKEN_FOR ||
      getTokenType(x, s) == SVS_TOKEN_WHILE
      ) {
    x = exprSkip(x, s); // skip expression
    x++;
    x = commSkip(x, s); // skip loop body
    return x;
  }

  while(1) { //začátek bloku
    if (getTokenType(x,s) == SVS_TOKEN_LCBR) {
      count += 1 ;
    }

    if (getTokenType(x ,s) == SVS_TOKEN_RCBR) {
      if (count > 0) {
        count -= 1;
      } else {
        errSoft((uint8_t *)"commSkip: Curly bracket sanity error. Maybe missing \";\"?" ,s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)x ,s);
        errSoftSetToken(x, s);
        return 0;
      }
      if (count == 0) {
        commExDMSG("commSkip end (skipped BLOCK(S))", x, s);
        break;
      }
    }

    //řeší situaci kdy za příkazem není závorka, funguje ok, pokud je na konci řádku středník
    if ((getTokenType(x, s) == 9) && (count == 0)) {
      commExDMSG("commSkip end (skipped LINE)", x, s);
      break;
      //blok má ve zvyku obdržet token posledního vykonaného příkazu, inkrement provádí sám.
    }

    x++; //poskočí na další token
  }

  if ((x >= index) && (x <= s->tokenMax)) {
    return x;
  } else {
    errSoft((uint8_t *)"commSkip: Skip sanity error.", s);
    errSoftSetParam((uint8_t *)"after TokenId", (varType)index, s);
    errSoftSetToken(index, s);
    return 0;
  }
}


// main execution function, takes token index, executes it and returns the next token
uint16_t commExecLoop(uint16_t index, svsVM *s) {
  uint16_t currToken = 0;
  uint16_t x = 0;
  uint16_t prac = 0;
  uint16_t backBr = 0;
  // zde je uložená pozice ve vartable, pro použití lokálních proměnných
  // varTableSP - variable table stack pointer, used with local variables
  uint16_t varTableSP = 0;
  uint8_t newToken;
  varRetVal varPrac;

  varRetValZero(&varPrac);

  currToken = index;

  errSoftSetToken(currToken, s);

  if (s->handbrake == 1) {
    return 0;
  }

  newToken = getTokenType(currToken, s);

  if(newToken == SVS_TOKEN_ENDPROG) {
    return 0;
  }

  commExDMSG("commExecLoop", currToken, s);

  // garbage collection is called each exec loop, but it might do nothing
  // its behaviour depends on size of the empty memory space
  garbageCollect(0, s);

  // function call
  // volání funkce
  if (newToken == SVS_TOKEN_CALL) {
    commExDMSG("commExecLoop: Function Call", currToken, s);
    commExDMSG((char *)s->stringField + getTokenData(currToken, s).val_u, currToken, s);
    return commParseCall(currToken, s);
  }

  // local statement
  if (newToken == SVS_TOKEN_LOCAL) {
    commExDMSG("commExecLoop: LOCAL statement", currToken, s);
    currToken++;

    if (getTokenType(currToken, s) != SVS_TOKEN_VAR) { // expecting VAR
      errSoft((uint8_t *)"commEx: Syntax error next to LOCAL: Expected VAR after LOCAL.", s);
      errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
      errSoftSetToken(currToken, s);
      return 0;
    } else {
      varAddLocal(getTokenData(currToken, s), s); //přidáme lokální promněnnou / adds a local variable

      if (errCheck(s)) { //kontrola a obsluha možné chyby v předchozím volání / error check
        return 0;
      }
      currToken++;

      if (getTokenType(currToken, s) != SVS_TOKEN_SCOL) { //zkontrolujeme středník / semicolon check
        errSoft((uint8_t *)"commEx: Syntax error, missing ; .", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
        errSoftSetToken(currToken, s);
        return 0;
      }
    }
    return currToken;
  }

  // array statement
  if (newToken == SVS_TOKEN_ARRAY) {
    varType tmp;
    varType id;
    varType len;
    commExDMSG("commExecLoop: ARRAY statement", currToken, s);
    currToken++;

    if (getTokenType(currToken, s) != SVS_TOKEN_VAR) { // expecting VAR
      errSoft((uint8_t *)"commEx: Syntax error next to ARRAY: Expected VAR after ARRAY.", s);
      errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
      errSoftSetToken(currToken, s);
      return 0;
    } else {
      id = getTokenData(currToken, s);

      currToken++;

      if (getTokenType(currToken, s) != SVS_TOKEN_LSQB) {
        errSoft((uint8_t *)"commEx: Syntax error next to ARRAY: Missing [.", s);
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

      len = varPrac.value;

      tmp = newArray(id, (uint16_t) len.val_s, s);
      if (errCheck(s)) {
        return 0;
      }

      varSetVal(id, tmp, s);
      if (errCheck(s)) {
        return 0;
      }

      varSetType(id, SVS_TYPE_ARR, s);
      if (errCheck(s)) {
        return 0;
      }

      if (getTokenType(currToken, s) != SVS_TOKEN_RSQB) {
        errSoft((uint8_t *)"commEx: Syntax error, missing ].", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
        errSoftSetToken(currToken, s);
        return 0;
      }

      currToken++;

      if (getTokenType(currToken, s) != SVS_TOKEN_SCOL) { //zkontrolujeme středník / semicolon check
        errSoft((uint8_t *)"commEx: Syntax error, missing ; .", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
        errSoftSetToken(currToken, s);
        return 0;
      }
    }

    return currToken;
  }

  if (newToken == SVS_TOKEN_SYS) { //sys statement

    commExDMSG("commExecLoop: SYS statement", currToken, s);
    sysExec(currToken, &varPrac, s);
    if (errCheck(s)) {
      return 0;
    }
    currToken = varPrac.tokenId;
    return currToken;
  }

  // built-in function
  if (newToken == SVS_TOKEN_FUNCTION_BUILTIN) {

    commExDMSG("commExecLoop: BUILT-IN FUNCTION statement", currToken, s);
    processBuiltInCall(currToken, &varPrac, s);
    if (errCheck(s)) {
      return 0;
    }
    currToken = varPrac.tokenId;
    return currToken;
  }

  // equals statement
  if (newToken == SVS_TOKEN_VAR) {
    //pokud narazíme na typ VAR / if we found a variable

    x = currToken; //uložíme token indexu promněnné / we store index of the variable
    currToken++;
    if (getTokenType(currToken,s) == SVS_TOKEN_ASSIGN) { // =
      commExDMSG("commExecLoop: = statement", currToken, s);
      if (varGetType(getTokenData(x, s), s) == SVS_TYPE_ARR) {
        errSoft((uint8_t *)"commEx: Assign on array is not supported.", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
        errSoftSetToken(currToken, s);
        return 0;
      }

      currToken++;
      //printf("var set val id: %u, value: %u\n", tokenData[x], exprExec(currToken));
      //printf("math exec id: %u\n", currToken);

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
      commExDMSG("commExecLoop: ++ statement", currToken, s);
      currToken++;
      if (getTokenType(currToken, s) == SVS_TOKEN_ADD) {
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
      }else{
        errSoft((uint8_t *)"commEx: Syntax error next to ++ (missing \"+\").", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
        errSoftSetToken(currToken, s);
        return 0;
      }

    } else if(getTokenType(currToken, s) == SVS_TOKEN_SUBT) { // --
      commExDMSG("commExecLoop: -- statement", currToken, s);
      currToken++;
      if (getTokenType(currToken, s) == SVS_TOKEN_SUBT) {
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
      } else {
        errSoft((uint8_t *)"commEx: Syntax error next to -- (missing \"-\").", s);
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

    if (getTokenType(currToken,s) != 9) { //zkontrolujeme středník / semicolon check
        errSoft((uint8_t *)"commEx: Syntax error, missing ; .", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
        errSoftSetToken(currToken, s);
        return 0;
      }

    return currToken;
  }

  if (newToken == SVS_TOKEN_RETURN) { //return

    if (s->commRetFlag == 0) {
      commExDMSG("commExecLoop: RETURN statement", currToken, s);
      currToken++;
      exprExec(currToken, &varPrac, s);
      if (errCheck(s)) {
        return 0;
      }
      s->commRetVal = varPrac.value;
      s->commRetType = varPrac.type;
      s->commRetFlag = 1;
      return currToken - 1;
    }
    return currToken;
  }

  // začátek bloku / start of block
  if (newToken == SVS_TOKEN_LCBR) {

    commExDMSG("commExecLoop: start of block ({)", currToken, s);
    currToken++; // skipping "{"
    varTableSP = s->varTableLen; // storing current variable stack
    while (getTokenType(currToken, s) != SVS_TOKEN_RCBR) { // loop until "}"
      /*
      Break funguje tak, že když se na něj narazí, tak se snaží commExecLoop
      returnovat break token tak dlouho, až se narazí na while co ho zavolal
      a ten se breakne.
      */

      if (getTokenType(currToken, s) != SVS_TOKEN_BREAK) { //break? ne
        //vykonáváme příkaz dokud nenarazíme na break a nebo end of block
        currToken = commExecLoop(currToken, s);

        if (errCheck(s)) {
          return 0;
        }

        if (s->handbrake == 1) {
          return 0;
        }

        if (getTokenType(currToken, s) == SVS_TOKEN_BREAK) { //break
          s->varTableLen = varTableSP; //při opuštění bloku vrátíme stack
          commExDMSG("commExecLoop: block: break occured inside block!", currToken, s);
          return currToken;
        }

        if (getTokenType(currToken, s) == SVS_TOKEN_RETURN) { //return
          s->varTableLen = varTableSP;
          commExDMSG("commExecLoop: block: return inside block.", currToken, s);
          return currToken;
        }

        currToken++;
      } else { // break
        s->varTableLen = varTableSP;
        commExDMSG("commExecLoop: start of block: break occured!", currToken, s);
        return currToken;
      }
    }
    s->varTableLen = varTableSP;
    return currToken;
  }

  if (newToken == SVS_TOKEN_IF) { //if / else
    commExDMSG("commExecLoop: if statement", currToken, s);
    currToken++;
    if (getTokenType(currToken, s) != 5) { // detekce závorky
      errSoft((uint8_t *)"commExecLoop: Unknown statement after if", s);
      errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
      errSoftSetToken(currToken, s);
      return 0;
    } else {
      currToken++;
      exprExec(currToken, &varPrac, s);
      if (errCheck(s)) {
        return 0;
      }
      x = varPrac.value.val_s; //výsledek ifu
      currToken = varPrac.tokenId;
      if (getTokenType(currToken,s) != 6) {
        errSoft((uint8_t *)"commEx: Syntax error in if statement, missing \")\"", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
        errSoftSetToken(currToken, s);
        return 0;
      }
      currToken++;
      if (x) { //podmínka splněna
        commExDMSG("commExecLoop: if statement: expression TRUE", currToken, s);
        //curr token teď míří přímo na novej příkaz
        if (getTokenType(currToken, s) != 15) {
          currToken = commExecLoop(currToken, s);
          if (errCheck(s)) {
            return 0;
          }
          if (getTokenType(currToken, s) == 16) {
            //printf("if: return occured token: %u \n", currToken);
            return currToken;
          }
        } else {
          //printf("if: break occured token: %u \n", currToken);
          return currToken;
        }
      } else {
        //currToken++; //curr token teď míří přímo na novej příkaz
        //printf("comSkip míří na token %u \n", currToken);
        currToken = commSkip(currToken, s);
        if (errCheck(s)) {
          return 0;
        }
        commExDMSG("commExecLoop: if statement: expression FALSE, code skipped", currToken, s);
      }
      //pokud existuje else větev
      if(getTokenType(currToken + 1, s) == 13) {
        commExDMSG("commExecLoop: if statement: else detected", currToken, s);
        currToken += 1;
        if (!x){ //podmínka nesplněna
          currToken++;
          commExDMSG("commExecLoop: if statement: else: executing", currToken, s);
          if (getTokenType(currToken,s) != 15) {
            currToken = commExecLoop(currToken, s);
            if (errCheck(s)) {
              return 0;
            }
            if (getTokenType(currToken, s) == 16) {
              //printf("if: return occured token: %u \n", currToken);
              return currToken;
            }
          } else {
            //printf("if: break occured token: %u \n", currToken);
            return currToken;
          }
        } else {
          currToken++; //curr token teď míří přímo na novej příkaz
          currToken = commSkip(currToken, s);
          if (errCheck(s)) {
            return 0;
          }
          commExDMSG("commExecLoop: if statement: else: skipped", currToken, s);
        }
      }
    }
    commExDMSG("commExecLoop: if statement: end of if", currToken, s);
    return currToken;
  }

  if (newToken == 34) { //for
    uint16_t exprPrac = 0;
    uint16_t endPrac = 0;
    uint16_t loopPrac = 0;
    uint32_t x = 0;

    commExDMSG("commExecLoop: for statement", currToken, s);
    currToken++;

    //struktura: for(init expr;check expr;endloop expr)

    if (getTokenType(currToken,s) != 5) { // detekce závorky
      errSoft((uint8_t *)"commEx: Unknown statement after while, missing \"(\"", s);
      errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
      errSoftSetToken(currToken, s);
      return 0;
    } else {
      currToken++;
      currToken = commExecLoop(currToken, s);
      if (errCheck(s)) {
        return 0;
      }
      if (getTokenType(currToken, s) != 9) { //zkontrolujeme středník
        errSoft((uint8_t *)"commExecLoop: for: Syntax error, missing ; .", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
        errSoftSetToken(currToken, s);
        return 0;
      }

      currToken++;
      //kód podmínky
      exprPrac = currToken;
      while(1) {
        currToken = exprPrac;
        exprExec(currToken, &varPrac, s);
        if (errCheck(s)) {
          return 0;
        }
        x = varPrac.value.val_s; //výsledek podmínky
        currToken = varPrac.tokenId;
        endPrac = currToken + 1;

        //skip end prac
        currToken = commSkip(currToken + 1, s);
        if(x == 0) { //skip
          commExDMSG("commExecLoop: for: expression FALSE, skipping", currToken, s);
          //skip loop body
          currToken++;
          currToken = commSkip(currToken + 1, s);
          if (errCheck(s)) {
            return 0;
          }
          //break out
          break;
        } else {
          commExDMSG("commExecLoop: for: expression TRUE", currToken, s);
        }
        if (getTokenType(currToken, s) == 9) {
          currToken++; //přeskočíme středník na konci výrazu
        }

        if (getTokenType(currToken, s) != 6) {
          errSoft((uint8_t *)"commEx: Syntax error in for statement, missing \")\" or maybe \";\"", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)exprPrac, s);
          //jako parametr stavíme exprPrac, kdyby current token mířil po skipu někam do háje
          errSoftSetToken(exprPrac, s);
          return 0;
        }
        currToken++;

        loopPrac = currToken;
        //tělo smyčky
        currToken = commExecLoop(loopPrac, s);
        if (errCheck(s)) {
          return 0;
        }

        if (s->handbrake == 1) {
          return 0;
        }

        //Detekce breaku
        if (getTokenType(currToken, s) == 15) {
          //v případě breaku nahrajeme do current tokenu start smyčky a pak jí přeskočíme comm skipem
          currToken = loopPrac;
          currToken = commSkip(currToken, s);
          if (errCheck(s)) {
            return 0;
          }
          commExDMSG("commExecLoop: for: break occured, skipping", currToken, s);
          break;
        }

        if (getTokenType(currToken, s) == 16) {
          commExDMSG("commExecLoop: while: return occured, returning", currToken, s);
          return currToken;
        }

        //increment/decrement/whatever
        commExecLoop(endPrac, s);
        if (errCheck(s)) {
          return 0;
        }
      }
    }
    return currToken;
  }

  if (newToken == 14) { //while
    commExDMSG("commExecLoop: while statement", currToken, s);
    currToken++;
    if (getTokenType(currToken, s) != 5) { // detekce závorky
      errSoft((uint8_t *)"commEx: Unknown statement after while, missing \"(\"", s);
      errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
      errSoftSetToken(currToken, s);
      return 0;
    } else {
      currToken++;
      prac = currToken;
      while(1) {
        currToken = prac;
        exprExec(currToken, &varPrac, s);
        if (errCheck(s)) {
          return 0;
        }
        if (s->handbrake == 1) {
          return 0;
        }
        x = varPrac.value.val_s; //výsledek podmínky
        currToken = varPrac.tokenId;
        backBr = currToken;
        if (getTokenType(currToken, s) != 6) {
          errSoft((uint8_t *)"commEx: Syntax error in while statement, missing \")\"", s);
          errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
          errSoftSetToken(currToken, s);
          return 0;
        }
        if (x) { //podmínka splněna
          commExDMSG("commExecLoop: while: expression TRUE", currToken, s);
          currToken++; //curr token teď míří přímo na novej příkaz
          currToken = commExecLoop(currToken, s);
          if (errCheck(s)) {
            return 0;
          }
          //Detekce breaku
          if (getTokenType(currToken, s) == 15) {
            //v případě breaku nahrajeme do current tokenu start smyčky a pak jí přeskočíme comm skipem
            currToken = backBr + 1;
            currToken = commSkip(currToken, s);
            if (errCheck(s)) {
              return 0;
            }
            commExDMSG("commExecLoop: while: break occured, skipping", currToken, s);
            break;
          }

          if (getTokenType(currToken, s) == 16) {
            commExDMSG("commExecLoop: while: return occured, returning", currToken, s);
            return currToken;
          }

        } else {
          currToken++; //curr token teď míří přímo na novej příkaz
          currToken = commSkip(currToken, s);
          if (errCheck(s)) {
            return 0;
          }
          commExDMSG("commExecLoop: while: expression FALSE, skipping", currToken, s);
          break;
        }
      }
    }
    commExDMSG("commExecLoop: while: end of while", currToken, s);
    return currToken;
  }

  errSoft((uint8_t *)"commEx: Unexpected command!", s);
  errSoftSetParam((uint8_t *)"TokenType", (varType)(uint16_t)getTokenType(currToken, s), s);
  errSoftSetToken(currToken, s);
  return 0;
}

uint16_t commExecById(uint16_t id, svsVM *s) {
  //třeba nastavit startovací token funkce
  //printf("Jumping into: %s starting at token %u \n",funcTable[id].fString, funcTable[id].tokenId );
  s->commRetVal.val_u = 0;
  s->commRetType = 0;
  s->commRetFlag = 0;
  commExecLoop(s->funcTable[id].tokenId, s);
  return 0;
}

uint16_t commExec(uint8_t * name, svsVM *s) {
  if (functionExists(name, s)) {
    return commExecById(functionGetId(name, s), s);
  } else {
    errSoft((uint8_t *)"commExecById: Function does not exist.", s);
    return 0;
  }
}

void commArgNull(comExArgs *target) {
  uint8_t x;

  for(x = 0; x < FUNCTION_ARGS_MAX; x++) {
    target->arg[x] = (varType)((uint16_t)0);
    target->argType[x] = 0;
  }

  target->usedup = 0;
}

void commArgCopy(comExArgs* source, comExArgs* target){
  uint8_t x;

  for(x = 0; x < FUNCTION_ARGS_MAX; x++) {
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

