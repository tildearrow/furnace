/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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

#include "spc.h"
#include "../engine.h"
#include "../ta-log.h"
#include "../../fileutils.h"
#include "../../executils.h"
#include <fmt/printf.h>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include "../../utfutils.h"
#else
#include <dirent.h>
#include <unistd.h>
#endif

static const char* snesSources[]={
  "snes.s",
  "spc.i",
  "spc.s",
  "stream.s",
  NULL
};

void DivExportSNES::run() {
  DivObjectPool pool;
  String tempDir=e->createTempDir();

  if (tempDir.empty()) {
    logAppend("ERROR: couldn't create a temporary directory!");
    failed=true;
    running=false;
    return;
  }

  String songDataPath=tempDir+DIR_SEPARATOR_STR+"songData.s";
  String samplePath=tempDir+DIR_SEPARATOR_STR+"sample.bin";
  String songInfoPath=tempDir+DIR_SEPARATOR_STR+"songInfo.s";
  String songSpeedPath=tempDir+DIR_SEPARATOR_STR+"songSpeed.s";
  String linkerInfoPath=tempDir+DIR_SEPARATOR_STR+"spc.ini";

  String asmSourcePath=e->getConfString("exportSourcePath","." DIR_SEPARATOR_STR "drivers");

  String wlaspcPath=e->getConfString("wlaspcPath","");
  String wlalinkPath=e->getConfString("wlalinkPath","");
  
  String romPath=fmt::sprintf("%s%sfurnace.spc",tempDir,DIR_SEPARATOR_STR);

  int chipIndex=conf.getInt("sysToExport",-1);
  if (chipIndex<0 || chipIndex>=e->song.systemLen) {
    chipIndex=-1;
    for (int i=0; i<e->song.systemLen; i++) {
      if (e->song.system[i]==DIV_SYSTEM_SNES) {
        chipIndex=i;
        break;
      }
    }
    if (chipIndex<0) {
      logAppend("ERROR: SNES not found!");
      failed=true;
      running=false;
      return;
    }
  } else if (e->song.system[chipIndex]!=DIV_SYSTEM_SNES) {
    logAppend("ERROR: selected chip is not an SNES!");
    failed=true;
    running=false;
    return;
  }

  DivDispatch* dispatch=e->getDispatch(chipIndex);

  if (dispatch==NULL) {
    logAppend("ERROR: dispatch is NULL!");
    failed=true;
    running=false;
    return;
  }

  // prepare ROM sources
  logAppend("preparing driver source...");
  for (int i=0; snesSources[i]; i++) {
    String srcPath=fmt::sprintf("%s%sspc700%s%s",asmSourcePath,DIR_SEPARATOR_STR,DIR_SEPARATOR_STR,snesSources[i]);
    String destPath=fmt::sprintf("%s%s%s",tempDir,DIR_SEPARATOR_STR,snesSources[i]);
    if (!copyFiles(srcPath.c_str(),destPath.c_str())) {
      logAppend(fmt::sprintf("ERROR: couldn't copy %s to %s! (%s)",srcPath,destPath,strerror(errno)));
    }
  }

  // export chip data
  logAppend("compiling system data...");
  if (!dispatch->compileROMData(1,pool)) {
    logAppend("ERROR: couldn't compile system data!");
    failed=true;
    running=false;
    for (DivObject& i: pool) {
      delete[] i.data;
    }
    pool.clear();
    return;
  }

  // export ins
  logAppend("compiling instruments...");
  if (!e->compileAllIns(pool,DIV_INS_SNES)) {
    logAppend("ERROR: couldn't compile instruments!");
    failed=true;
    running=false;
    for (DivObject& i: pool) {
      delete[] i.data;
    }
    pool.clear();
    return;
  }

  // export seq
  logAppend("compiling sequence data...");
  e->saveCommand(pool);

  // bake
  logAppend("baking song data...");
  SafeWriter* w=bakeObjectsASM(pool);
  if (w==NULL) {
    logAppend("ERROR: couldn't bake song data!");
    for (DivObject& i: pool) {
      delete[] i.data;
    }
    pool.clear();
    failed=true;
    running=false;
    return;
  }

  FILE* f=ps_fopen(songDataPath.c_str(),"wb");
  if (f!=NULL) {
    if (fwrite(w->getFinalBuf(),1,w->size(),f)!=w->size()) {
      logAppend(fmt::sprintf("ERROR: couldn't write data! (%s)",strerror(errno)));
      fclose(f);
      for (DivObject& i: pool) {
        delete[] i.data;
      }
      pool.clear();
      w->finish();
      delete w;
      failed=true;
      running=false;
      return;
    }
    fclose(f);
  } else {
    logAppend(fmt::sprintf("ERROR: couldn't open %s! (%s)",songDataPath,strerror(errno)));
    for (DivObject& i: pool) {
      delete[] i.data;
    }
    pool.clear();
    w->finish();
    delete w;
    failed=true;
    running=false;
    return;
  }
  w->finish();
  delete w;

  // destroy current pool
  for (DivObject& i: pool) {
    delete[] i.data;
  }
  pool.clear();

  // export sample
  logAppend("compiling sample data...");
  if (!dispatch->compileROMData(0,pool)) {
    logAppend("ERROR: couldn't compile system data!");
    failed=true;
    running=false;
    for (DivObject& i: pool) {
      delete[] i.data;
    }
    pool.clear();
    return;
  }

  // bake
  logAppend("baking sample data...");
  w=bakeObjectsBinary(pool,0);
  if (w==NULL) {
    logAppend("ERROR: couldn't bake sample data!");
    failed=true;
    running=false;
    for (DivObject& i: pool) {
      delete[] i.data;
    }
    pool.clear();
    return;
  }

  f=ps_fopen(samplePath.c_str(),"wb");
  if (f!=NULL) {
    if (fwrite(w->getFinalBuf(),1,w->size(),f)!=w->size()) {
      logAppend(fmt::sprintf("ERROR: couldn't write data! (%s)",strerror(errno)));
      fclose(f);
      for (DivObject& i: pool) {
        delete[] i.data;
      }
      pool.clear();
      w->finish();
      delete w;
      failed=true;
      running=false;
      return;
    }
    fclose(f);
  } else {
    logAppend(fmt::sprintf("ERROR: couldn't open %s! (%s)",samplePath,strerror(errno)));
    for (DivObject& i: pool) {
      delete[] i.data;
    }
    pool.clear();
    w->finish();
    delete w;
    failed=true;
    running=false;
    return;
  }
  w->finish();
  delete w;
  w=NULL;

  // destroy current pool
  for (DivObject& i: pool) {
    delete[] i.data;
  }
  pool.clear();

  // write song info and speed
  logAppend("writing song info...");
  f=ps_fopen(songInfoPath.c_str(),"wb");
  if (f==NULL) {
    logAppend(fmt::sprintf("ERROR: couldn't open %s! (%s)",songInfoPath,strerror(errno)));
    failed=true;
    running=false;
    return;
  }

  // pad name to 32 chars
  fprintf(f,
    "; generated by Furnace.\n\n"
    "; name\n"
    ".db "
  );
  for (size_t i=0; i<32; i++) {
    if (i>=e->song.name.size()) {
      fprintf(f,"0,");
    } else {
      fprintf(f,"%d,",e->song.name[i]);
    }
  }

  // pad album to 32 chars
  fprintf(f,
    "\n\n"
    "; album\n"
    ".db "
  );
  for (size_t i=0; i<32; i++) {
    if (i>=e->song.category.size()) {
      fprintf(f,"0,");
    } else {
      fprintf(f,"%d,",e->song.category[i]);
    }
  }

  // dumper
  fprintf(f,
    "\n\n"
    "; dumper\n"
    ".db \"Furnace\"\n"
    ".dsb 9, 0"
  );

  // pad comment to 32 chars
  fprintf(f,
    "\n\n"
    "; comment\n"
    ".db "
  );
  for (size_t i=0; i<32; i++) {
    if (i>=e->song.notes.size()) {
      fprintf(f,"0,");
    } else {
      fprintf(f,"%d,",e->song.notes[i]);
    }
  }

  // write dump date
  time_t thisMakesNoSense=time(NULL);
  struct tm curTime;
#ifdef _WIN32
  struct tm* tempTM=localtime(&thisMakesNoSense);
  if (tempTM==NULL) {
    memset(&curTime,0,sizeof(struct tm));
  } else {
    memcpy(&curTime,tempTM,sizeof(struct tm));
  }
#else
  if (localtime_r(&thisMakesNoSense,&curTime)==NULL) {
    memset(&curTime,0,sizeof(struct tm));
  }
#endif

  fprintf(f,
    "\n\n"
    "; dumper\n"
    ".db \"%02d/%02d/%04d\"\n"
    ".db 0",
    curTime.tm_mon+1,
    curTime.tm_mday,
    (1900+curTime.tm_year)%10000
  );

  // song length
  fprintf(f,
    "\n\n"
    "; song length\n"
    ".db \"%03d\"",
    e->curSubSong->ts.totalTime.seconds+1
  );

  // song fade-out (TODO: make it configurable)
  fprintf(f,
    "\n\n"
    "; song fade-out\n"
    ".db \"10000\""
  );

  // pad author to 32 chars
  fprintf(f,
    "\n\n"
    "; author\n"
    ".db "
  );
  for (size_t i=0; i<32; i++) {
    if (i>=e->song.author.size()) {
      fprintf(f,"0,");
    } else {
      fprintf(f,"%d,",e->song.author[i]);
    }
  }

  // other flags
  fprintf(f,
    "\n\n"
    "; other flags\n"
    ".db 0, 0\n"
  );

  fclose(f);

  f=ps_fopen(songSpeedPath.c_str(),"wb");
  if (f==NULL) {
    logAppend(fmt::sprintf("ERROR: couldn't open %s! (%s)",songSpeedPath,strerror(errno)));
    failed=true;
    running=false;
    return;
  }

  // write tick rate
  fprintf(f,"initTickRate=%d\n",(int)round(e->curSubSong->hz*1000.0f));
  fclose(f);

  f=ps_fopen(linkerInfoPath.c_str(),"wb");
  if (f==NULL) {
    logAppend(fmt::sprintf("ERROR: couldn't open %s! (%s)",songSpeedPath,strerror(errno)));
    failed=true;
    running=false;
    return;
  }

  // write linker info
  fprintf(
    f,
    "[objects]\n"
    "%s%sspc.o\n",
    tempDir.c_str(),
    DIR_SEPARATOR_STR
  );
  fclose(f);

  // prepare compiler arguments
  std::vector<String> compileFlags;
  // include dir
  compileFlags.push_back("-I");
  compileFlags.push_back(tempDir);
  // features/compat flags
  if (e->song.compatFlags.linearPitch) {
    compileFlags.push_back("-D");
    compileFlags.push_back("DIV_LINEAR_FREQ");
  }
  // output path
  compileFlags.push_back("-o");
  compileFlags.push_back(fmt::sprintf("%s%sspc.o",tempDir,DIR_SEPARATOR_STR));
  // source
  compileFlags.push_back(fmt::sprintf("%s%sspc.s",tempDir,DIR_SEPARATOR_STR));

  // compile
  logAppend("building ROM...");
  int pid=taExec(wlaspcPath.c_str(),compileFlags,NULL);
  if (pid<0) {
    logAppend(fmt::sprintf("ERROR: failed to start wla-spc700! (%s)",strerror(errno)));
    failed=true;
    running=false;
    return;
  }
  // wait for compiler
  int exitCode=taWaitProcess(pid);
  if (exitCode!=0) {
    logAppend(fmt::sprintf("ERROR: process exited with code %d!",exitCode));
    failed=true;
    running=false;
    return;
  }

  logAppend("linking ROM...");
  pid=taExec(wlalinkPath.c_str(),{
    "-v",
    fmt::sprintf("%s%sspc.ini",tempDir,DIR_SEPARATOR_STR),
    romPath
  },NULL);
  if (pid<0) {
    logAppend(fmt::sprintf("ERROR: failed to start wlalink! (%s)",strerror(errno)));
    failed=true;
    running=false;
    return;
  }
  // wait for compiler
  exitCode=taWaitProcess(pid);
  if (exitCode!=0) {
    logAppend(fmt::sprintf("ERROR: process exited with code %d!",exitCode));
    failed=true;
    running=false;
    return;
  }

  // read the result
  f=ps_fopen(romPath.c_str(),"rb");
  if (f==NULL) {
    logAppend(fmt::sprintf("ERROR: couldn't open ROM for reading! (%s)",strerror(errno)));
    failed=true;
    running=false;
    return;
  }
  unsigned char readBuf[4096];
  size_t readBufLen=0;

  w=new SafeWriter;
  w->init();

  while (!feof(f)) {
    readBufLen=fread(readBuf,1,4096,f);
    if (ferror(f)) {
      logAppend(fmt::sprintf("ERROR: while reading ROM (%s)",strerror(errno)));
      break;
    }
    w->write(readBuf,readBufLen);
  }
  fclose(f);
  output.push_back(DivROMExportOutput("export.spc",w));

  // remove temporary files
  logAppend("cleaning up...");
#ifdef _WIN32
  WIN32_FIND_DATAW de;
  String tempDirGlob=tempDir;
  tempDirGlob+=DIR_SEPARATOR_STR;
  tempDirGlob+="*";
  HANDLE tempDirD=FindFirstFileW(utf8To16(tempDirGlob.c_str()).c_str(),&de);
  if (tempDirD!=INVALID_HANDLE_VALUE) {
    do {
      if (wcscmp(de.cFileName,L".")==0) continue;
      if (wcscmp(de.cFileName,L"..")==0) continue;
      String delPath=tempDir+DIR_SEPARATOR_STR+utf16To8(de.cFileName);
      WString delPathW=utf8To16(delPath);
      if (!DeleteFileW(delPathW.c_str())) {
        logAppend(fmt::sprintf("could not remove %s! (%x)",delPath,GetLastError()));
      }
    } while (FindNextFileW(tempDirD,&de)!=0);
    FindClose(tempDirD);
  } else {
    logAppend(fmt::sprintf("could not open dir %s! (%x)",tempDir,GetLastError()));
  }
  if (!RemoveDirectoryW(utf8To16(tempDir.c_str()).c_str())) {
    logAppend(fmt::sprintf("could not remove %s! (%x)",tempDir,GetLastError()));
  }
#else
  DIR* tempDirD=opendir(tempDir.c_str());
  if (tempDirD!=NULL) {
    struct dirent* de;
    while ((de=readdir(tempDirD))!=NULL) {
      if (strcmp(de->d_name,".")==0) continue;
      if (strcmp(de->d_name,"..")==0) continue;
      String delPath=tempDir+DIR_SEPARATOR_STR+de->d_name;
      if (unlink(delPath.c_str())<0) {
        logAppend(fmt::sprintf("could not remove %s! (%s)",delPath,strerror(errno)));
      }
    }
    closedir(tempDirD);
  } else {
    logAppend(fmt::sprintf("could not open dir %s! (%s)",tempDir,strerror(errno)));
  }
  if (rmdir(tempDir.c_str())<0) {
    logAppend(fmt::sprintf("could not remove %s! (%s)",tempDir,strerror(errno)));
  }
#endif

  logAppend("finished!");

  running=false;
}

bool DivExportSNES::go(DivEngine* eng) {
  progress[0].name="Progress";
  progress[0].amount=0.0f;

  e=eng;
  running=true;
  failed=false;
  mustAbort=false;
  exportThread=new std::thread(&DivExportSNES::run,this);
  return true;
}

void DivExportSNES::wait() {
  if (exportThread!=NULL) {
    exportThread->join();
    delete exportThread;
  }
}

void DivExportSNES::abort() {
  mustAbort=true;
  wait();
}

bool DivExportSNES::isRunning() {
  return running;
}

bool DivExportSNES::hasFailed() {
  return failed;
}

DivROMExportProgress DivExportSNES::getProgress(int index) {
  if (index<0 || index>1) return progress[1];
  return progress[index];
}
