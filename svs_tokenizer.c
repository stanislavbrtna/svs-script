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
*/

#include "svs_tokenizer.h"

#define INCREMENT_STATIC_STRINGS s->stringConstMax = s->stringFieldLen;

// enable tokenizer debug messages
static uint8_t tokenizerDebug = 0;

static uint8_t tokenizerInit;
static uint8_t tokenizerFerrFlag;
static uint8_t tokenizerStringFlag;

static uint8_t prevChar;
static uint8_t prevChar2;

//exact debug helpers
static uint8_t tokenizer_exact_debug;
static uint16_t tokenizer_exact_token;
static uint16_t tokenizer_exact_char;
static uint16_t tokenizer_exact_line;

uint8_t debug_buffer[101];
uint8_t debug_buffer_pos;

void setTokenizerDebug(uint8_t level) {
  tokenizerDebug = level;
}

void setTokenizerFerrFlag() {
  tokenizerFerrFlag = 1;
}

uint8_t tokenGetch();

void tokenizerReset() {
  tokenizerInit = 0;
  tokenizer_exact_debug = 0;
  tokenizerStringFlag = 0;
  prevChar = 0;
  prevChar2 = 0;
}

void token_line_print() {
  uint16_t x;
  uint8_t c = 0;
  debug_buffer[100] = 0;
  printf("near line %u :\n %s", tokenizer_exact_line, debug_buffer);

  x = 0;
  c = tokenGetch();
  while ((c != '\n') && (c != 0) && (x < 100)) {
    printf("%c", c);
    c = tokenGetch();
    x++;
  }
}

// this will run tokenizer on svs file and print line with given token.
void tokenzer_print_token_line(uint16_t token, svsVM *s) {
  uint16_t x;

  for(x = 0; x < 100; x++) {
    debug_buffer[x] = 0;
  }
  tokenizer_exact_token = token;

  tokenizer_exact_char = 0;
  tokenizer_exact_line = 1;
  debug_buffer_pos = 0;

  printf("File: %s\n", s->fName);

  tokenGetchSetup(s->fName, 0);

  tokenizerReset();
  tokenizer_exact_debug = 1;

  // reset some tokenizer variables
  s->funcTableLen = 0;
  s->varTableLen = 0;
  s->syscallTableLen = 0;
  s->stringFieldLen = 0;
  s->errToken = 0;
  s->err = 0;

  tokenParse(s);
  printf("Token %u, ", token);
  token_line_print();
}

void tokenizerErrorPrint(uint8_t *str) {
  printf("Tokenizer Error: %s\n", str);
  tokenizer_exact_line++;
  token_line_print();
  printf("\n");
}

void tokenizerErrorPrintNL(uint8_t *str) {
  printf("Tokenizer Error: %s\n", str);
}

void tokenDMSG(char *text, uint16_t tokenId, VARTYPE data, uint8_t type, uint16_t textPos) {
#ifndef DBG_DISABLED
  if(tokenizerDebug == 1) {
    printf("TokenDMSG: %s \ntokenId: %u\n", text, tokenId);
  }
  if(tokenizerDebug == 2) {
    printf("%u:%u:%i:%u (id:type:data:tP)\n", tokenId, type, (int)data.val_s, textPos);
  }
#endif
}

uint8_t tokenGetchDbg() {
  uint8_t x;
  uint8_t c = 0;

  c = tokenGetch();

  if (c == '\n') {
    tokenizer_exact_line++;
    for(x = 0; x < 100; x++) {
      debug_buffer[x] = 0;
    }
    debug_buffer_pos = 0;
  } else {
    if (debug_buffer_pos < 99) {
      debug_buffer[debug_buffer_pos] = c;
      debug_buffer_pos++;
    } else {
      debug_buffer_pos = 0;
    }
  }

  //printf("token getch dbg: %c (%u)\n", c, c);
  return c;
}


uint8_t tokenPreprocessor() {
  uint8_t c = 0;

  c = tokenGetchDbg();

  if ((c == '\"' && prevChar!='\\') || (prevChar2 == '\\' && prevChar == '\\' && c == '\"')) {
    tokenizerStringFlag = 1 - tokenizerStringFlag;
  }

  if (!tokenizerStringFlag) {
    if (c == '#') {
      c = tokenGetchDbg();
      if (c == '*') {
        // multiline comment
        while (1) {
          c = tokenGetchDbg();
          if (c == '*') {
            c = tokenGetchDbg();
            if (c == '#') {
              c = tokenGetchDbg();
              break;
            }
          }
        }
      } else {
        // single line comment
        while (c != '\n') {
          c = tokenGetchDbg();
        }
      }
    }
  }

  prevChar2 = prevChar;
  prevChar = c;
  return c;
}

uint8_t tokenInput(uint16_t *index, uint8_t inc) {
  static uint8_t peek;
  static uint8_t strBuff[SVS_TOK_STR_BUFFLEN + 1];
  static uint16_t vTextPos;
  uint16_t x;

  strBuff[SVS_TOK_STR_BUFFLEN] = 0; // for easy debug print

  if (tokenizerInit == 0) {
    tokenizerFerrFlag = 0;
    vTextPos = 0;
    tokenizerInit = 1;
    strBuff[0] = tokenPreprocessor();
  }

  if (inc == 1) {
    // TODO: Remove all the debug prints from token input and get keyword ext
    // once it is verified to be working
    *index += 1;

    if (vTextPos > *index) {
      peek = 1;
    }

    if (peek == 0) {
      vTextPos++;
      for(x = SVS_TOK_STR_BUFFLEN - 1; x > 0; x--) {
        strBuff[x] = strBuff[x - 1];
      }
      strBuff[0] = tokenPreprocessor();
    }
    if (peek == 1) {
      peek = 0;
    }
    //printf("r1: wat: %s\n", strBuff);
    return 0;
  }

  if (*index <= vTextPos) {
    //printf("pos: %u index: %u\n", vTextPos, *index);
    //printf("r2: c: %c (%u) wat:\n %s\n", strBuff[vTextPos - *index],strBuff[vTextPos - *index], strBuff);
    return strBuff[vTextPos - *index];
  } else {
    //peek
    vTextPos++;
    for(x = SVS_TOK_STR_BUFFLEN - 1; x > 0; x--) {
      strBuff[x] = strBuff[x - 1];
    }
    strBuff[0] = tokenPreprocessor();
    peek = 1;
    //printf("r3: wat: %s\n", strBuff);
    return strBuff[*index - vTextPos];
  }

  return 0;
}

