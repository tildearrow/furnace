#include "gui.h"
#include "guiConst.h"
#include "../ta-log.h"

int MIDIMap::at(const TAMidiMessage& where) {
  if (map==NULL) return 0;
  int type=(where.type>>4)-8;
  int chan=where.type&15;
  int data1=where.data[0];
  int data2=where.data[1];
  if (type<0) return 0;
  if (map[type]==NULL) return 0;
  
  int** chanMap=map[type][chan];
  if (chanMap==NULL) {
    chanMap=map[type][16];
    if (chanMap==NULL) return 0;
  }

  int* dataMap=chanMap[data1];
  if (dataMap==NULL) {
    dataMap=chanMap[128];
    if (dataMap==NULL) return 0;
  }

  int ret=dataMap[data2];
  if (ret==0) {
    ret=dataMap[128];
    if (ret==0) { // maybe this is not the correct mapping
      dataMap=chanMap[128];
      if (dataMap==NULL) {
        chanMap=map[type][16];
        if (chanMap==NULL) return 0;

        dataMap=chanMap[data1];
        if (dataMap==NULL) {
          dataMap=chanMap[128];
          if (dataMap==NULL) return 0;
        }

        ret=dataMap[data2];
        if (ret==0) {
          ret=dataMap[128];
        }
      } else {
        ret=dataMap[data2];
        if (ret==0) {
          ret=dataMap[128];
        }
      }
    }
  }

  return ret;
}

#define UNDERSTAND_OPTION(x) if (optionNameS==#x) { \
  x=std::stoi(optionValueS); \
}

#define UNDERSTAND_FLOAT_OPTION(x) if (optionNameS==#x) { \
  x=std::stof(optionValueS); \
}

#define UNDERSTAND_ARRAY_OPTION(x,yMax) if (optionNameS==#x) { \
  if (optionIndex<0 || optionIndex>=yMax) { \
    logW("MIDI map array option %d out of range (0-%d) at line %d: %s",optionIndex,yMax,curLine,line); \
    break; \
  } \
  x[optionIndex]=std::stoi(optionValueS); \
}

bool MIDIMap::read(String path) {
  char line[4096];
  int curLine=1;
  FILE* f=fopen(path.c_str(),"rb");
  if (f==NULL) {
    if (errno!=ENOENT) {
      logE("error while loading MIDI mapping! %s",strerror(errno));
    }
    return false;
  }

  binds.clear();
  while (fgets(line,4096,f)) {
    char* nlPos=strrchr(line,'\n');
    if (nlPos!=NULL) *nlPos=0;
    if (strstr(line,"aOption")==line) {
      char optionName[256];
      int optionIndex=-1;
      char optionValue[256];
      String optionNameS, optionValueS;

      int result=sscanf(line,"aOption %255s %d %255s",optionName,&optionIndex,optionValue);
      if (result!=3) {
        logW("MIDI map garbage data at line %d: %s",curLine,line);
        break;
      }

      optionNameS=optionName;
      optionValueS=optionValue;

      try {
        UNDERSTAND_ARRAY_OPTION(valueInputSpecificStyle,18) else
        UNDERSTAND_ARRAY_OPTION(valueInputSpecificMSB,18) else
        UNDERSTAND_ARRAY_OPTION(valueInputSpecificLSB,18) else
        UNDERSTAND_ARRAY_OPTION(valueInputSpecificSingle,18) else {
          logW("MIDI map unknown array option %s at line %d: %s",optionName,curLine,line);
        }
      } catch (std::out_of_range& e) {
        logW("MIDI map invalid value %s for array option %s at line %d: %s",optionValue,optionName,curLine,line);
      } catch (std::invalid_argument& e) {
        logW("MIDI map invalid value %s for array option %s at line %d: %s",optionValue,optionName,curLine,line);
      }

      curLine++;
      continue;
    }
    if (strstr(line,"option")==line) {
      char optionName[256];
      char optionValue[256];
      String optionNameS, optionValueS;
      int result=sscanf(line,"option %255s %255s",optionName,optionValue);
      if (result!=2) {
        logW("MIDI map garbage data at line %d: %s",curLine,line);
        break;
      }

      optionNameS=optionName;
      optionValueS=optionValue;

      try {
        UNDERSTAND_OPTION(noteInput) else
        UNDERSTAND_OPTION(volInput) else
        UNDERSTAND_OPTION(rawVolume) else
        UNDERSTAND_OPTION(polyInput) else
        UNDERSTAND_OPTION(directChannel) else
        UNDERSTAND_OPTION(programChange) else
        UNDERSTAND_OPTION(midiClock) else
        UNDERSTAND_OPTION(midiTimeCode) else
        UNDERSTAND_OPTION(yamahaFMResponse) else
        UNDERSTAND_OPTION(directProgram) else
        UNDERSTAND_OPTION(valueInputStyle) else
        UNDERSTAND_OPTION(valueInputControlMSB) else
        UNDERSTAND_OPTION(valueInputControlLSB) else
        UNDERSTAND_OPTION(valueInputControlSingle) else
        UNDERSTAND_FLOAT_OPTION(volExp) else {
          logW("MIDI map unknown option %s at line %d: %s",optionName,curLine,line);
        }
      } catch (std::out_of_range& e) {
        logW("MIDI map invalid value %s for option %s at line %d: %s",optionValue,optionName,curLine,line);
      } catch (std::invalid_argument& e) {
        logW("MIDI map invalid value %s for option %s at line %d: %s",optionValue,optionName,curLine,line);
      }

      curLine++;
      continue;
    }

    char bindAction[256];
    MIDIBind bind;
    int result=sscanf(line,"%d %d %d %d %255s",&bind.type,&bind.channel,&bind.data1,&bind.data2,bindAction);
    if (result!=5 || result==EOF) {
      logW("MIDI map garbage data at line %d: %s",curLine,line);
      break;
    }

    bool foundAction=false;
    for (int i=0; i<GUI_ACTION_MAX; i++) {
      if (strcmp(bindAction,guiActions[i].name)==0) {
        bind.action=i;
        foundAction=true;
        break;
      }
    }
    if (!foundAction) {
      logW("MIDI map unknown action %s at line %d: %s",bindAction,curLine,line);
      break;
    }

    binds.push_back(bind);
    curLine++;
  }

  fclose(f);
  return true;
}

