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

void pcBasicWrapInit();

#ifdef CMDLINE
int main(int argc, char *argv[]) {
  svsVM s;
  varRetVal result;
  time_t t;
  uint8_t dbginfo = 0;
  int retval = 0;
  puts("S! Script interpreter v."SVS_VERSION);
  srand((unsigned) time(&t));
  setTokenizerDebug(0);
  setCommExDebug(0);
  setExprExecDebug(0);
  pcBasicWrapInit();
  progname = argv[1];
  if (argc == 1) {
  printf("Usage:\n%s <fname> [-t] [-i]\n\
     fname - name of the script file\n\
     -t    - optional tokenizer debug output\n\
     -i    - optional debug output\n\n(c) Standa\n", argv[0]);
  return 0;
  }

  if (argc > 2) {
    if (strCmp(argv[2], "-t")) {
      setTokenizerDebug(1);
    }
  }

  if (argc>2){
    if (strCmp(argv[2], "-i")) {
      dbginfo = 1;
    }
  }

  if (argc>3){
    if (strCmp(argv[3], "-i")) {
      dbginfo = 1;
    }
  }

  if (loadApp(progname,"svs-cache", &s, 0)) {
    printf("Error while loading!\n");
    return 0;
  }

#ifdef TOK_CHECK
  //performing tokenizer check only
  tokenPrint(&s);
  return 0;
#endif
  puts("Script output begin:\n");
  if (functionExists("main", &s)){
    commExec("main", &s);
  } else{
    commExecFromStart(&s);
  }

  printf("\n");

  if (errCheck(&s)) {
    retval = -1;
    errSoftPrint(&s);
    printf("\n");
  } else {
    if (s.commRetType == 0) {
      printf("Returned: %i\n", getRetValInt(&s));
      retval = (int)getRetValInt(&s);
    } else if(s.commRetType == 1) {
      printf("Returned: %s\n", getRetValStr(&s));
    } else if(s.commRetType == 2) {
      printf("Returned: %f\n", getRetValFlt(&s));
    }
  }

  if (dbginfo == 1) {
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
