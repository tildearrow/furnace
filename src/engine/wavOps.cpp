/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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
#include "../stringutils.h"
#include "../subprocess.h"

#ifndef _WIN32
#include <fcntl.h>
#include <unistd.h>
#endif

#ifdef HAVE_SNDFILE
#include "sfWrapper.h"
#endif

#define EXPORT_BUFSIZE 2048

void _runExportThread(DivEngine* caller) {
  caller->runExportThread();
}

bool DivEngine::isExporting() {
  return exporting;
}

void DivEngine::getLoopsLeft(int &loops) {
  if (totalLoops<0 || exportLoopCount==0) {
    loops=0;
    return;
  }
  loops=exportLoopCount-1-totalLoops;
}

void DivEngine::getTotalLoops(int &loops) {
  loops=exportLoopCount-1;
}

void DivEngine::getCurSongPos(int &row, int &order) {
  row=curRow;
  order=curOrder;
}

void DivEngine::getTotalAudioFiles(int &files) {
  files=0;

  switch (exportMode) {
    case DIV_EXPORT_MODE_ONE: {
      files=1;
      break;
    }
    case DIV_EXPORT_MODE_MANY_SYS: {
      files=1; // there actually are several files but they are processed in the same loop, so to correctly draw progress we think of them as one file
      break;
    }
    case DIV_EXPORT_MODE_MANY_CHAN: {
      for (int i=0; i<chans; i++) {
        if (!exportChannelMask[i]) continue;

        files++;

        if (getChannelType(i)==5) {
          i++;
          while (true) {
            if (i>=chans) break;
            if (getChannelType(i)!=5) break;
            i++;
          }
          i--;
        }
      }
      break;
    }
    default:
      break;
  }
}

void DivEngine::getCurFileIndex(int &file) {
  file=0;

  switch (exportMode) {
    case DIV_EXPORT_MODE_ONE: {
      file=0;
      break;
    }
    case DIV_EXPORT_MODE_MANY_SYS: {
      file=0; // there actually are several files but they are processed in the same loop, so to correctly draw progress we think of them as one file
      break;
    }
    case DIV_EXPORT_MODE_MANY_CHAN: {
      file=curExportChan;
      break;
    }
    default:
      break;
  }
}

bool DivEngine::getIsFadingOut() {
  return isFadingOut;
}

class SndfileWavWriter {
  private:
    SNDFILE* sf;
    SF_INFO si;
    SFWrapper sfWrap;
  public:
    SndfileWavWriter(): sf(NULL) {}

    bool open(SF_INFO info, const char* exportPath) {
      si=info;
      sf=sfWrap.doOpen(exportPath,SFM_WRITE,&si);
      if (sf==NULL) {
        return false;
      }

      return true;
    }

    void close() {
      if (sfWrap.doClose()!=0) {
        logE("could not close audio file!");
      }
    }

    inline int getChannelCount() {
      return si.channels;
    }

    bool write(float* samples, sf_count_t count, size_t exportOutputs) {
      // FIXME: we're ignoring exportOutputs here. this is hacky!
      return sf_writef_float(sf,samples,count)==count;
    }

    bool writeShort(short* samples, sf_count_t count) {
      return sf_writef_short(sf,samples,count)==count;
    }
};

#ifndef _WIN32
class ProcWriter {
  private:
    Subprocess* proc;
    int writeFd;
    int format;
    int channelCount;
  public:
    ProcWriter(int channelCount):
      proc(NULL),
      writeFd(-1),
      channelCount(channelCount) {}

    bool open(Subprocess* proc_, int writeFd_, int format_) {
      proc=proc_;
      writeFd=writeFd_;
      format=format_;

      int flags=fcntl(writeFd,F_GETFL);
      if (flags==-1) {
        logE("error with fcntl (%s)",strerror(errno));
        return false;
      }
      flags|=O_NONBLOCK;
      if (fcntl(writeFd,F_SETFL,flags)==-1) {
        logE("error with fcntl (%s)",strerror(errno));
        return false;
      }

      return true;
    }

