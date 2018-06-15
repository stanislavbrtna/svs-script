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

typedef struct {
  uint8_t fString[NAME_LENGTH];
  uint16_t tokenId;
} fTableType;

typedef union{
   int32_t val_s; //signed const
   uint16_t val_str; //variable Id
   uint32_t val_u; //variable Id
   float val_f; //float value
} varType;

typedef struct {
  uint8_t name[NAME_LENGTH];
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
  uint8_t sysCallName[NAME_LENGTH];
} sysCall;


typedef struct {
  VARTYPE arg[FUNCTION_ARGS_MAX+1];      //hodnoty argumentů
  uint8_t argType[FUNCTION_ARGS_MAX+1];  //typy agrumentů
  uint8_t usedup;       //počet použitých argumentů
} comExArgs;


typedef struct {
  uint8_t vmName[NAME_LENGTH+1]; //název svsVM

  uint8_t fName[FILE_NAME_L]; //název svs souboru

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