#ifdef WEBTARGET
volatile uint8_t testoid;

void doNotRemoveThese(uint16_t callLen, uint8_t * call)  __attribute__ ((optimize("0")));

void doNotRemoveThese(uint16_t callLen, uint8_t * call) {
  printf("syscall id: %u, name: %s \n",callLen, call);
  testoid+= (uint8_t) callLen + (uint8_t) call;
}
#endif

/*
// stub used occasionaly for debug, TODO: use compiler flags
uint8_t tokenInput(uint16_t index, uint8_t inc){
  uint8_t c=0;
  c = tokenInputX(index, inc);

  printf("token input init:%u (index: %u, inc: %u): %c (%u)\n",tokenizerInit ,index, inc, c, c);
  return c;
}
*/

uint8_t tokenEofCheck(int16_t brCount1, int16_t  brCount2) {
  if ((brCount2 == 0) && (brCount1 == 0)) {
    return 0;
  } else {
    if (brCount1 < 0) {
      tokenizerErrorPrintNL((uint8_t *)"Bracket chceck error! Missing \"(\"");
      return 1;
    }

    if (brCount1 > 0) {
      tokenizerErrorPrintNL((uint8_t *)"Bracket chceck error! Missing \")\"");
      return 1;
    }

    if (brCount2 < 0) {
      tokenizerErrorPrintNL((uint8_t *)"Bracket chceck error! Missing \"{\"");
      return 1;
    }

    if (brCount2 > 0) {
      tokenizerErrorPrintNL((uint8_t *)"Bracket chceck error! Missing \"}\"");
      return 1;
    }
  }
  return 1;
}

// special case for getting sys.something.somethingelse.fn
uint8_t getKeywordExt(uint8_t *out_buffer, uint16_t *posText) {
  uint16_t pracStrInd = 0;
  uint8_t specChar = 0;
  uint8_t buffer[NAME_LENGTH*2];

  // first char
  buffer[pracStrInd] = tokenInput(posText, 0);

  // increment counters
  tokenInput(posText, 1);
  pracStrInd++;

  while(1) {
    if (pracStrInd < (NAME_LENGTH*2) - 1) {
      if (isRegChar(tokenInput(posText, 0)) || isNumber(tokenInput(posText, 0))
          || (tokenInput(posText, 0) == '.')
      ) {
        // mark spec char
        if (tokenInput(posText, 0) == '.') {
          specChar = 1;
        }
        buffer[pracStrInd] = tokenInput(posText, 0);
      } else {
        // if not regchar or num, break
        break;
      }
      tokenInput(posText, 1);
      pracStrInd++;
      // zero the next one
      buffer[pracStrInd] = 0;
    } else {
      // Copy the buffers
      for(uint16_t x = 0; x < NAME_LENGTH; x++){
        out_buffer[x] = buffer[x];
      }
      out_buffer[NAME_LENGTH - 1] = 0;
      return 1;
    }
  }

  if (specChar == 0) {
    for(uint16_t x = 0; x < NAME_LENGTH; x++){
      out_buffer[x] = buffer[x];
      if (buffer[x] == 0) {
        break;
      }
    }
    return 0;
  }

  //printf("pt: %u, len: %u, buffer: %s\n", *posText, pracStrInd, buffer);
  // move back to the spec char
  for (uint16_t i = pracStrInd; i > 0; i--) {
    if (buffer[i] == '.') {
      buffer[i] = 0;
      *posText -= pracStrInd - i;
      //printf("i:%u\n", i);
      break;
    }
  }

  for(uint16_t x = 0; x < NAME_LENGTH; x++) {
    out_buffer[x] = buffer[x];
    if (buffer[x] == 0) {
      break;
    }
  }
  //printf("pt: %u\n", *posText);
  //printf("buffer: %s, index on: %c (%u)\n", buffer, tokenInput(posText, 0), tokenInput(posText, 0));

  return 0;
}

uint8_t getKeyword(uint8_t *buffer, uint16_t *posText) {
  uint16_t pracStrInd = 0;

  buffer[pracStrInd] = tokenInput(posText, 0);
  tokenInput(posText, 1);
  pracStrInd++;
  while(1) {
    if (pracStrInd < NAME_LENGTH) {
      if (isRegChar(tokenInput(posText, 0)) || isNumber(tokenInput(posText, 0))) {
        buffer[pracStrInd] = tokenInput(posText, 0);
      } else {
        break;
      }
      tokenInput(posText, 1);
      pracStrInd++;
      buffer[pracStrInd] = 0;
    } else {
      buffer[NAME_LENGTH - 1] = 0;
      return 1;
    }
  }
  return 0;
}