    void close() {
      ::close(writeFd);
    }

    inline int getChannelCount() {
      return channelCount;
    }

    bool write(float* samples, size_t count_, size_t exportOutputs) {
      size_t count=count_*exportOutputs;

      const auto doWrite=[this](void* buf, size_t size) {
        while (true) {
          if (::write(writeFd,buf,size)==(ssize_t)size) return true;
          // buffer got full! wait for it to unblock
          if (!proc->waitStdinOrExit()) {
            int exitCode=-1;
            proc->getExitCode(&exitCode, false);
            logE("subprocess exited before finishing export (exit code %d)", exitCode);
            return false;
          }
        }
      };

      if (format==DIV_EXPORT_FORMAT_S16) {
#ifdef TA_BIG_ENDIAN
        uint16_t buf[count];
        for (size_t i=0; i<count; i++) {
          int16_t sample=32767.0f*samples[i];
          uint16_t sampleU=*(uint16_t*)&sample;
          buf[i]=(sampleU>>8)|(sampleU<<8); // byte swap from BE to LE
        }
        return doWrite(buf,count*sizeof(uint16_t));
#else
        int16_t buf[count];
        for (size_t i=0; i<count; i++) {
          buf[i]=32767.0f*samples[i];
        }
        return doWrite(buf,count*sizeof(int16_t));
#endif
      } else if (format==DIV_EXPORT_FORMAT_F32) {
        return doWrite(samples,count*sizeof(float));
      } else {
        logE("invalid export format: %d",format);
        return false;
      }
    }

    bool writeShort(short* samples, size_t count_) {
      size_t count=count_*channelCount;

      const auto doWrite=[this](void* buf, size_t size) {
        while (true) {
          if (::write(writeFd,buf,size)==(ssize_t)size) return true;
          // buffer got full! wait for it to unblock
          if (!proc->waitStdinOrExit()) {
            int exitCode=-1;
            proc->getExitCode(&exitCode, false);
            logE("subprocess exited before finishing export (exit code %d)", exitCode);
            return false;
          }
        }
      };

      if (format==DIV_EXPORT_FORMAT_S16) {
#ifdef TA_BIG_ENDIAN
        uint16_t buf[count];
        for (size_t i=0; i<count; i++) {
          uint16_t sampleU=*(uint16_t*)&samples[i];
          buf[i]=(sampleU>>8)|(sampleU<<8); // byte swap from BE to LE
        }
        return doWrite(buf,count*sizeof(uint16_t));
#else
        return doWrite(samples,count*sizeof(int16_t));
#endif
      } else if (format==DIV_EXPORT_FORMAT_F32) {
        float buf[count];
        for (size_t i=0; i<count; i++) {
          buf[i]=(float)samples[i]/32767.0f;
        }
        return doWrite(buf,count*sizeof(float));
      } else {
        logE("invalid export format: %d",format);
        return false;
      }
    }
};
#endif

#include <iomanip>

std::vector<String> substituteArgs(const std::vector<String>& args, const std::map<String, String>& defs) {
  std::vector<String> result;
  for (const String& arg : args) {
    String finalArg;
    size_t start=0;
    while (start<arg.size()) {
      size_t end=arg.find('%',start);
      if (end==start) {
        // we found a % right where we are

        size_t secondEnd=arg.find('%',start+1);
        if (secondEnd==end+1) {
          // we got another % right after - it's an escaped %
          finalArg.push_back('%');
        } else if (secondEnd==String::npos) {
          // we would error but let's just not substitute
          finalArg.append(arg,start);
          start=end;
        } else {
          String key=arg.substr(end+1,secondEnd-(end+1));
          if (defs.find(key)!=defs.end()) {
            finalArg.append(defs.at(key));
          }
        }
        start=(secondEnd==String::npos)?(String::npos):(secondEnd+1);
      } else {
        // we found a % more ahead (or got to the end of the string)
        finalArg.append(arg,start,end-start);
        start=end;
      }
    }
    result.push_back(finalArg);
  }
  return result;
}

