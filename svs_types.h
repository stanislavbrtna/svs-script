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
#define SVS_TOKEN_SYS 28
#define SVS_TOKEN_FOR 34
#define SVS_TOKEN_LOCAL 32
#define SVS_TOKEN_ARG 35

#define SVS_TOKEN_CONST_NUM 0
#define SVS_TOKEN_CONST_STR 25
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

#define SVS_TOKEN_CONTINUE 43

#define SVS_TOKEN_ENDPROG 255


typedef struct {
  uint8_t *name;
  uint16_t tokenId;
} fTableType;

typedef union {
  int32_t  val_s;   // signed type, used for SVS_TYPE_NUM
  uint16_t val_str; // position in string field, used for SVS_TYPE_STR
  uint32_t val_u;   // Used for system call ids
  float    val_f;   // float value for SVS_TYPE_FLT
} varType;

typedef struct {
  uint8_t *name;
  uint32_t maskId; // maskovací id lokální proměnné/mask-id used for loacal variables
  varType  value;   // num - hodnota, str-nultý charakter v tabulce, string je ukončen \0 / value of the variable
  uint8_t  type;    // variable type defined in SVS_TYPE_*
} varTableType;

typedef struct {    // return structure for a function call
  varType  value;   
  uint8_t  type;     // var type
  uint16_t tokenId; // token to return to
} varRetVal;

typedef struct {
  char     *errString;
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
  VARTYPE arg[FUNCTION_ARGS_MAX+1];      // argument values
  uint8_t argType[FUNCTION_ARGS_MAX+1];  // types
  uint8_t usedup;                        // count of used arguments
} comExArgs;


typedef struct {
  uint8_t vmName[SVS_FILE_NAME_L+1];  // vm name
  uint8_t fName[SVS_FILE_NAME_L]; // .svs file name

#ifdef PC //chache file, different for pc and umc
  FILE *vmCache;
#else
  FIL vmCache;
  FRESULT cacheFr;
#endif
  // token cache
  uint8_t          vmCacheUsed;
  uint16_t         cacheStart;
  uint16_t         tokenMax;
  tokenCacheStruct tokenCache[TOKEN_LENGTH+1];

#ifdef SVS_TOKEN_CACHE_SEGMENTED
  uint16_t tokenSegmentStart[TOKEN_SEGMENTS];
  uint16_t tokenSegmentValid[TOKEN_SEGMENTS];
  uint16_t tokenSegmentHits[TOKEN_SEGMENTS];
  uint8_t  tokenLastSegment;
#endif

  fTableType   funcTable[FUNCTION_TABLE_L+1];   // indexed from 1
  uint16_t	   funcTableLen;                    // number of functions in a script

  sysCall      syscallTable[SYSCALL_TABLE_L+1]; // indexed from 1
  uint16_t     syscallTableLen;                 // number of sys.* calls

  varTableType varTable[VAR_TABLE_L+1];         // indexed from 1
  uint16_t     varTableLen;                     // number of variables

#ifdef SVS_USE_SEPARATE_STRING_FIELD
  uint8_t  *stringField;                        // indexed from 0
  uint32_t  stringFieldMax;
#else
  uint8_t   stringField[STRING_FIELD_L + 1];    // indexed from 0
  uint32_t  stringFieldMax;
#endif
  
  // number of used chars, always points to the next free char
  uint16_t stringFieldLen;
  
  // position where string constants ends in a string field, so garbage collector doesnt throw them away
  uint16_t stringConstMax;
  // string garbage collection safe point 
  uint16_t gcSafePoint;
  // string garbage collection profiler status
  uint8_t  profilerEnabled;

  comExArgs commArgs;
  uint8_t   commFlag;
  varType   commRetVal;
  uint8_t   commRetType;
  uint8_t   commRetFlag;
  uint16_t  progLine;
  
  uint8_t   globalDebug; // debug mode
  uint8_t   handbrake; // if 1, vm tries to halt the execution

  // array field
  varType  varArray[SVS_ARRAY_LEN+1];
  uint8_t  varArrayType[SVS_ARRAY_LEN+1];
  uint16_t varArrayLen;

  //soft errors
  uint8_t  err;
  char     *errString;
  volatile uint16_t errToken;
  uint16_t  errDbgVar[5];
  uint8_t  *errDbgStr[5];
  uint8_t   errDbgUsed;

} svsVM;


#endif
