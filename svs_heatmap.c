/*
Copyright (c) 2023 Stanislav Brtna

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

#include "svs_heatmap.h"

#ifdef SVS_DBG_HEATMAP

static uint32_t heatmap[65000];
static uint16_t linebreaks[65000];
static uint32_t linebreaksCnt;
static uint8_t enabled;

void svs_heatmap_start(svsVM *s) {
  for (int i = 0; i < 65000; i++) {
    heatmap[i] = 0;
  }
  enabled = 1;
}

void svs_heatmap_mark(uint16_t token, svsVM *s) {
  if (heatmap[token] < 10000 && enabled == 1) {
    heatmap[token]++;
  }
}

void svs_heatmap_set_break(uint16_t token, uint16_t line, svsVM *s) {
  linebreaks[line] = token;
}

void svs_heatmap_stop() {
  enabled = 0;
}

void svs_heatmap_print(svsVM *s) {
  printf("Heatmap:\n");
  int line = 0;
  int currentLine = 0;
  int maxHeat = 0;
  for (int i = 0; i < s->tokenMax; i++) {
    
    if (linebreaks[line] < i) {
      line++;
    }
    
    if(line != currentLine) {
      if (maxHeat > 0) {
        printf("line %03d:  %ux \n", currentLine, maxHeat);
      }
      currentLine = line;
      maxHeat = 0;
    }

    if(heatmap[i] > maxHeat) {
      maxHeat = heatmap[i];
    }
    //printf("token: %05d line %03d: %u \n", i, line, heatmap[i]);
  }
}

#endif