String vec2str(const std::vector<String>& vec) {
  String output;
  output+="[";
  for (size_t i=0; i<vec.size(); i++) {
    output+='"';
    output+=vec[i];
    output+='"';
    if (i<vec.size()-1)
      output+=", ";
  }
  output+="]";
  return output;
}

#ifdef HAVE_SNDFILE
void DivEngine::runExportThread() {
  size_t fadeOutSamples=got.rate*exportFadeOut;
  size_t curFadeOutSample=0;
  isFadingOut=false;

  const auto makeSfInfo=[this]() {
    SF_INFO si;
    si.samplerate=got.rate;
    si.channels=exportOutputs;
    switch (exportFormat) {
    case DIV_EXPORT_FORMAT_S16:
      si.format=SF_FORMAT_WAV|SF_FORMAT_PCM_16;
      break;
    case DIV_EXPORT_FORMAT_F32:
    default:
      si.format=SF_FORMAT_WAV|SF_FORMAT_FLOAT;
      break;
    }
    return si;
  };

  const auto doExport=[&fadeOutSamples,&curFadeOutSample,this](auto wr) {
    float* outBuf[DIV_MAX_OUTPUTS];
    float* outBufFinal;
    for (int i=0; i<exportOutputs; i++) {
      outBuf[i]=new float[EXPORT_BUFSIZE];
    }
    outBufFinal=new float[EXPORT_BUFSIZE*exportOutputs];

    // take control of audio output
    deinitAudioBackend();
    freelance=false;
    playSub(false);
    freelance=false;

    logI("exporting song to file...");

    while (playing) {
      size_t total=0;
      nextBuf(NULL,outBuf,0,exportOutputs,EXPORT_BUFSIZE);
      if (totalProcessed>EXPORT_BUFSIZE) {
        logE("error: total processed is bigger than export bufsize! %d>%d",totalProcessed,EXPORT_BUFSIZE);
        totalProcessed=EXPORT_BUFSIZE;
      }
      int fi=0;
      for (int i=0; i<(int)totalProcessed; i++) {
        total++;
        if (isFadingOut) {
          double mul=(1.0-((double)curFadeOutSample/(double)fadeOutSamples));
          for (int j=0; j<exportOutputs; j++) {
            outBufFinal[fi++]=MAX(-1.0f,MIN(1.0f,outBuf[j][i]))*mul;
          }
          if (++curFadeOutSample>=fadeOutSamples) {
            playing=false;
            break;
          }
        } else {
          for (int j=0; j<exportOutputs; j++) {
            outBufFinal[fi++]=MAX(-1.0f,MIN(1.0f,outBuf[j][i]));
          }
          if (lastLoopPos>-1 && i>=lastLoopPos && totalLoops>=exportLoopCount) {
            logD("start fading out...");
            isFadingOut=true;
            if (fadeOutSamples==0) break;
          }
        }
      }

      if (!wr->write(outBufFinal,total,exportOutputs)) {
        logE("error: failed to write entire buffer!");
        break;
      }
    }

    wr->close();

    delete[] outBufFinal;
    for (int i=0; i<exportOutputs; i++) {
      delete[] outBuf[i];
    }

    if (initAudioBackend()) {
      for (int i=0; i<song.systemLen; i++) {
        disCont[i].setRates(got.rate);
        disCont[i].setQuality(lowQuality,dcHiPass);
      }
      if (!output->setRun(true)) {
        logE("error while activating audio!");
      }
    }
  };

  const auto doExportManySys=[&fadeOutSamples,&curFadeOutSample,this](auto writers) {
    float* outBuf[DIV_MAX_OUTPUTS];
    memset(outBuf,0,sizeof(void*)*DIV_MAX_OUTPUTS);
    outBuf[0]=new float[EXPORT_BUFSIZE];
    outBuf[1]=new float[EXPORT_BUFSIZE];
    short* sysBuf[DIV_MAX_CHIPS];
    for (int i=0; i<song.systemLen; i++) {
      sysBuf[i]=new short[EXPORT_BUFSIZE*writers[i].getChannelCount()];
    }

    // take control of audio output
    deinitAudioBackend();
    freelance=false;
    playSub(false);
    freelance=false;

    logI("rendering to files...");

    while (playing) {
      size_t total=0;
      nextBuf(NULL,outBuf,0,2,EXPORT_BUFSIZE);
      if (totalProcessed>EXPORT_BUFSIZE) {
        logE("error: total processed is bigger than export bufsize! %d>%d",totalProcessed,EXPORT_BUFSIZE);
        totalProcessed=EXPORT_BUFSIZE;
      }
      for (int j=0; j<(int)totalProcessed; j++) {
        total++;
        if (isFadingOut) {
          double mul=(1.0-((double)curFadeOutSample/(double)fadeOutSamples));
          for (int i=0; i<song.systemLen; i++) {
            int channelCount=writers[i].getChannelCount();
            for (int k=0; k<channelCount; k++) {
              if (disCont[i].bbOut[k]==NULL) {
                sysBuf[i][k+(j*channelCount)]=0;
              } else {
                sysBuf[i][k+(j*channelCount)]=(double)disCont[i].bbOut[k][j]*mul;
              }
            }
          }
          if (++curFadeOutSample>=fadeOutSamples) {
            playing=false;
            break;
          }
        } else {
          for (int i=0; i<song.systemLen; i++) {
            int channelCount=writers[i].getChannelCount();
            for (int k=0; k<channelCount; k++) {
              if (disCont[i].bbOut[k]==NULL) {
                sysBuf[i][k+(j*channelCount)]=0;
              } else {
                sysBuf[i][k+(j*channelCount)]=disCont[i].bbOut[k][j];
              }
            }
          }
          if (lastLoopPos>-1 && j>=lastLoopPos && totalLoops>=exportLoopCount) {
            logD("start fading out...");
            isFadingOut=true;
            if (fadeOutSamples==0) break;
          }
        }
      }
      for (int i=0; i<song.systemLen; i++) {
        if (!writers[i].writeShort(sysBuf[i],total)) {
          logE("error: failed to write entire buffer! (%d)",i);
          break;
        }
      }
    }

    delete[] outBuf[0];
    delete[] outBuf[1];

    for (int i=0; i<song.systemLen; i++) {
      delete[] sysBuf[i];
      writers[i].close();
    }

    if (initAudioBackend()) {
      for (int i=0; i<song.systemLen; i++) {
        disCont[i].setRates(got.rate);
        disCont[i].setQuality(lowQuality,dcHiPass);
      }
      if (!output->setRun(true)) {
        logE("error while activating audio!");
      }
    }
  };

  const auto buildFinalArgs=[this](std::vector<String> args, String inputFormat, long int sampleRate, int channelCount, String path) {
    std::map<String, String> defMap;
    defMap["ffmpeg"]=exportFfmpegPath;
    defMap["input_format"]=inputFormat;
    defMap["sample_rate"]=fmt::sprintf("%ld",sampleRate);
    defMap["channel_count"]=fmt::sprintf("%d",channelCount);
    defMap["extra_flags"]=exportExtraFlags; // FIXME: the flags won't be split!!! whoops
    defMap["output_file"]=path;
    return substituteArgs(args,defMap);
  };

  switch (exportMode) {
    case DIV_EXPORT_MODE_ONE: {
      if (exportWriter==DIV_EXPORT_WRITER_SNDFILE) {
        SndfileWavWriter wr;
        if (!wr.open(makeSfInfo(),exportPath.c_str())) {
          logE("could not initialize export writer");
          exporting=false;
          return;
        }
        doExport(&wr);
      } else if (exportWriter==DIV_EXPORT_WRITER_COMMAND) {
#ifdef _WIN32
        logE("ffmpeg export is not yet supported");
#else
        std::vector<String> args;
        splitString(exportCommand,' ',args);
        String inputFormatArg=(exportFormat==DIV_EXPORT_FORMAT_S16)?"s16le":"f32le";
        args=buildFinalArgs(args,inputFormatArg,got.rate,exportOutputs,exportPath);

        Subprocess proc(args);
        int writeFd=proc.pipeStdin();
        if (writeFd==-1) {
          logE("failed to create stdin pipe for subprocess");
          exporting=false;
          return;
        }

        if (!proc.start()) {
          logE("failed to start ffmpeg subprocess");
          exporting=false;
          return;
        }

        ProcWriter wr(exportOutputs);
        if (!wr.open(&proc,writeFd,exportFormat)) {
          logE("could not initialize export writer");
          exporting=false;
          return;
        }
        doExport(&wr);

        // be sure we closed the write pipe to avoid stalling the child process
        proc.closeStdinPipe(false);
#endif
      }

      logI("exporting done!");
      exporting=false;
      break;
    }
    case DIV_EXPORT_MODE_MANY_SYS: {
      if (exportWriter==DIV_EXPORT_WRITER_SNDFILE) {
        SndfileWavWriter writers[DIV_MAX_CHIPS];
        for (int i=0; i<song.systemLen; i++) {
          SndfileWavWriter* writer = &writers[i];

          SF_INFO si;
          si.samplerate=got.rate;
          si.channels=disCont[i].dispatch->getOutputCount();
          si.format=SF_FORMAT_WAV|SF_FORMAT_PCM_16;

          String fname=fmt::sprintf("%s_s%02d.%s",exportPath,i+1,exportFileExtNoDot);
          logD("will open %s",fname.c_str());

          if (!writer->open(si,fname.c_str())) {
            logE("could not initialize export writer #%02d",i);
            for (int j=0; j<i; j++) {
              writers[i].close();
            }
            exporting=false;
            return;
          }
        }
        doExportManySys(writers);
      } else if (exportWriter==DIV_EXPORT_WRITER_COMMAND) {
#ifdef _WIN32
        logE("ffmpeg export is not yet supported");
#else
        std::vector<Subprocess> subprocesses;
        std::vector<ProcWriter> writers;

        std::vector<String> args;
        splitString(exportCommand,' ',args);

        for (int i=0; i<song.systemLen; i++) {
          String fname=fmt::sprintf("%s_s%02d.%s",exportPath,i+1,exportFileExtNoDot);
          int channelCount=disCont[i].dispatch->getOutputCount();
          std::vector<String> finalArgs=buildFinalArgs(args,"s16le",got.rate,channelCount,fname);

          // create subprocess
          subprocesses.emplace_back(finalArgs);
          Subprocess* proc=&subprocesses[i];

          int writeFd=proc->pipeStdin();
          if (writeFd==-1) {
            logE("failed to create stdin pipe for subprocess");
            // TODO: deinit all so far
            exporting=false;
            return;
          }

          if (!proc->start()) {
            logE("failed to start ffmpeg subprocess");
            // TODO: deinit all so far
            exporting=false;
            return;
          }

          // create writer
          writers.emplace_back(channelCount);
          ProcWriter* writer=&writers[i];

          if (!writer->open(proc,writeFd,DIV_EXPORT_FORMAT_S16)) {
            logE("could not initialize export writer");
            // TODO: deinit all so far
            exporting=false;
            return;
          }
        }

        doExportManySys((std::vector<ProcWriter>&)writers);

        for (int i=0; i<song.systemLen; i++) {
          // be sure we closed the write pipe to avoid stalling the child process
          subprocesses[i].closeStdinPipe(false);
        }
#endif
      }
      logI("done!");
      exporting=false;
      break;
    }
    case DIV_EXPORT_MODE_MANY_CHAN: {
      // take control of audio output
      deinitAudioBackend();

      curExportChan=0;

      float* outBuf[DIV_MAX_OUTPUTS];
      float* outBufFinal;
      for (int i=0; i<exportOutputs; i++) {
        outBuf[i]=new float[EXPORT_BUFSIZE];
      }
      outBufFinal=new float[EXPORT_BUFSIZE*exportOutputs];

      logI("rendering to files...");

      for (int i=0; i<chans; i++) {
        if (!exportChannelMask[i]) continue;

        SNDFILE* sf;
        SF_INFO si;
        SFWrapper sfWrap;
        String fname=fmt::sprintf("%s_c%02d.wav",exportPath,i+1);
        logI("- %s",fname.c_str());
        si.samplerate=got.rate;
        si.channels=exportOutputs;
        if (exportFormat==DIV_EXPORT_FORMAT_S16) {
          si.format=SF_FORMAT_WAV|SF_FORMAT_PCM_16;
        } else {
          si.format=SF_FORMAT_WAV|SF_FORMAT_FLOAT;
        }

        sf=sfWrap.doOpen(fname.c_str(),SFM_WRITE,&si);
        if (sf==NULL) {
          logE("could not open file for writing! (%s)",sf_strerror(NULL));
          break;
        }

        for (int j=0; j<chans; j++) {
          bool mute=(j!=i);
          isMuted[j]=mute;
        }
        if (getChannelType(i)==5) {
          for (int j=i; j<chans; j++) {
            if (getChannelType(j)!=5) break;
            isMuted[j]=false;
          }
        }
        for (int j=0; j<chans; j++) {
          if (disCont[dispatchOfChan[j]].dispatch!=NULL) {
            disCont[dispatchOfChan[j]].dispatch->muteChannel(dispatchChanOfChan[j],isMuted[j]);
          }
        }

        curOrder=0;
        prevOrder=0;
        curFadeOutSample=0;
        lastLoopPos=-1;
        totalLoops=0;
        isFadingOut=false;
        remainingLoops=-1;
        freelance=false;
        playSub(false);
        freelance=false;

        while (playing) {
          size_t total=0;
          nextBuf(NULL,outBuf,0,exportOutputs,EXPORT_BUFSIZE);
          if (totalProcessed>EXPORT_BUFSIZE) {
            logE("error: total processed is bigger than export bufsize! %d>%d",totalProcessed,EXPORT_BUFSIZE);
            totalProcessed=EXPORT_BUFSIZE;
          }
          int fi=0;
          for (int j=0; j<(int)totalProcessed; j++) {
            total++;
            if (isFadingOut) {
              double mul=(1.0-((double)curFadeOutSample/(double)fadeOutSamples));
              for (int k=0; k<exportOutputs; k++) {
                outBufFinal[fi++]=MAX(-1.0f,MIN(1.0f,outBuf[k][j]))*mul;
              }
              if (++curFadeOutSample>=fadeOutSamples) {
                playing=false;
                break;
              }
            } else {
              for (int k=0; k<exportOutputs; k++) {
                outBufFinal[fi++]=MAX(-1.0f,MIN(1.0f,outBuf[k][j]));
              }
              if (lastLoopPos>-1 && j>=lastLoopPos && totalLoops>=exportLoopCount) {
                logD("start fading out...");
                isFadingOut=true;
                if (fadeOutSamples==0) break;
              }
            }
          }
          if (sf_writef_float(sf,outBufFinal,total)!=(int)total) {
            logE("error: failed to write entire buffer!");
            break;
          }
        }

        curExportChan++;

        if (sfWrap.doClose()!=0) {
          logE("could not close audio file!");
        }

        if (getChannelType(i)==5) {
          i++;
          while (true) {
            if (i>=chans) break;
            if (getChannelType(i)!=5) break;
            i++;
          }
          i--;
        }

        if (stopExport) break;
      }

      delete[] outBufFinal;
      for (int i=0; i<exportOutputs; i++) {
        delete[] outBuf[i];
      }

      for (int i=0; i<chans; i++) {
        isMuted[i]=false;
        if (disCont[dispatchOfChan[i]].dispatch!=NULL) {
          disCont[dispatchOfChan[i]].dispatch->muteChannel(dispatchChanOfChan[i],false);
        }
      }

      if (initAudioBackend()) {
        for (int i=0; i<song.systemLen; i++) {
          disCont[i].setRates(got.rate);
          disCont[i].setQuality(lowQuality,dcHiPass);
        }
        if (!output->setRun(true)) {
          logE("error while activating audio!");
        }
      }
      logI("done!");
      exporting=false;
      curExportChan=0;
      break;
    }
  }

  stopExport=false;
}
#else
void DivEngine::runExportThread() {
}
#endif

