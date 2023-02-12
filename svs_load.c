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

#include "svs_load.h"

uint8_t ldinit;
uint8_t ldmode;
uint8_t fclosed;


void tokenGetchSetup(uint8_t * fname, uint8_t mode) {
  ldinit = 0;
  ldmode = mode;
  fclosed = 0;
}


uint8_t loadApp(uint8_t *fname, uint8_t *name, svsVM *s, uint8_t mode) {
  //static uint8_t fsloaded;
  svsReset(s);
  svsSetName(name,s);
  svsSetFileName(fname, s);
  printf("SVS loadApp: loading: %s\n", fname);
  ldinit = 0;
  ldmode = mode; //0=file 1=uart

  fclosed = 0;
  tokenizerReset(s);

  if(mode == 1) {
    setTokenType(0, 0, s);
    printf("Serial loading Ready!\n");
  }

  if (tokenParse(s) != 0) {
    return 1;
  }
  #ifndef PC
  f_sync(&(s->vmCache));
  #endif
  printf("Program loaded!\n");
  return 0;
}

#ifndef PC

static FIL fp;

uint8_t tokenGetch(svsVM *s) {
  uint8_t x = 0;
  uint32_t br;

  if (ldmode == 0) {
    if (ldinit == 0) {
      if (f_open(&fp, (char *)s->fName, FA_READ) != FR_OK) {
        printf("tokenGetch: Opening file: %s\n", s->fName);
        errMsgS((uint8_t *)"tokenGetch: Error while opening file!");
        return 0;
      }
      ldinit = 1;
    }

    if (fclosed == 0) {
      if (f_read(&fp, &x, sizeof(x), &br) != FR_OK) {
        errMsgS((uint8_t *)"tokenGetch: Error while reading from file!");
      }

      if (f_eof(&fp)) {
        f_close(&fp);
        fclosed = 1;
        return 0;
      } else {
        return x;
      }
    } else {
      printf("Reading file while fclosed!\n%s\n", s->fName);
      return 0;
    }
  } else {
    return 0;
  }
}

uint8_t tokenGetchOpen(uint8_t *fname, svsVM *s) {
  if (f_open(&fp, (char *)fname, FA_READ) != FR_OK) {
    puts("tokenGetch: Error while opening file!");
    errMsgS((uint8_t *)"tokenGetch: Error while opening file!");
    return(0);
  }

  return 1;
}

FIL tokenGetchGetFP() {
  return fp;
}

void tokenGetchSetFP(FIL f) {
  fp = f;
}

#else

static FILE *fp;

uint8_t tokenGetch(svsVM *s) {
  uint8_t x;

  if (ldinit == 0) {
    if(ldmode == 0) {
      
      fp = fopen(s->fName, "r");
      if(fp == NULL) {
        printf("tokenGetch: Opening file: %s\n", s->fName);
        puts("tokenGetch: Error while opening file!");
        return(0);
      }
    } else {
      puts("tokenGetch: Serial loading is not supported on PC");
      return(0);
    }
    ldinit = 1;
  }
  if (fclosed == 0) {
    x = fgetc(fp);

    if (feof(fp)) {
      fclose(fp);
      fclosed = 1;
      return 0;
    } else {
  #ifdef TOK_CHECK
    printf("%c", x);
  #endif
      return x;
    }
  } else {
    printf("Reading file while fclosed!\n%s\n", s->fName);
    return 0;
  }
}

uint8_t tokenGetchOpen(uint8_t *fname, svsVM *s) {
  fp = fopen(fname, "r");
  if(fp == NULL) {
    puts("tokenGetch: Error while opening file!");
    return(0);
  }

  return 1;
}

FILE * tokenGetchGetFP() {
  return fp;
}

void tokenGetchSetFP(FILE * f) {
  fp = f;
}

#endif

