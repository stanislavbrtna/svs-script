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

uint8_t filename[32];
uint8_t ldinit;
uint8_t ldmode;

void setFName(uint8_t * name) {
  uint16_t x;
  for(x = 0; x < 32; x++) {
	  filename[x] = name[x];
  }
}

void tokenGetchSetup(uint8_t * fname, uint8_t mode) {
  setFName(fname);
  ldinit = 0;
  ldmode = mode;
}


uint8_t loadApp(uint8_t *fname, uint8_t *name, svsVM *s, uint8_t mode) {
	static uint8_t fsloaded;
	svsReset(s);
	svsSetName(name,s);
	setFName(fname);
	svsSetFileName(fname, s);
  printf("SVS loadApp: loading: %s\n", fname);
	ldinit = 0;
	ldmode = mode; //0=file 1=uart

	tokenizerReset();

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
uint8_t tokenGetch() {
	static FIL fp;
	static FRESULT fr;
	uint8_t x = 0;
	if(ldmode == 0) {
		if (ldinit == 0) {
			printf("Opening file: %s\n", filename);
			fr = f_open(&fp, filename, FA_READ);
			if (fr != FR_OK) {
			  errMsgS("tokenGetch: Error while opening file!");
			}
			ldinit = 1;
		}
		f_read(&fp,&x,sizeof(x),&fr);
		//printf("tokenGetch: %c\n",x  );
		if (f_eof(&fp)) {
			f_close(&fp);
			return 0;
		} else {
		  return x;
		}
	} else {
		if (ldinit == 0) {
			ldinit = 1;
		}
		return x;
	}
}

#else

uint8_t tokenGetch() {
	static FILE *fp;
	uint8_t x;

	if (ldinit == 0) {
		if(ldmode == 0) {
		  printf("tokenGetch: Opening file: %s\n", filename);
		  fp = fopen(filename,"r");
		  if(fp == NULL) {
			  puts("tokenGetch: Error while opening file!");
			  return(0);
		  }
		} else {
		  puts("tokenGetch: Serial loading is not supported on PC");
		  return(0);
		}
		ldinit = 1;
	}
	x = fgetc(fp);

	if (feof(fp)) {
		fclose(fp);
		return 0;
	} else {
#ifdef TOK_CHECK
  printf("%c", x);
#endif
	  return x;
	}
}
#endif