bool DivEngine::shallSwitchCores() {
  return true;
}

bool DivEngine::saveAudio(const char* path, DivAudioExportOptions options) {
#ifndef HAVE_SNDFILE
  logE("Furnace was not compiled with libsndfile. cannot export!");
  return false;
#else
  exportPath=path;
  exportMode=options.mode;
  exportFormat=options.format;
  exportFadeOut=options.fadeOut;
  exportWriter=options.curWriter;
  exportExtraFlags=options.extraFlags;

  if (exportWriter==DIV_EXPORT_WRITER_SNDFILE) {
    exportFileExtNoDot="wav";
  } else {
    DivAudioCommandExportDef& def=options.commandExportWriterDefs[options.curCommandWriterIndex];
    exportCommand=def.commandTemplate;
    exportFileExtNoDot=def.fileExt;
  }

  memcpy(exportChannelMask,options.channelMask,DIV_MAX_CHANS*sizeof(bool));
  if (exportMode!=DIV_EXPORT_MODE_ONE) {
    removeFileExt(exportPath,exportFileExtNoDot.c_str());
  }
  exporting=true;
  stopExport=false;
  stop();
  repeatPattern=false;
  setOrder(0);
  remainingLoops=-1;
  got.rate=options.sampleRate;

  if (shallSwitchCores()) {
    bool isMutedBefore[DIV_MAX_CHANS];
    memcpy(isMutedBefore,isMuted,DIV_MAX_CHANS*sizeof(bool));
    quitDispatch();
    initDispatch(true);
    renderSamplesP();
    for (int i=0; i<chans; i++) {
      if (isMutedBefore[i]) {
        muteChannel(i,true);
      }
    }
  }

  exportOutputs=options.chans;
  if (exportOutputs<1) exportOutputs=1;
  if (exportOutputs>DIV_MAX_OUTPUTS) exportOutputs=DIV_MAX_OUTPUTS;

  exportLoopCount=options.loops+1;
  exportThread=new std::thread(_runExportThread,this);
  return true;
#endif
}

void DivEngine::waitAudioFile() {
  if (exportThread!=NULL) {
    exportThread->join();
  }
}

bool DivEngine::haltAudioFile() {
  stopExport=true;
  stop();
  waitAudioFile();
  finishAudioFile();
  return true;
}

void DivEngine::finishAudioFile() {
  if (shallSwitchCores()) {
    bool isMutedBefore[DIV_MAX_CHANS];
    memcpy(isMutedBefore,isMuted,DIV_MAX_CHANS*sizeof(bool));
    quitDispatch();
    initDispatch(false);
    renderSamplesP();
    for (int i=0; i<chans; i++) {
      if (isMutedBefore[i]) {
        muteChannel(i,true);
      }
    }
  }
}

