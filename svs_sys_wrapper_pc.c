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

#include "svs_basics.h"

svsConstType pcBasicWrapConsts[]={
  {"TEST_VAL", 5},
  {"TEST_VAL2", 8},
  // type defines
  {"TYPE_NUM", 0},
  {"TYPE_STR", 1},
  {"TYPE_FLOAT", 3},
  {"TYPE_ARRAY", 4},
  {"TYPE_UNDEF", 5},

  {"end", 0}
};


uint8_t pcBasicsWrap(varRetVal *result, argStruct *argS, svsVM *s);

void pcBasicWrapInit(){
  addSysWrapper(pcBasicsWrap, (uint8_t *)"dbg");
  addSysConsts(pcBasicWrapConsts);
}

uint8_t pcBasicsWrap(varRetVal *result, argStruct *argS, svsVM *s){
  uint16_t x;
  uint8_t argType[11];

  //printf("basics wrap got: %s \n",s->syscallTable[argS->callId.val_u].sysCallName);

  // sys test(1,2,3,4,5);
  if (sysFuncMatch(argS->callId,"test",s)){ //test
    argType[1]=0;
    argType[2]=0;
    argType[3]=0;
    argType[4]=0;
    argType[5]=0;
    if(sysExecTypeCheck(argS, argType, 5,s)){
      return 0;
    }
    printf("Sys test called!  arg1=%i arg2=%i arg3=%i arg4=%i arg5=%i \n",
    		(int)argS->arg[1].val_s,
				(int)argS->arg[2].val_s,
				(int)argS->arg[3].val_s,
				(int)argS->arg[4].val_s,
				(int)argS->arg[5].val_s);
    result->value.val_s = 8;
    return 1;
  }

  //sys dbg(num lvl);
  //debug pro comm exec a expr exec
  if (sysFuncMatch(argS->callId,"dbg",s)){
    argType[1]=0;
    if (sysExecTypeCheck(argS, argType, 1,s)){
      return 0;
    }
    s->globalDebug = (uint8_t) argS->arg[1].val_s;
    result->value.val_s = 0;
    return 1;
  }

  //sys profiler(enable);
  //GC profiler
  if (sysFuncMatch(argS->callId,"profiler",s)){
    argType[1]=0;
    if (sysExecTypeCheck(argS, argType, 1,s)){
      return 0;
    }
    s->profilerEnabled=argS->arg[1].val_s;

    result->value.val_s=0;
    return 1;
  }

  //sys GC(num to free);
  if (sysFuncMatch(argS->callId,"GC",s)){
    argType[1]=0;
    if (sysExecTypeCheck(argS, argType, 1,s)){
      return 0;
    }

    garbageCollect(argS->arg[1].val_s,s);

    result->value.val_s=0;
    return 1;
  }

  //sys dbgGc(num lvl);
  //debug pro GC
  if (sysFuncMatch(argS->callId,"dbgGc",s)){
    argType[1]=0;
    if (sysExecTypeCheck(argS, argType, 1,s)){
      return 0;
    }
    setGcDebug(argS->arg[1].val_s);
    result->value.val_s=0;
    return 1;
  }

  // dbgCache
  if (sysFuncMatch(argS->callId,"dbgCache",s)){
    argType[1]=0;
    if (sysExecTypeCheck(argS, argType, 1,s)){
      return 0;
    }
    setCacheDebug(argS->arg[1].val_s);
    result->value.val_s=0;
    return 1;
  }

  //sys info();
  if (sysFuncMatch(argS->callId,"info",s)){
    if (sysExecTypeCheck(argS, argType, 0,s)){
      return 0;
    }

    svsInfo(s);
    return 1;
  }

  //sys.heatmapStart();
  if (sysFuncMatch(argS->callId,"heatmapStart",s)){
    if (sysExecTypeCheck(argS, argType, 0,s)){
      return 0;
    }

    #ifdef SVS_DBG_HEATMAP
    svs_heatmap_start(s);
    #else
    puts("sys.heatmapStart(): Disabled in this build.");
    #endif
    return 1;
  }

  //sys.heatmapStop();
  if (sysFuncMatch(argS->callId,"heatmapStop",s)){
    if (sysExecTypeCheck(argS, argType, 0,s)){
      return 0;
    }

    #ifdef SVS_DBG_HEATMAP
    svs_heatmap_stop();
    #else
    puts("sys.heatmapStop(): Disabled in this build.");
    #endif

    return 1;
  }

  //sys.heatmapPrint();
  if (sysFuncMatch(argS->callId,"heatmapPrint",s)){
    if (sysExecTypeCheck(argS, argType, 0,s)){
      return 0;
    }

    #ifdef SVS_DBG_HEATMAP
    svs_heatmap_print(s);
    #else
    puts("sys.heatmapPrint(): Disabled in this build.");
    #endif

    return 1;
  }

  //sys print(str);
  if (sysFuncMatch(argS->callId,"print",s)){
    argType[1]=1;
    if(sysExecTypeCheck(argS,argType, 1,s)){
      return 0;
    }
    x=0;
    while(argS->arg[1].val_s+x<s->stringFieldLen){
      if (s->stringField[x+argS->arg[1].val_str]!=0){
        printf("%c",s->stringField[argS->arg[1].val_str+x]);
      }else{
        break;
      }
      x++;
    }
    puts("");

    return 1;
  }

  return 0;
}

