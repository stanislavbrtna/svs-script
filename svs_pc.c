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

#define MAIN_SVS
#include "svs.h"
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
char *progname;

void tokenPrint(svsVM *s){
	int x=0;
	puts ("TokenList:");
	//getTokenMax(s)
	for (x=0;x<80;x++){
		if (getTokenType(x, s)==255){
			puts ("EOF token");
			break;
		}
		if (getTokenType(x, s)!=31){
		  printf("index:%u type:%u data:%u \n",x,getTokenType(x, s), getTokenData(x, s).val_u);
		}else{
		  printf("index:%u type:%u data:%f \n",x,getTokenType(x, s), getTokenData(x, s).val_f);
		}
	}
}

void varTablePrint(svsVM *s){
	uint16_t x=1;
	puts ("varTable:");
	while(x<=s->varTableLen){
		if(varGetType((varType)x,s)==0){
			printf("var id: %u name: %s type: num value: %i \n", x, s->varTable[x].name, s->varTable[x].value.val_s);
		}else if (varGetType((varType)x,s)==1){
			printf("var id: %u name: %s type: str value: %s\n",x, s->varTable[x].name, s->stringField+s->varTable[x].value.val_str);
		}else if(varGetType((varType)x,s)==3){
			printf("var id: %u name: %s type: float value: %f \n", x, s->varTable[x].name, s->varTable[x].value.val_f);
		}
		x++;
	}
}

void strTablePrint(svsVM *s){
	int x=0;
	puts ("stringTable:");
	printf("%u: ",x);
	while(x<s->stringFieldLen){
		if (s->stringField[x]!=0){
			printf("%c",s->stringField[x]);
		}else{
			printf("\n%u: ",x);
		}
		x++;
	}
}

void pcBasicWrapInit();

#ifdef CMDLINE
int main(int argc, char *argv[]){
  svsVM s;
	varRetVal result;
	uint8_t dbginfo=0;
	int retval=0;
	puts("S! Script interpreter v."SVS_VERSION);
	setTokenizerDebug(0);
	setCommExDebug(0);
	setExprExecDebug(0);
	pcBasicWrapInit();
	progname=argv[1];
	if (argc==1){
	printf("Usage:\n%s <fname> [-t] [-i]\n\
     fname - name of the script file\n\
     -t    - optional tokenizer debug output\n\
     -i    - optional debug output\n\n(c) Standa\n", argv[0]);
  return 0;
	}

	if (argc>2){
	  if (strCmp(argv[2], "-t")){
	    setTokenizerDebug(1);
	  }
	}

	if (argc>2){
	  if (strCmp(argv[2], "-i")){
	    dbginfo=1;
	  }
	}

	if (argc>3){
	  if (strCmp(argv[3], "-i")){
	    dbginfo=1;
	  }
	}

	if (loadApp(progname,"svs-cache", &s,0)){
	  printf("Error while loading!\n");
	  return 0;
	}

#ifdef TOK_CHECK
  //performing tokenizer check only
  tokenPrint(&s);
  return 0;
#endif
	puts("Script output begin:\n");
	commExec("main", &s);
	errSoftPrint(&s);


	if (s.commRetType==0){
	  printf("Returned: %i\n",getRetValInt(&s));
	  retval=(int)getRetValInt(&s);
	}else if(s.commRetType==1){
	  printf("Returned: %s\n", getRetValStr(&s));
	}else if(s.commRetType==2) {
	  printf("Returned: %f\n",getRetValFlt(&s));
	}

	if(dbginfo==1){
	  printf("DBG Info:\n");
	  tokenPrint(&s);
	  puts("Variables:\n");
	  varTablePrint(&s);
	  strTablePrint(&s);
	}
#ifndef TOKEN_CACHE_DISABLED
	unlink("svs-cache");
#endif

	return retval;
}
#endif