uint8_t tokenParse(svsVM *s) {
  uint16_t posText = 0;
  uint16_t posToken = 0;
  uint8_t  pracName[NAME_LENGTH];
  uint8_t  pracName2[NAME_LENGTH];
  uint8_t  Lock = 1;
  uint16_t x;
  uint16_t currLine = 0;
  uint16_t tokenMax = 0;
  VARTYPE numPrac;
  VARTYPE numPracF;
  int16_t brCount1 = 0; //počítá jednoduché závorky ( => +1 a ) => -1
  int16_t brCount2 = 0;  //počítá složené závorky {}
  uint8_t floatFound;
  uint16_t float_dp = 0;
  uint8_t esc_c_prac = 0;
  uint16_t tokenizer_prev_text;

  tokenizer_prev_text = 1;

  numPrac.val_s = 0;
  numPracF.val_s = 0;

  tokenMax = getTokenMax(s);

  // token input error testing
  tokenInput(&posText, 0);
  if (tokenizerFerrFlag) {
    tokenizerErrorPrintNL((uint8_t *)"tokenParse:Error in opening file.");
    return 1;
  }

  while (posToken < tokenMax) {

    if (posText == tokenizer_prev_text) {
      tokenizerErrorPrint((uint8_t *)"Tokenizer got stuck!");
      return 1;
    } else {
      tokenizer_prev_text = posText;
    }

    // nulování pracovních textových proměnných
    // putting zero in temp variables
    for(x = 0; x < NAME_LENGTH; x++) {
      pracName[x] = 0;
      pracName2[x] = 0;
    }

    Lock = 1;

    // number, float, hex parsing, negative numbers are ignored here,
    // the minus sign is treated as expression during execution
    if (isNumber(tokenInput(&posText, 0))) {
      setTokenType(posToken, SVS_TOKEN_CONST_NUM, s);
      numPrac.val_s  = 0;
      numPracF.val_s = 0;
      floatFound     = 0;

      numPrac.val_s = tokenInput(&posText, 0) - 48;
      tokenInput(&posText, 1);

      // handling 0.x
      if (tokenInput(&posText, 0) == '.') {
      #ifdef USE_FLOAT
        floatFound = 1;
        float_dp   = 1;
        tokenInput(&posText, 1);
        numPrac.val_f = (float) numPrac.val_s;
      #else
        tokenizerErrorPrint("tokenParse: Float not supported in this build!");
        return 1;
      #endif
      }
      // hex number input
      if(tokenInput(&posText, 0) == 'x') {
        numPrac.val_s = 0;
        tokenInput(&posText, 1);
        while (((tokenInput(&posText, 0) >= '0') && (tokenInput(&posText, 0) <= '9'))
               || ((tokenInput(&posText, 0) >= 'a') && (tokenInput(&posText, 0) <= 'f'))
               || ((tokenInput(&posText, 0) >= 'A') && (tokenInput(&posText, 0) <= 'F'))
        ) {

          if((tokenInput(&posText, 0) >= '0') && (tokenInput(&posText, 0) <= '9')) {
            numPrac.val_s *= 16;
            numPrac.val_s += tokenInput(&posText, 0) - '0';
            tokenInput(&posText, 1);
          }

          if((tokenInput(&posText, 0) >= 'a') && (tokenInput(&posText, 0) <= 'f')) {
            numPrac.val_s *= 16;
            numPrac.val_s += tokenInput(&posText, 0) - 'a' + 10;
            tokenInput(&posText, 1);
          }

          if((tokenInput(&posText, 0) >= 'A') && (tokenInput(&posText, 0) <= 'F')) {
            numPrac.val_s *= 16;
            numPrac.val_s += tokenInput(&posText, 0) - 'A' + 10;
            tokenInput(&posText, 1);
          }
        }
        setTokenData(posToken, numPrac, s);
        tokenDMSG("Token set, type NUM",
                  posToken,
                  getTokenData(posToken, s),
                  getTokenType(posToken, s),
                  posText);
      } else {
        while (isNumber(tokenInput(&posText, 0))) {
          if(floatFound == 0) {
            numPrac.val_s *= 10;
            numPrac.val_s += tokenInput(&posText, 0) - 48;
            tokenInput(&posText, 1);
            if (tokenInput(&posText, 0) == '.') {
            #ifdef USE_FLOAT
              floatFound = 1;
              float_dp   = 1;
              tokenInput(&posText, 1);
              numPrac.val_f = (float)numPrac.val_s;
            #else
              tokenizerErrorPrint("tokenParse: Float not supported in this build!");
              return 1;
            #endif
            }
          } else {
            #ifdef USE_FLOAT
            numPracF.val_f += ((float)(tokenInput(&posText, 0) - 48) / (float)((exp_helper(10, float_dp))));
            float_dp++;
            tokenInput(&posText, 1);
            #endif
          }
        }
        if (floatFound == 1) {
          #ifdef USE_FLOAT
          setTokenType(posToken, SVS_TOKEN_CONST_FLOAT, s);
          numPrac.val_f += numPracF.val_f;
          setTokenData(posToken, numPrac, s);
          tokenDMSG("Token set, type FLT",
                    posToken,
                    getTokenData(posToken, s),
                    getTokenType(posToken, s),
                    posText);
          //printf("flt: %f \n",getTokenData(posToken,s).val_f);
          #endif
        } else {
          setTokenData(posToken, numPrac, s);
          tokenDMSG("Token set, type NUM",
                    posToken,
                    getTokenData(posToken, s),
                    getTokenType(posToken, s),
                    posText);
        }
      }
      posToken++;
    }

    // space (ignored)
    // mezeru ignorujeme
    if (tokenInput(&posText, 0) == ' ') {
      tokenInput(&posText, 1);
    }

    // tab  (ignored)
    // tab ignorujeme
    if (tokenInput(&posText, 0) == 9) {
      tokenInput(&posText, 1);
    }

    // newline: current line incremented, otherwise ignored
    if (tokenInput(&posText, 0) == '\n') {
      if(tokenizerDebug){
        printf("\n->Now proccesing line %u:\n", tokenizer_exact_line);
      }
      currLine++;
      tokenInput(&posText, 1);
    }

    // +
    if (tokenInput(&posText, 0) == '+') {
      setTokenType(posToken, SVS_TOKEN_ADD, s);
      tokenInput(&posText, 1);
      tokenDMSG("Token set, type +",
                posToken,
                getTokenData(posToken, s),
                getTokenType(posToken, s),
                posText);
      posToken++;
    }

    // -
    if (tokenInput(&posText, 0) == '-') {
      setTokenType(posToken, SVS_TOKEN_SUBT, s);
      tokenInput(&posText, 1);
      tokenDMSG("Token set, type -",
                posToken,
                getTokenData(posToken, s),
                getTokenType(posToken, s),
                posText);
      posToken++;
    }

    // *
    if (tokenInput(&posText, 0) == '*') {
      setTokenType(posToken, SVS_TOKEN_MUL, s);
      tokenInput(&posText, 1);
      tokenDMSG("Token set, type *",
                posToken,
                getTokenData(posToken, s),
                getTokenType(posToken, s),
                posText);
      posToken++;
    }

    // /
    if (tokenInput(&posText, 0) == '/') {
      setTokenType(posToken, SVS_TOKEN_DIV, s);
      tokenInput(&posText, 1);
      tokenDMSG("Token set, type /",
                posToken,
                getTokenData(posToken, s),
                getTokenType(posToken,s),
                posText);
      posToken++;
    }

    // %
    if (tokenInput(&posText, 0) == '%') {
      setTokenType(posToken, SVS_TOKEN_MOD, s);
      tokenInput(&posText, 1);
      tokenDMSG("Token set, type %",
                posToken,
                getTokenData(posToken, s),
                getTokenType(posToken, s),
                posText);
      posToken++;
    }

    // (
    if (tokenInput(&posText, 0) == '(') {
      setTokenType(posToken, SVS_TOKEN_LBR, s);
      tokenInput(&posText, 1);
      tokenDMSG("Token set, type (",
                posToken,
                getTokenData(posToken, s),
                getTokenType(posToken, s),
                posText);
      posToken++;
      brCount1++; // variable used for bracket check
    }

    // )
    if (tokenInput(&posText, 0) == ')') {
      setTokenType(posToken, SVS_TOKEN_RBR, s);
      tokenInput(&posText, 1);
      tokenDMSG("Token set, type )",
                posToken,
                getTokenData(posToken, s),
                getTokenType(posToken, s),
                posText);
      posToken++;
      brCount1--; //kontrola počtu závorek
    }

    // {
    if (tokenInput(&posText, 0) == '{') {
      setTokenType(posToken, SVS_TOKEN_LCBR, s);
      tokenInput(&posText, 1);
      tokenDMSG("Token set, type {",
                posToken,
                getTokenData(posToken, s),
                getTokenType(posToken, s),
                posText);
      posToken++;
      brCount2++;
    }

    // }
    if (tokenInput(&posText, 0) == '}') {
      setTokenType(posToken, SVS_TOKEN_RCBR, s);
      tokenInput(&posText, 1);
      tokenDMSG("Token set, type }",
                posToken,
                getTokenData(posToken, s),
                getTokenType(posToken, s),
                posText);
      posToken++;
      brCount2--;
    }

    // ;
    if (tokenInput(&posText, 0) == ';') {
      setTokenType(posToken, SVS_TOKEN_SCOL, s);
      tokenInput(&posText, 1);
      tokenDMSG("Token set, type ;",
                posToken,
                getTokenData(posToken, s),
                getTokenType(posToken, s),
                posText);
      posToken++;
    }

    // ,
    if (tokenInput(&posText, 0) == ',') {
      setTokenType(posToken, SVS_TOKEN_COL, s);
      tokenInput(&posText, 1);
      tokenDMSG("Token set, type ,",
                posToken,
                getTokenData(posToken, s),
                getTokenType(posToken, s),
                posText);
      posToken++;
    }

    if (tokenInput(&posText, 0) == '[') {
      setTokenType(posToken, SVS_TOKEN_LSQB, s);
      tokenInput(&posText, 1);
      tokenDMSG("Token set, type [",
                posToken,
                getTokenData(posToken, s),
                getTokenType(posToken, s),
                posText);
      posToken++;
    }

    if (tokenInput(&posText, 0) == ']') {
      setTokenType(posToken, SVS_TOKEN_RSQB, s);
      tokenInput(&posText, 1);
      tokenDMSG("Token set, type ]",
                posToken,
                getTokenData(posToken, s),
                getTokenType(posToken, s),
                posText);
      posToken++;
    }

    //logic
    /*
    Logic:
    18 - ==  - rovno
    19 - !=  - nerovno
    20 - <   - menší než
    21 - >   - větší než
    22 - <=  - menší rovno
    23 - >=  - větší rovno

    24 - = přiřazení
    */

    if (tokenInput(&posText, 0) == '=') { //== a =
      tokenInput(&posText, 1);
      if (tokenInput(&posText, 0) == '=') {
        setTokenType(posToken, SVS_TOKEN_EQUALS, s);
        tokenDMSG("Token set, type ==",
                  posToken,
                  getTokenData(posToken, s),
                  getTokenType(posToken, s),
                  posText);
        tokenInput(&posText, 1);
      } else {
        setTokenType(posToken, SVS_TOKEN_ASSIGN, s);
        tokenDMSG("Token set, type =",
                  posToken,
                  getTokenData(posToken, s),
                  getTokenType(posToken, s),
                  posText);
      }
      posToken++;
    }

    if (tokenInput(&posText, 0) == '<') { //< a <=
      tokenInput(&posText, 1);
      if (tokenInput(&posText, 0) == '=') {
        setTokenType(posToken, SVS_TOKEN_LESS_OR_EQ, s);
        tokenDMSG(
                  "Token set, type <=",
                  posToken,
                  getTokenData(posToken, s),
                  getTokenType(posToken, s),
                  posText);
        tokenInput(&posText, 1);
      } else {
        setTokenType(posToken, SVS_TOKEN_LESS_THAN, s);
        tokenDMSG("Token set, type <",
                  posToken,
                  getTokenData(posToken, s),
                  getTokenType(posToken, s),
                  posText);
      }
      posToken++;
    }

    if (tokenInput(&posText, 0) == '>') { //> a >=
      tokenInput(&posText, 1);
      if (tokenInput(&posText, 0) == '='){ //pouze přiřazení
        setTokenType(posToken, SVS_TOKEN_GREATER_OR_EQ, s);
        tokenDMSG("Token set, type >=",
                  posToken,
                  getTokenData(posToken, s),
                  getTokenType(posToken, s),
                  posText);
        tokenInput(&posText, 1);
      } else {
        setTokenType(posToken, SVS_TOKEN_GREATER_THAN, s);
        tokenDMSG("Token set, type >",
                  posToken,
                  getTokenData(posToken, s),
                  getTokenType(posToken, s),
                  posText);
      }
      posToken++;
    }

    if (tokenInput(&posText, 0) == '!') {
      tokenInput(&posText, 1);
      if (tokenInput(&posText, 0) == '='){ //nerovno
        tokenDMSG("Token set, type !=",
                  posToken,
                  getTokenData(posToken, s),
                  getTokenType(posToken, s),
                  posText);
        tokenInput(&posText, 1);
        setTokenType(posToken, SVS_TOKEN_NOTEQUALS, s);
      } else {
        tokenizerErrorPrint((uint8_t *)"Tokenizer: unknown symbol after \"!\"");
        return 1;
      }
      posToken++;
    }

    // Text constant
    if (tokenInput(&posText, 0) == '\"') {
      tokenInput(&posText, 1);
      setTokenType(posToken, 25, s);
      setTokenData(posToken, (varType)s->stringFieldLen, s);

      // stores where constant strings end in string mem
      // usefull for garbage collection
      INCREMENT_STATIC_STRINGS

      while((tokenInput(&posText, 0) != '\"') && (tokenInput(&posText, 0) != 0)) {
        if (s->stringFieldLen >= (STRING_FIELD_L - 1)) {
          tokenizerErrorPrint((uint8_t *)"Tokenizer: String field full!");
          return 1;
        }
        if(tokenInput(&posText, 0) != '\\') {
          s->stringField[s->stringFieldLen] = tokenInput(&posText, 0);
        } else {
          tokenInput(&posText, 1);
          esc_c_prac = tokenInput(&posText, 0);
          if (esc_c_prac == 'a') {
            s->stringField[s->stringFieldLen] = '\a';
          } else if (esc_c_prac == 'b') {
            s->stringField[s->stringFieldLen] = '\b';
          } else if (esc_c_prac == 'f') {
            s->stringField[s->stringFieldLen] = '\f';
          } else if (esc_c_prac == 'n') {
            s->stringField[s->stringFieldLen] = '\n';
          } else if (esc_c_prac == 'r') {
            s->stringField[s->stringFieldLen] = '\r';
          } else if (esc_c_prac == 't') {
            s->stringField[s->stringFieldLen] = '\t';
          } else if (esc_c_prac == 'v') {
            s->stringField[s->stringFieldLen] = '\v';
          } else if(esc_c_prac == '\\') {
            s->stringField[s->stringFieldLen] = '\\';
          } else if(esc_c_prac == '\'') {
            s->stringField[s->stringFieldLen] = '\'';
          } else if(esc_c_prac=='\"') {
            s->stringField[s->stringFieldLen] = '\"';
          } else {
            tokenizerErrorPrint((uint8_t *)"tokenParse:Undefined character after \"\\\"");
            return 1;
          }

        }
        tokenInput(&posText, 1);
        s->stringFieldLen++;
      }
      s->stringField[s->stringFieldLen] = 0;
      s->stringFieldLen++; //míří vždy na první volné místo
      tokenInput(&posText, 1);
      tokenDMSG("Token set, type STR, value:",
                posToken,
                getTokenData(posToken, s),
                getTokenType(posToken, s),
                posText);
      tokenDMSG((char *)s->stringField + getTokenData(posToken,  s).val_str,
                posToken,
                getTokenData(posToken, s),
                getTokenType(posToken, s),
                posText);
      posToken++;
    }

    // & - creates string constant of a following symbol
    if (tokenInput(&posText, 0) == '&') {
      tokenInput(&posText, 1);

      setTokenType(posToken, SVS_TOKEN_AMP, s);
      setTokenData(posToken, (varType)s->stringFieldLen, s);
      INCREMENT_STATIC_STRINGS

      while((isRegChar(tokenInput(&posText, 0)) || isNumber(tokenInput(&posText, 0))) && (tokenInput(&posText, 0) != 0)) {
        if (s->stringFieldLen >= (STRING_FIELD_L - 1)) {
          tokenizerErrorPrint((uint8_t *)"Tokenizer: String field full!");
          return 1;
        }

        s->stringField[s->stringFieldLen] = tokenInput(&posText, 0);
        tokenInput(&posText, 1);
        s->stringFieldLen++;
      }
      s->stringField[s->stringFieldLen] = 0;
      s->stringFieldLen++; // always points on the next free
      tokenDMSG(
          "Token set, type STR, value:",
          posToken,
          getTokenData(posToken,s),
          getTokenType(posToken,s),
          posText
      );
      tokenDMSG(
          (char*)s->stringField + getTokenData(posToken,s).val_str,
          posToken,
          getTokenData(posToken,s),
          getTokenType(posToken,s),
          posText
      );
      posToken++;
    }

    // analizace textu
    if (isRegChar(tokenInput(&posText, 0))) {
      // A-Z || a-z || _
      if (getKeyword(pracName, &posText)) {
        tokenizerErrorPrint((uint8_t *)"tokenParse: Symbol name too long!");
        return 1;
      }
      // detekce klíčových slov v prac name
      /*
      11 - function - ??
      12 - if
      13 - else
      14 - while
      15 - break
      16 - return

      když nic, tak
      10 - var
      */

      if (strCmp(pracName, (uint8_t *)"end")) {
        setTokenType(posToken, SVS_TOKEN_ENDPROG, s);
        tokenDMSG("Token set, type EOF/END",
                  posToken,
                  getTokenData(posToken, s),
                  getTokenType(posToken, s),
                  posText);
        printf("Tokenizer Done, %u of %u tokens used.\n", s->tokenMax, getTokenMax(s));

        if (tokenEofCheck(brCount1, brCount2)) {
          return 1;
        } else {
          return 0;
        }

      }
      if (strCmp(pracName, (uint8_t *)"if")) {
        tokenDMSG("Token set, type IF",
                  posToken,
                  getTokenData(posToken, s),
                  getTokenType(posToken, s),
                  posText);
        setTokenType(posToken, SVS_TOKEN_IF, s);
        Lock = 0;
      }
      if (strCmp(pracName, (uint8_t *)"else")) {
        tokenDMSG("Token set, type ELSE",
                  posToken,
                  getTokenData(posToken, s),
                  getTokenType(posToken, s),
                  posText);
        setTokenType(posToken, SVS_TOKEN_ELSE, s);
        Lock = 0;
      }
      if (strCmp(pracName, (uint8_t *)"while")) {
        tokenDMSG("Token set, type WHILE",
                  posToken,
                  getTokenData(posToken, s),
                  getTokenType(posToken, s),
                  posText);
        setTokenType(posToken, SVS_TOKEN_WHILE, s);
        Lock = 0;
      }
      if (strCmp(pracName, (uint8_t *)"break")) {
        tokenDMSG("Token set, type BREAK",
                  posToken,
                  getTokenData(posToken, s),
                  getTokenType(posToken, s),
                  posText);
        setTokenType(posToken, SVS_TOKEN_BREAK, s);
        Lock = 0;
      }
      if (strCmp(pracName, (uint8_t *)"return")) {
        tokenDMSG("Token set, type RETURN",
                  posToken,
                  getTokenData(posToken, s),
                  getTokenType(posToken, s),
                  posText);
        setTokenType(posToken, SVS_TOKEN_RETURN, s);
        setTokenData(posToken, (varType)((uint16_t)0), s);
        Lock = 0;
      }

      if (strCmp(pracName, (uint8_t *)"local")) {
        tokenDMSG("Token set, type LOCAL",
                  posToken,
                  getTokenData(posToken, s),
                  getTokenType(posToken, s),
                  posText);
        setTokenType(posToken, SVS_TOKEN_LOCAL, s);
        setTokenData(posToken, (varType)((uint16_t)0), s);
        Lock = 0;
      }

      if (strCmp(pracName, (uint8_t *)"for")) {
        tokenDMSG("Token set, type FOR",
                  posToken,
                  getTokenData(posToken, s),
                  getTokenType(posToken, s),
                  posText);
        setTokenType(posToken, SVS_TOKEN_FOR, s);
        setTokenData(posToken, (varType)((uint16_t)0), s);
        Lock = 0;
      }

      if (pracName[0] == 'a'
          && pracName[1] == 'r'
          && pracName[2] == 'g'
          && pracName[3] >= '0'
          && pracName[3] <= '9'
      ) {
        setTokenType(posToken, SVS_TOKEN_ARG, s);
        setTokenData(posToken, (varType)((uint16_t)(pracName[3] - '0')), s);
        tokenDMSG("Token set, type ARG",
          posToken,
          getTokenData(posToken, s),
          getTokenType(posToken, s),
          posText
        );
        Lock = 0;
      }

      if (strCmp(pracName, (uint8_t *)"and")){
        tokenDMSG("Token set, type AND",
                  posToken,
                  getTokenData(posToken, s),
                  getTokenType(posToken, s),
                  posText);
        setTokenType(posToken, SVS_TOKEN_AND, s);
        setTokenData(posToken, (varType)((uint16_t)9), s);
        Lock = 0;
      }

      if (strCmp(pracName, (uint8_t *)"or")) {
        tokenDMSG("Token set, type OR",
                  posToken,
                  getTokenData(posToken, s),
                  getTokenType(posToken, s),
                  posText);
        setTokenType(posToken, SVS_TOKEN_OR, s);
        setTokenData(posToken, (varType)((uint16_t)9), s);
        Lock = 0;
      }

      if (strCmp(pracName, (uint8_t *)"not")) {
        tokenDMSG("Token set, type NOT",
                  posToken,
                  getTokenData(posToken, s),
                  getTokenType(posToken, s),
                  posText);
        setTokenType(posToken, SVS_TOKEN_NOT, s);
        Lock = 0;
      }

      if (strCmp(pracName, (uint8_t *)"array")) {
        tokenDMSG("Token set, type ARRAY",
                  posToken,
                  getTokenData(posToken, s),
                  getTokenType(posToken, s),
                  posText);
        setTokenType(posToken, SVS_TOKEN_ARRAY, s);
        Lock = 0;
      }

      //check for constants
      for(x = 0; (x < sysConstsNum) && (Lock == 1); x++) {
        uint16_t y = 0;
        while ((strCmp((uint8_t *)"end", (uint8_t *)(sysConsts[x])[y].name) != 1) && (Lock == 1)) {
          if(strCmp(pracName, (uint8_t *)(sysConsts[x])[y].name)) {
            tokenDMSG("Token set, type SYS CONSTANT",
                      posToken,
                      getTokenData(posToken, s),
                      getTokenType(posToken, s),
                      posText);
            setTokenType(posToken, 0, s);
            setTokenData(posToken, (varType)((sysConsts[x])[y].value), s);
            Lock = 0;
          }
          y++;
        }
      }

      //end of constants check
      if (strCmp(pracName, (uint8_t *)"sys")) {
        setTokenType(posToken, SVS_TOKEN_SYS, s);
        setTokenData(posToken, (varType)((uint16_t)0), s);
        Lock = 0;

        // now it is time to get the sys function call name:
        // filtering whitespace etc.
        while(1) {
          if (isRegChar(tokenInput(&posText, 0))) {
            break;
          } else {
            if (!(tokenInput(&posText, 0) == ' ' || tokenInput(&posText, 0) == '.')) {
              tokenizerErrorPrint((uint8_t *)"tokenParse: Undefined symbol after sys statement!");
              return 1;
            }
          }
          tokenInput(&posText, 1);
        }

        // function name or wrapper name
        if (getKeywordExt(pracName2, &posText)) {
          tokenizerErrorPrint((uint8_t *)"tokenParse: internal function name too long!");
          printf("Function: %s \n", pracName2);
          return 1;
        }

        if (tokenInput(&posText, 0) == '.') {
          tokenInput(&posText, 1);
          uint16_t wrapperId;
          wrapperId = getSysWrapperId(pracName2);
          if (wrapperId) {
            setTokenData(posToken, (varType)((uint32_t)wrapperId - 1), s);
            posToken++;
            setTokenType(posToken, SVS_TOKEN_SYS, s);
            setTokenData(posToken, (varType)((uint16_t)0), s);
          } else {
            tokenizerErrorPrint((uint8_t *)"tokenParse: Named wrapper not found:");
            printf("Wrapper not found: %s\n", pracName2);
            return 1;
          }

          if (getKeyword(pracName2, &posText)) {
            tokenizerErrorPrint((uint8_t *)"tokenParse: internal function name too long!");
            return 1;
          }
        }

        // název interní funkce máme v pracName2
        // Zde se dosazují indexy interních funkcí z pole sys name list
        // končícího end

        if (syscallExists(pracName2, s)) { // check if syscall exists
          // then set token as that syscall
          setTokenData(posToken, (varType)((uint16_t)syscallGetId(pracName2, s)), s);
          tokenDMSG("Token set, type existing SYSCALL",
                    posToken,
                    getTokenData(posToken, s),
                    getTokenType(posToken, s),
                    posText);
        } else {
          // add sys call
          if(s->syscallTableLen < SYSCALL_TABLE_L) {
            s->syscallTableLen++;

            s->syscallTable[s->syscallTableLen].sysCallName = strNewP(pracName2, s);
            INCREMENT_STATIC_STRINGS
            setTokenData(posToken, (varType) s->syscallTableLen, s);
            #ifdef WEBTARGET
            //doNotRemoveThese(s->syscallTableLen, s->syscallTable[s->syscallTableLen].sysCallName);
            //printf("Hotfix: There must be a better way to tell the llvm to not throw away theese variables than with printf... %u - %s \nyeah, I tried volatilize the s**t out, didn't help.\n",s->syscallTableLen,s->syscallTable[s->syscallTableLen].sysCallName );
            #endif
            //printf("syscall id: %u, name: %s \n",s->syscallTableLen,s->syscallTable[s->syscallTableLen].sysCallName );
            tokenDMSG("Token set, type new SYSCALL",
                      posToken,
                      getTokenData(posToken, s),
                      getTokenType(posToken, s),
                      posText);
          } else {
            tokenizerErrorPrint((uint8_t *)"tokenParse: Too many SYS calls in one file!");
            return 1;
          }
        }
      }

      //definice funkce: kontrola funkční lookup table -> přidání názvu a tokenu do funkční lookup table
      if (strCmp(pracName, (uint8_t *)"function")) {
        setTokenType(posToken, SVS_TOKEN_FUNCTION, s);
        tokenDMSG("Token set, type FUNCTION, name:",
                  posToken,
                  getTokenData(posToken, s),
                  getTokenType(posToken, s),
                  posText);
        Lock = 0;
        //získání názvu funkce:
        //filtrace bordelu
        while(1) {
          if (isRegChar(tokenInput(&posText, 0))) {
            break;
          } else {
            if (tokenInput(&posText, 0) != ' ') {
              tokenizerErrorPrint((uint8_t *)"tokenParse: Undefined symbol after function statement!");
              return 1;
            }
          }
          tokenInput(&posText, 1);
        }
        //získání názvu funkce
        if (getKeyword(pracName2, &posText)) {
          tokenizerErrorPrint((uint8_t *)"tokenParse: function name too long!");
          return 1;
        }

        //název funkce máme v pracName2, zkotrolujem function lookup table
        if (functionExists(pracName2, s)) {
          tokenizerErrorPrint((uint8_t *)"Error: multiple definitions of function!");
          return 1;
        } else {
          // add new function
          if (s->funcTableLen < FUNCTION_TABLE_L) {
            s->funcTableLen++;
            s->funcTable[s->funcTableLen].tokenId = posToken + 1; // point token to first token of the function
            s->funcTable[s->funcTableLen].name = strNewP(pracName2, s);
            INCREMENT_STATIC_STRINGS
          } else {
            tokenizerErrorPrint((uint8_t *)"Error: too many functions!");
            return 1;
          }
        }
        tokenDMSG((char *)s->funcTable[s->funcTableLen].name,
                  posToken,
                  getTokenData(posToken, s),
                  getTokenType(posToken, s),
                  posText);
      }

      if (Lock) {
        // variable statement can be folowed with only these symbols
        // there was attempt to do some sort of syntax check on tokenizer level
        // but the outcome was only this.
        while(1) {
          if (isRegChar(tokenInput(&posText, 0))
              || (tokenInput(&posText, 0) == 0)
              || (tokenInput(&posText, 0) == '(')
              || (tokenInput(&posText, 0) == '=')
              || (tokenInput(&posText, 0) == '<')
              || (tokenInput(&posText, 0) == '>')
              || (tokenInput(&posText, 0) == ';')
              || (tokenInput(&posText, 0) == ',')
              || (tokenInput(&posText, 0) == '+')
              || (tokenInput(&posText, 0) == '-')
              || (tokenInput(&posText, 0) == '*')
              || (tokenInput(&posText, 0) == '/')
              || (tokenInput(&posText, 0) == '!')
              || (tokenInput(&posText, 0) == ')')
              || (tokenInput(&posText, 0) == '%')
              || (tokenInput(&posText, 0) == '\n')
              || (tokenInput(&posText, 0) == '[')
              || (tokenInput(&posText, 0) == ']')){
            break;
          } else {
            if (tokenInput(&posText, 0) != ' ') {
              tokenizerErrorPrint((uint8_t *)"tokenParse: Undefined symbol after variable statement!");
              printf("symbol: %u (%c)\n", tokenInput(&posText, 0), tokenInput(&posText, 0));
              return 1;
            }
          }
          tokenInput(&posText, 1);
        }
        if (tokenInput(&posText, 0) != '(') { //pokud nejde o volání fce
          if (varExists(pracName, s)) { //jedná se o existující promněnou
            setTokenType(posToken, SVS_TOKEN_VAR, s);
            setTokenData(posToken, varGetId(pracName, s), s);
            tokenDMSG("Token set, type VAR, name:",
                      posToken,
                      getTokenData(posToken, s),
                      getTokenType(posToken, s),
                      posText);
            tokenDMSG((char *)pracName,
                      posToken,
                      getTokenData(posToken, s),
                      getTokenType(posToken, s),
                      posText);
          } else {
            s->varTableLen++;
            setTokenType(posToken, SVS_TOKEN_VAR, s); //nastavíme typ
            setTokenData(posToken, (varType) s->varTableLen, s);
            s->varTable[s->varTableLen].type = SVS_TYPE_UNDEF;
            s->varTable[s->varTableLen].name = strNewP(pracName, s);
            INCREMENT_STATIC_STRINGS

            tokenDMSG("New variable found!",
                      posToken,
                      getTokenData(posToken, s),
                      getTokenType(posToken, s),
                      posText);
            tokenDMSG("Token set, type VAR, name:",
                      posToken,
                      getTokenData(posToken, s),
                      getTokenType(posToken, s),
                      posText);
            tokenDMSG((char *)pracName,
                      posToken,
                      getTokenData(posToken, s),
                      getTokenType(posToken, s),
                      posText);
          }
        } else {
            // if it is built-in function call and function with that name is not registered
            // we register it as a built-in call, otherwise it will be a normal call
            if (getBuiltInCallId(pracName) && !(functionExists(pracName, s))) {
              setTokenType(posToken, SVS_TOKEN_FUNCTION_BUILTIN, s);
              setTokenData(posToken, (varType)getBuiltInCallId(pracName), s);
              tokenDMSG("Token set, type BUILT-IN CALL, function name:",
                          posToken, getTokenData(posToken, s),
                          getTokenType(posToken, s),
                          posText
                        );
              tokenDMSG((char *)pracName,
                          posToken,
                          getTokenData(posToken,s),
                          getTokenType(posToken,s),
                          posText
                        );
            } else {
              setTokenType(posToken, SVS_TOKEN_CALL, s);
              INCREMENT_STATIC_STRINGS
              // TODO: deduplicate call names somehow
              setTokenData(posToken, (varType)strNew(pracName, s), s);
              if (errCheck(s)) {
                return 1;
              }
              tokenDMSG("Token set, type CALL, function name:",
                          posToken,
                          getTokenData(posToken,s),
                          getTokenType(posToken,s),
                          posText
                        );
              tokenDMSG((char *)s->stringField + getTokenData(posToken, s).val_str,
                          posToken,
                          getTokenData(posToken, s),
                          getTokenType(posToken, s),
                          posText
                        );
            }

        }
      }
      posToken++;
    }

    //eof
    if (tokenInput(&posText, 0) == 0) {
      setTokenType(posToken, SVS_TOKEN_ENDPROG, s);
      tokenDMSG("Token set, type EOF/END", \
                  posToken, \
                  getTokenData(posToken,s), \
                  getTokenType(posToken,s), \
                  posText \
                );
      printf("Tokenizer Done, %u of %u tokens used.\n", s->tokenMax, getTokenMax(s));
      if (tokenEofCheck(brCount1, brCount2)) {
        return 1;
      } else {
        return 0;
      }
    }

    //line level debug
    if (tokenizer_exact_debug == 1) {
      if(tokenizer_exact_token <= posToken - 1) {
        return 0;
      }
    }

    // update délky programu
    // update script lenght (in tokens)
    s->tokenMax = posToken;
  }

  errSoft((uint8_t *)"tokenParse: Tokenizer overflow!",s);
  return 1;
}

