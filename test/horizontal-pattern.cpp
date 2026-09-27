// Regression tests for piano-roll edits. No GUI or audio device required.
// Build: c++ -std=c++14 -fsanitize=address,undefined -g test/horizontal-pattern.cpp -o /tmp/horizontal-pattern-test
#include "../src/gui/horizontalPattern.h"
#include <cassert>
#include <cstdio>

static short data[DIV_MAX_ROWS][DIV_MAX_COLS];
static short before[DIV_MAX_ROWS][DIV_MAX_COLS];
static void clear() {
  for (int r=0; r<DIV_MAX_ROWS; r++) {
    for (int c=0; c<DIV_MAX_COLS; c++) data[r][c]=-1;
  }
}

int main() {
  using namespace FurnaceHorizontalPattern;
  clear();
  data[4][DIV_PAT_FX(0)]=0x0f;
  data[4][DIV_PAT_FXVAL(0)]=6;
  assert(edit(data,64,1,0,4,108,8,2));
  assert(data[4][DIV_PAT_NOTE]==108 && data[4][DIV_PAT_INS]==2);
  assert(data[8][DIV_PAT_NOTE]==DIV_NOTE_OFF && endRow(data,64,4)==8);
  assert(data[4][DIV_PAT_FXVAL(0)]==6);
  data[4][DIV_PAT_VOL]=12;
  assert(edit(data,64,2,4,12,110,16,-1));
  assert(data[4][DIV_PAT_NOTE]==-1 && data[8][DIV_PAT_NOTE]==-1);
  assert(data[12][DIV_PAT_INS]==2 && data[12][DIV_PAT_VOL]==12);
  assert(data[4][DIV_PAT_FX(0)]==0x0f && data[12][DIV_PAT_FX(0)]==-1);
  assert(edit(data,64,2,12,12,110,20,-1));
  assert(data[16][DIV_PAT_NOTE]==-1 && data[20][DIV_PAT_NOTE]==DIV_NOTE_OFF);
  assert(edit(data,64,1,0,24,112,28,3));
  memcpy(before,data,sizeof(data));
  assert(!edit(data,64,2,12,22,110,26,-1));
  assert(memcmp(before,data,sizeof(data))==0); // collisions are atomic
  assert(edit(data,64,3,12,0,0,0,-1));
  assert(data[12][DIV_PAT_NOTE]==-1 && data[20][DIV_PAT_NOTE]==-1);
  assert(data[24][DIV_PAT_NOTE]==112 && data[4][DIV_PAT_FXVAL(0)]==6);

  clear();
  assert(edit(data,64,1,0,60,179,64,0)); // boundary: no out-of-range cut
  assert(endRow(data,64,60)==64 && data[64][DIV_PAT_NOTE]==-1);
  assert(!edit(data,64,1,0,63,180,64,0));
  assert(!edit(data,64,1,0,-1,108,4,0));
  assert(!edit(data,64,1,0,60,108,65,0));
  assert(!edit(data,64,2,64,0,108,4,0));
  assert(!edit(data,0,1,0,0,108,4,0));

  clear();
  data[8][DIV_PAT_NOTE]=112;
  assert(edit(data,64,1,0,4,108,8,0)); // adjacent note is not overwritten
  assert(data[8][DIV_PAT_NOTE]==112);
  data[6][DIV_PAT_FX(7)]=0xee;
  data[6][DIV_PAT_FXVAL(7)]=0xaa;
  assert(edit(data,64,3,4,0,0,0,-1));
  assert(data[8][DIV_PAT_NOTE]==112 && data[6][DIV_PAT_FXVAL(7)]==0xaa);

  clear();
  data[8][DIV_PAT_NOTE]=DIV_NOTE_REL;
  assert(edit(data,64,1,0,4,108,8,0));
  assert(edit(data,64,3,4,0,0,0,-1));
  assert(data[8][DIV_PAT_NOTE]==DIV_NOTE_REL); // keep independent release events
  data[4][DIV_PAT_INS]=7;
  memcpy(before,data,sizeof(data));
  assert(!edit(data,64,1,0,4,108,6,0)); // keep instrument-only events
  assert(memcmp(before,data,sizeof(data))==0);
  clear();
  assert(edit(data,64,1,0,0,108,4,0));
  assert(edit(data,64,1,0,4,112,8,0));
  assert(endRow(data,64,0)==4 && data[4][DIV_PAT_NOTE]==112);

  clear();
  data[0][DIV_PAT_NOTE]=DIV_NOTE_REL;
  assert(eventPitch(data,64,0,60)==60); // orphan event uses current pitch
  data[4][DIV_PAT_NOTE]=108;
  data[8][DIV_PAT_NOTE]=DIV_NOTE_OFF;
  data[12][DIV_PAT_NOTE]=DIV_MACRO_REL;
  data[16][DIV_PAT_NOTE]=112;
  memcpy(before,data,sizeof(data));
  assert(eventPitch(data,64,0,60)==108); // leading release follows next note
  assert(eventPitch(data,64,4,60)==108);
  assert(eventPitch(data,64,8,60)==108);
  assert(eventPitch(data,64,12,60)==108); // skip intervening unpitched events
  assert(endRow(data,64,8)==12);
  assert(memcmp(before,data,sizeof(data))==0); // rendering does not alter song
  data[4][DIV_PAT_NOTE]=DIV_NOTE_RAW;
  data[4][DIV_PAT_NOTE_BUFFER]=110;
  assert(eventPitch(data,64,0,60)==110);
  assert(eventPitch(data,64,4,60)==110);
  assert(eventPitch(data,64,8,60)==110);
  assert(strcmp(eventLabel(DIV_NOTE_OFF),"OFF")==0);
  assert(strcmp(eventLabel(DIV_NOTE_REL),"REL")==0);
  assert(strcmp(eventLabel(DIV_MACRO_REL),"MREL")==0);
  assert(strcmp(eventLabel(DIV_NOTE_RAW),"RAW")==0);
  puts("Horizontal pattern regression tests passed.");
}