#define WRITE_OPTION(x) fprintf(f,"option " #x " %d\n",x);
#define WRITE_FLOAT_OPTION(x) fprintf(f,"option " #x " %f\n",x);
#define WRITE_ARRAY_OPTION(x,y) fprintf(f,"aOption " #x " %d %d\n",y,x[y]);

bool MIDIMap::write(String path) {
  FILE* f=fopen(path.c_str(),"wb");
  if (f==NULL) {
    logE("error while saving MIDI mapping! %s",strerror(errno));
    return false;
  }

  WRITE_OPTION(noteInput);
  WRITE_OPTION(volInput);
  WRITE_OPTION(rawVolume);
  WRITE_OPTION(polyInput);
  WRITE_OPTION(directChannel);
  WRITE_OPTION(programChange);
  WRITE_OPTION(midiClock);
  WRITE_OPTION(midiTimeCode);
  WRITE_OPTION(yamahaFMResponse);
  WRITE_OPTION(directProgram);
  WRITE_OPTION(valueInputStyle);
  WRITE_OPTION(valueInputControlMSB);
  WRITE_OPTION(valueInputControlLSB);
  WRITE_OPTION(valueInputControlSingle);
  WRITE_FLOAT_OPTION(volExp);

  for (int i=0; i<18; i++) {
    WRITE_ARRAY_OPTION(valueInputSpecificStyle,i);
    WRITE_ARRAY_OPTION(valueInputSpecificMSB,i);
    WRITE_ARRAY_OPTION(valueInputSpecificLSB,i);
    WRITE_ARRAY_OPTION(valueInputSpecificSingle,i);
  }

  for (MIDIBind& i: binds) {
    if (fprintf(f,"%d %d %d %d %s\n",i.type,i.channel,i.data1,i.data2,guiActions[i.action].name)<0) {
      logW("did not write MIDI mapping entirely! %s",strerror(errno));
      break;
    }
  }

  fclose(f);
  return true;
}

void MIDIMap::deinit() {
  if (map!=NULL) {
    for (int i=0; i<8; i++) {
      if (map[i]!=NULL) {
        for (int j=0; j<17; j++) {
          if (map[i][j]!=NULL) {
            for (int k=0; k<129; k++) {
              if (map[i][j][k]!=NULL) {
                delete[] map[i][j][k];
              }
            }
            delete[] map[i][j];
          }
        }
        delete[] map[i];
      }
    }
    delete[] map;
    map=NULL;
  }
}

void MIDIMap::compile() {
  deinit();

  map=new int***[8];
  memset(map,0,sizeof(int***)*8);
  for (MIDIBind& i: binds) {
    if (i.type<8 || i.type>15) continue;
    if (i.channel<0 || i.channel>16) continue;
    if (i.data1<0 || i.data1>128) continue;
    if (i.data2<0 || i.data2>128) continue;

    if (map[i.type-8]==NULL) {
      map[i.type-8]=new int**[17];
      memset(map[i.type-8],0,sizeof(int**)*17);
    }
    if (map[i.type-8][i.channel]==NULL) {
      map[i.type-8][i.channel]=new int*[129];
      memset(map[i.type-8][i.channel],0,sizeof(int*)*129);
    }
    if (map[i.type-8][i.channel][i.data1]==NULL) {
      map[i.type-8][i.channel][i.data1]=new int[129];
      memset(map[i.type-8][i.channel][i.data1],0,sizeof(int)*129);
    }

    map[i.type-8][i.channel][i.data1][i.data2]=i.action;
    logD("MIDI mapping %d %d %d %d to %d",i.type-8,i.channel,i.data1,i.data2,i.action);
  }
}
