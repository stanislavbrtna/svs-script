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

#ifndef SVS_TYPES_H
#define SVS_TYPES_H

//#define VARTYPE int16_t
#define VARTYPE varType

#define SVS_TYPE_NUM 0
#define SVS_TYPE_STR 1
#define SVS_TYPE_FLT 3
#define SVS_TYPE_ARR 4
#define SVS_TYPE_UNDEF 5

#define SVS_TOKEN_VAR 10
#define SVS_TOKEN_FUNCTION 11
#define SVS_TOKEN_IF 12
#define SVS_TOKEN_ELSE 13
#define SVS_TOKEN_WHILE 14
#define SVS_TOKEN_BREAK 15
#define SVS_TOKEN_RETURN 16
#define SVS_TOKEN_CALL 17
#define SVS_TOKEN_EQUALS 18
#define SVS_TOKEN_NOTEQUALS 19
#define SVS_TOKEN_LESS_THAN 20
#define SVS_TOKEN_LESS_OR_EQ 22
#define SVS_TOKEN_GREATER_THAN 21
#define SVS_TOKEN_GREATER_OR_EQ 23
#define SVS_TOKEN_ASSIGN 24
#define SVS_TOKEN_AMP 25
#define SVS_TOKEN_SYS 28
#define SVS_TOKEN_FOR 34
#define SVS_TOKEN_LOCAL 32
#define SVS_TOKEN_ARG 35

#define SVS_TOKEN_CONST_NUM 0
#define SVS_TOKEN_CONST_FLOAT 31

#define SVS_TOKEN_ADD 1
#define SVS_TOKEN_SUBT 2
#define SVS_TOKEN_MUL 3
#define SVS_TOKEN_DIV 4
#define SVS_TOKEN_MOD 30
#define SVS_TOKEN_LBR 5
#define SVS_TOKEN_RBR 6
#define SVS_TOKEN_LCBR 7 // {
#define SVS_TOKEN_RCBR 8 // }
#define SVS_TOKEN_SCOL 9 // ;
#define SVS_TOKEN_COL 33 // ,

#define SVS_TOKEN_AND 37
#define SVS_TOKEN_OR 38
#define SVS_TOKEN_NOT 39

#define SVS_TOKEN_ARRAY 40
#define SVS_TOKEN_LSQB 41 // [
#define SVS_TOKEN_RSQB 42 // ]

#define SVS_TOKEN_FUNCTION_BUILTIN 36

#define SVS_TOKEN_ENDPROG 255


typedef struct {
  uint8_t *name;
  uint16_t tokenId;
} fTableType;

typedef union {
   int32_t val_s; //signed const
   uint16_t val_str; //variable Id
   uint32_t val_u; //variable Id
   float val_f; //float value
} varType;

typedef struct {
  uint8_t *name;
  uint32_t maskId; // maskovací id lokální proměnné
  varType value; //num - hodnota, str-nultý charakter v tabulce, string je ukončen \0
  uint8_t type; //0-num (int32) 1-string 3-float
} varTableType;

typedef struct { //struktura kterou vrací funkce výsledek
  varType value; //num - hodnota, str-nultý charakter v tabulce, string je ukončen \0
  uint8_t type; //0-num 1-string
  uint16_t tokenId; //pokud funkce vrací i návratovou adresu
} varRetVal;

typedef struct {
  char *errString;
  uint16_t tokenId;
} errStruct;

typedef struct {
  uint8_t Type;
  varType Data;
} tokenCacheStruct;

typedef struct {
  uint8_t *sysCallName;
} sysCall;


typedef struct {
  VARTYPE arg[FUNCTION_ARGS_MAX+1];      //hodnoty argumentů
  uint8_t argType[FUNCTION_ARGS_MAX+1];  //typy agrumentů
  uint8_t usedup;       //počet použitých argumentů
} comExArgs;


typedef struct {
  uint8_t vmName[NAME_LENGTH+1]; //název svsVM

  uint8_t fName[SVS_FILE_NAME_L]; //název svs souboru

#ifdef PC //chache file, odlišné pro pc a fatfs
  FILE *vmCache;
#else
  FIL vmCache;
  FRESULT cacheFr;
#endif
  uint8_t vmCacheUsed;
  uint16_t cacheStart;
  uint16_t tokenMax;

  fTableType funcTable[FUNCTION_TABLE_L+1]; //indexace od jedničky
  uint16_t	funcTableLen; //počet funkcí

  sysCall syscallTable[SYSCALL_TABLE_L+1]; //indexace od jedničky
  uint16_t  syscallTableLen; //počet volání

  varTableType varTable[VAR_TABLE_L+1]; //indexace od jedničky
  uint16_t varTableLen; //pročet promněných

  uint8_t stringField[STRING_FIELD_L+1]; //indexace od nuly
  // number of used chars, always points to the next free char
  uint16_t stringFieldLen;
  uint16_t stringConstMax;
  uint16_t gcSafePoint;

  comExArgs commArgs;

  uint8_t commFlag;
  varType commRetVal;
  uint8_t commRetType;
  uint8_t commRetFlag;
  uint16_t progLine;
  uint8_t globalDebug; //povoluje ladící režim
  uint8_t handbrake; //pokud 1, zastavuje virtuální stroj

  tokenCacheStruct tokenCache[TOKEN_LENGTH+1]; //cache tokenů

  varType varArray[SVS_ARRAY_LEN+1];
  uint8_t varArrayType[SVS_ARRAY_LEN+1];
  uint16_t varArrayLen;

  uint8_t profilerEnabled;

  //soft errors
  uint8_t err;
  char *errString;
  volatile uint16_t errToken;
  uint16_t errDbgVar[5];
  uint8_t *errDbgStr[5];
  uint8_t errDbgUsed;

} svsVM;


#endif
