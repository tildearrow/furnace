#include "engine.h"

void DivEngine::nextOrder() {
  curRow=0;
  if (++curOrder>=song.ordersLen) {
    curOrder=0;
  }
}

const char* notes[12]={
  "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
};

const char* formatNote(unsigned char note, unsigned char octave) {
  static char ret[4];
  if (note==100) {
    return "OFF";
  } else if (octave==0 && note==0) {
    return "---";
  }
  snprintf(ret,4,"%s%d",notes[note%12],octave+note/12);
  return ret;
}

bool DivEngine::perSystemEffect(int ch, unsigned char effect, unsigned char effectVal) {
  switch (song.system) {
    case DIV_SYSTEM_GENESIS:
      switch (effect) {
        case 0x17: // DAC enable
          dispatch->dispatch(DivCommand(DIV_CMD_SAMPLE_MODE,ch,(effectVal>0)));
          break;
        default:
          return false;
      }
      break;
    default:
      return false;
  }
  return true;
}

void DivEngine::nextRow() {
  static char pb[4096];
  static char pb1[4096];
  static char pb2[4096];
  static char pb3[4096];
  if (++curRow>=song.patLen) {
    nextOrder();
  }
  if (changeOrd>=0) {
    curRow=changePos;
    curOrder=changeOrd;
    if (curOrder>=song.ordersLen) {
      curOrder=0;
    }
    changeOrd=-1;
  }
  strcpy(pb1,"");
  strcpy(pb3,"");
  for (int i=0; i<chans; i++) {
    snprintf(pb,4095," %.2x",song.orders.ord[i][curOrder]);
    strcat(pb1,pb);
    
    DivPattern* pat=song.pat[i]->data[curOrder];
    snprintf(pb2,4095,"\x1b[37m %s",
             formatNote(pat->data[curRow][0],pat->data[curRow][1]));
    strcat(pb3,pb2);
    if (pat->data[curRow][3]==255) {
      strcat(pb3,"\x1b[m--");
    } else {
      snprintf(pb2,4095,"\x1b[1;32m%.2x",pat->data[curRow][3]);
      strcat(pb3,pb2);
    }
    if (pat->data[curRow][2]==255) {
      strcat(pb3,"\x1b[m--");
    } else {
      snprintf(pb2,4095,"\x1b[0;36m%.2x",pat->data[curRow][2]);
      strcat(pb3,pb2);
    }
    for (int j=0; j<song.pat[i]->effectRows; j++) {
      if (pat->data[curRow][4+(j<<1)]==255) {
        strcat(pb3,"\x1b[m--");
      } else {
        snprintf(pb2,4095,"\x1b[1;31m%.2x",pat->data[curRow][4+(j<<1)]);
        strcat(pb3,pb2);
      }
      if (pat->data[curRow][5+(j<<1)]==255) {
        strcat(pb3,"\x1b[m--");
      } else {
        snprintf(pb2,4095,"\x1b[1;37m%.2x",pat->data[curRow][5+(j<<1)]);
        strcat(pb3,pb2);
      }
    }
  }
  printf("| %.2x:%s | \x1b[1;33m%3d%s\x1b[m\n",curOrder,pb1,curRow,pb3);

  for (int i=0; i<chans; i++) {
    DivPattern* pat=song.pat[i]->data[curOrder];
    // effects
    for (int j=0; j<song.pat[i]->effectRows; j++) {
      unsigned char effect=pat->data[curRow][4+(j<<1)];
      unsigned char effectVal=pat->data[curRow][5+(j<<1)];

      // per-system effect
      if (!perSystemEffect(i,effect,effectVal)) switch (effect) {
        case 0x09: // speed 1
          song.speed1=effectVal;
          break;
        case 0x0f: // speed 2
          song.speed2=effectVal;
          break;
        case 0x0b: // change order
          changeOrd=effectVal;
          changePos=0;
          break;
        case 0x0d: // next order
          changeOrd=curOrder+1;
          changePos=effectVal;
          break;
        case 0x08: // panning
          dispatch->dispatch(DivCommand(DIV_CMD_PANNING,i,effectVal));
          break;
      }
    }

    // instrument
    if (pat->data[curRow][2]!=255) {
      dispatch->dispatch(DivCommand(DIV_CMD_INSTRUMENT,i,pat->data[curRow][2]));
    }
    // note
    if (pat->data[curRow][0]==100) {
      dispatch->dispatch(DivCommand(DIV_CMD_NOTE_OFF,i));
    } else if (!(pat->data[curRow][0]==0 && pat->data[curRow][1]==0)) {
      dispatch->dispatch(DivCommand(DIV_CMD_NOTE_ON,i,pat->data[curRow][0]+pat->data[curRow][1]*12));
    }

    // volume
    if (pat->data[curRow][3]!=255) {
      dispatch->dispatch(DivCommand(DIV_CMD_VOLUME,i,pat->data[curRow][3]));
    }


  }
}

void DivEngine::nextTick() {
  if (song.customTempo) {
    cycles=dispatch->rate/song.hz;
  } else {
    if (song.pal) {
      cycles=dispatch->rate/60;
    } else {
      cycles=dispatch->rate/50;
    }
  }
  if (--ticks<=0) {
    nextRow();
    if (speedAB) {
      ticks=song.speed2*(song.timeBase+1);
    } else {
      ticks=song.speed1*(song.timeBase+1);
    }
    speedAB=!speedAB;
  }
  dispatch->tick();
}

void DivEngine::nextBuf(float** in, float** out, int inChans, int outChans, unsigned int size) {
  size_t runtotal=blip_clocks_needed(bb[0],size);
  for (size_t i=0; i<runtotal; i++) {
    if (--cycles<=0) {
      nextTick();
    }
    dispatch->acquire(temp[0],temp[1]);

    blip_add_delta(bb[0],i,temp[0]-prevSample[0]);
    blip_add_delta(bb[1],i,temp[1]-prevSample[1]);
    prevSample[0]=temp[0];
    prevSample[1]=temp[1];
  }

  blip_end_frame(bb[0],runtotal);
  blip_end_frame(bb[1],runtotal);

  blip_read_samples(bb[0],bbOut[0],size,0);
  blip_read_samples(bb[1],bbOut[1],size,0);

  for (size_t i=0; i<size; i++) {
    out[0][i]=(float)bbOut[0][i]/16384.0;
    out[1][i]=(float)bbOut[1][i]/16384.0;
  }
}
