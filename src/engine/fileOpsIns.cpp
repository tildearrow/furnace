/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "engine.h"
#include "../ta-log.h"
#include "../fileutils.h"
#include "importExport/importExport.h"
#include <fmt/printf.h>
#include <limits.h>

enum DivInsFormats {
  DIV_INSFORMAT_DMP,
  DIV_INSFORMAT_TFI,
  DIV_INSFORMAT_VGI,
  DIV_INSFORMAT_FTI,
  DIV_INSFORMAT_BTI,
  DIV_INSFORMAT_S3I,
  DIV_INSFORMAT_SBI,
  DIV_INSFORMAT_Y12,
  DIV_INSFORMAT_OPLI,
  DIV_INSFORMAT_OPNI,
  DIV_INSFORMAT_BNK,
  DIV_INSFORMAT_GYB,
  DIV_INSFORMAT_OPM,
  DIV_INSFORMAT_WOPL,
  DIV_INSFORMAT_WOPN,
  DIV_INSFORMAT_FF,
};

std::vector<DivInstrument*> DivEngine::instrumentFromFile(const char* path, bool loadAssets, bool readInsName) {
  std::vector<DivInstrument*> ret;
  warnings="";

  const char* pathRedux=strrchr(path,DIR_SEPARATOR);
  if (pathRedux==NULL) {
    pathRedux=path;
  } else {
    pathRedux++;
  }
  String stripPath;
  const char* pathReduxEnd=strrchr(pathRedux,'.');
  if (pathReduxEnd==NULL) {
    stripPath=pathRedux;
  } else {
    for (const char* i=pathRedux; i!=pathReduxEnd && (*i); i++) {
      stripPath+=*i;
    }
  }

  FILE* f=ps_fopen(path,"rb");
  if (f==NULL) {
    lastError=strerror(errno);
    return ret;
  }
  unsigned char* buf;
  ssize_t len;
  if (fseek(f,0,SEEK_END)!=0) {
    lastError=strerror(errno);
    fclose(f);
    return ret;
  }
  len=ftell(f);
  if (len<0) {
    lastError=strerror(errno);
    fclose(f);
    return ret;
  }
  if (len==(SIZE_MAX>>1)) {
    lastError=strerror(errno);
    fclose(f);
    return ret;
  }
  if (len==0) {
    lastError=strerror(errno);
    fclose(f);
    return ret;
  }
  if (fseek(f,0,SEEK_SET)!=0) {
    lastError=strerror(errno);
    fclose(f);
    return ret;
  }
  buf=new unsigned char[len];
  if (fread(buf,1,len,f)!=(size_t)len) {
    logW("did not read entire instrument file buffer!");
    lastError="did not read entire instrument file!";
    delete[] buf;
    return ret;
  }
  fclose(f);

  SafeReader reader=SafeReader(buf,len);

  unsigned char magic[16];
  bool isFurnaceInstr=false;
  bool isOldFurnaceIns=false;
  try {
    reader.read(magic,4);
    if (memcmp("FINS",magic,4)==0) {
      isFurnaceInstr=true;
      logV("found a new Furnace ins");
    } 
    else if (memcmp("FINB",magic,4)==0) {
      isFurnaceInstr=true;
      logV("found a new Furnace-B ins");
    }
    else {
      reader.read(&magic[4],12);
      if (memcmp("-Furnace instr.-",magic,16)==0) {
        logV("found an old Furnace ins");
        isFurnaceInstr=true;
        isOldFurnaceIns=true;
      }
    }
  } catch (EndOfFileException& e) {
    reader.seek(0,SEEK_SET);
  }

  if (isFurnaceInstr) {
    DivInstrument* ins=new DivInstrument;
    try {
      short version=0;
      if (isOldFurnaceIns) {
        version=reader.readS();
        reader.readS(); // reserved
      } else {
        version=reader.readS();
        reader.seek(0,SEEK_SET);
      }

      if (version>DIV_ENGINE_VERSION) {
        warnings="this instrument is made with a more recent version of Furnace!";
      }

      if (isOldFurnaceIns) {
        unsigned int dataPtr=reader.readI();
        reader.seek(dataPtr,SEEK_SET);
      }

      ins->name=stripPath;

      if (ins->readInsData(reader,version,loadAssets?(&song):NULL)!=DIV_DATA_SUCCESS) {
        lastError="invalid instrument header/data!";
        delete ins;
        delete[] buf;
        return ret;
      } else {
        if (!readInsName) {
          ins->name=stripPath;
        }
        ret.push_back(ins);
      }
    } catch (EndOfFileException& e) {
      lastError="premature end of file";
      logE("premature end of file");
      delete ins;
      delete[] buf;
      return ret;
    }
  } else { // read as a different format
    const char* ext=strrchr(path,'.');
    DivInsFormats format=DIV_INSFORMAT_DMP;
    if (ext!=NULL) {
      String extS;
      for (; *ext; ext++) {
        char i=*ext;
        if (i>='A' && i<='Z') {
          i+='a'-'A';
        }
        extS+=i;
      }
      if (extS==".dmp") {
        format=DIV_INSFORMAT_DMP;
      } else if (extS==".tfi") {
        format=DIV_INSFORMAT_TFI;
      } else if (extS==".vgi") {
        format=DIV_INSFORMAT_VGI;
      } else if (extS==".fti") {
        format=DIV_INSFORMAT_FTI;
      } else if (extS==".bti") {
        format=DIV_INSFORMAT_BTI;
      } else if (extS==".s3i") {
        format=DIV_INSFORMAT_S3I;
      } else if (extS==".sbi") {
        format=DIV_INSFORMAT_SBI;
      } else if (extS==".opli") {
        format=DIV_INSFORMAT_OPLI;
      } else if (extS==".opni") {
        format=DIV_INSFORMAT_OPNI;
      } else if (extS==".y12") {
        format=DIV_INSFORMAT_Y12;
      } else if (extS==".bnk") {
        format=DIV_INSFORMAT_BNK;
      } else if (extS==".gyb") {
        format=DIV_INSFORMAT_GYB;
      } else if (extS==".opm") {
        format=DIV_INSFORMAT_OPM;
      } else if (extS==".ff") {
        format=DIV_INSFORMAT_FF;
      } else if (extS==".wopl") {
        format=DIV_INSFORMAT_WOPL;
      } else if (extS==".wopn") {
        format=DIV_INSFORMAT_WOPN;
      } else {
        // unknown format
        lastError="unknown instrument format";
        delete[] buf;
        return ret;
      }
    }

    switch (format) {
      case DIV_INSFORMAT_DMP:
        loadDMP(reader,ret,stripPath);
        break;
      case DIV_INSFORMAT_TFI:
        loadTFI(reader,ret,stripPath);
        break;
      case DIV_INSFORMAT_VGI:
        loadVGI(reader,ret,stripPath);
        break;
      case DIV_INSFORMAT_FTI: // TODO
        break;
      case DIV_INSFORMAT_BTI: // TODO
        break;
      case DIV_INSFORMAT_S3I:
        loadS3I(reader,ret,stripPath);
        break;
      case DIV_INSFORMAT_SBI:
        loadSBI(reader,ret,stripPath);
        break;
      case DIV_INSFORMAT_OPLI:
        loadOPLI(reader,ret,stripPath);
        break;
      case DIV_INSFORMAT_OPNI:
        loadOPNI(reader,ret,stripPath);
        break;
      case DIV_INSFORMAT_Y12:
        loadY12(reader,ret,stripPath);
        break;
      case DIV_INSFORMAT_BNK:
        loadBNK(reader,ret,stripPath);
        break;
      case DIV_INSFORMAT_FF:
        loadFF(reader,ret,stripPath);
        break;
      case DIV_INSFORMAT_GYB:
        loadGYB(reader,ret,stripPath);
        break;
      case DIV_INSFORMAT_OPM:
        loadOPM(reader,ret,stripPath);
        break;
      case DIV_INSFORMAT_WOPL:
        loadWOPL(reader,ret,stripPath);
        break;
      case DIV_INSFORMAT_WOPN:
        loadWOPN(reader,ret,stripPath);
        break;
    }

    if (reader.tell()<reader.size()) {
      addWarning("https://github.com/tildearrow/furnace/issues/84");
      addWarning("there is more data at the end of the file! what happened here!");
      addWarning(fmt::sprintf("exactly %d bytes, if you are curious",reader.size()-reader.tell()));
    }
  }

  delete[] buf; // since we're done with this buffer
  return ret;
}
