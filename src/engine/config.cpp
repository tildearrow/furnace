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

#include "config.h"
#include "../ta-log.h"
#include "../baseutils.h"
#include "../fileutils.h"
#include <fmt/printf.h>

#define REDUNDANCY_NUM_ATTEMPTS 5
#define CHECK_BUF_SIZE 8192

bool DivConfig::save(const char* path, bool redundancy) {
  if (redundancy) {
    char oldPath[4096];
    char newPath[4096];

    if (fileExists(path)==1) {
      logD("rotating config files...");
      for (int i=4; i>=0; i--) {
        if (i>0) {
          snprintf(oldPath,4095,"%s.%d",path,i);
        } else {
          strncpy(oldPath,path,4095);
        }
        snprintf(newPath,4095,"%s.%d",path,i+1);

        if (i>=4) {
          logV("remove %s",oldPath);
          deleteFile(oldPath);
        } else {
          logV("move %s to %s",oldPath,newPath);
          moveFiles(oldPath,newPath);
        }
      }
    }
  }
  logD("opening config for write: %s",path);
  FILE* f=ps_fopen(path,"wb");
  if (f==NULL) {
    logW("could not write config file! %s",strerror(errno));
    reportError(fmt::sprintf("could not write config file! %s",strerror(errno)));
    return false;
  }
  for (auto& i: conf) {
    String toWrite=fmt::sprintf("%s=%s\n",i.first,i.second);
    if (fwrite(toWrite.c_str(),1,toWrite.size(),f)!=toWrite.size()) {
      logW("could not write config file! %s",strerror(errno));
      reportError(fmt::sprintf("could not write config file! %s",strerror(errno)));
      logV("removing config file");
      fclose(f);
      deleteFile(path);
      return false;
    }
  }
  fclose(f);
  logD("config file written successfully.");
  return true;
}

String DivConfig::toString() {
  String ret;
  for (auto& i: conf) {
    ret+=fmt::sprintf("%s=%s\n",i.first,i.second);
  }
  return ret;
}

String DivConfig::toBase64() {
  String data=toString();
  return taEncodeBase64(data);
}

const std::map<String,String>& DivConfig::configMap() {
  return conf;
}

void DivConfig::parseLine(const char* line) {
  String key="";
  String value="";
  bool keyOrValue=false;
  for (const char* i=line; *i; i++) {
    if (*i=='\r') continue;
    if (*i=='\n') continue;
    if (keyOrValue) {
      value+=*i;
    } else {
      if (*i=='=') {
        keyOrValue=true;
      } else {
        key+=*i;
      }
    }
  }
  if (keyOrValue) {
    conf[key]=value;
  }
}

bool DivConfig::loadFromFile(const char* path, bool createOnFail, bool redundancy) {
  char line[4096];
  String lineStr;
  lineStr.reserve(4096);
  logD("opening config for read: %s",path);

  FILE* f=NULL;

  if (redundancy) {
    unsigned char* readBuf=new unsigned char[CHECK_BUF_SIZE];
    size_t readBufLen=0;
    for (int i=0; i<REDUNDANCY_NUM_ATTEMPTS; i++) {
      bool viable=false;
      if (i>0) {
        snprintf(line,4095,"%s.%d",path,i);
      } else {
        strncpy(line,path,4095);
      }
      logV("trying: %s",line);

      // try to open config
      f=ps_fopen(line,"rb");
      // check whether we could open it
      if (f==NULL) {
        logV("fopen(): %s",strerror(errno));
        continue;
      }

      // check whether there's something
      while (!feof(f)) {
        readBufLen=fread(readBuf,1,CHECK_BUF_SIZE,f);
        if (ferror(f)) {
          logV("fread(): %s",strerror(errno));
          break;
        }

        for (size_t j=0; j<readBufLen; j++) {
          if (readBuf[j]==0) {
            viable=false;
            logW("a zero?");
            break;
          }
          if (readBuf[j]!='\r' && readBuf[j]!='\n' && readBuf[j]!=' ') {
            viable=true;
          }
        }

        if (viable) break;
      }

      // there's something
      if (viable) {
        if (fseek(f,0,SEEK_SET)==-1) {
          logV("fseek(): %s",strerror(errno));
          viable=false;
        } else {
          break;
        }
      }
      
      // close it (because there's nothing)
      fclose(f);
      f=NULL;
    }
    delete[] readBuf;

    // we couldn't read at all
    if (f==NULL) {
      logD("config does not exist");
      if (createOnFail) {
        logI("creating default config.");
        //reportError(fmt::sprintf("Creating default config: %s",strerror(errno)));
        return save(path,redundancy);
      } else {
        reportError(fmt::sprintf("COULD NOT LOAD CONFIG %s",strerror(errno)));
        return false;
      }
    }
  } else {
    f=ps_fopen(path,"rb");
    if (f==NULL) {
      logD("config does not exist");
      if (createOnFail) {
        logI("creating default config.");
        //reportError(fmt::sprintf("Creating default config: %s",strerror(errno)));
        return save(path);
      } else {
        reportError(fmt::sprintf("COULD NOT LOAD CONFIG %s",strerror(errno)));
        return false;
      }
    }
  }


  logI("loading config.");
  while (!feof(f)) {
    if (fgets(line,4095,f)==NULL) {
      break;
    }
    lineStr+=line;
    if (!lineStr.empty() && !feof(f)) {
      if (lineStr[lineStr.size()-1]!='\n') {
        continue;
      }
    }
    parseLine(lineStr.c_str());
    lineStr="";
    lineStr.reserve(4096);
  }
  logD("end of file (%s)",strerror(errno));
  fclose(f);
  return true;
}

bool DivConfig::loadFromMemory(const char* buf) {
  String line;
  const char* readPos=buf;
  while (*readPos) {
    line+=*readPos;
    readPos++;
    if ((*readPos)=='\n' || (*readPos)==0) {
      parseLine(line.c_str());
      line="";
    }
  }
  return true;
}

bool DivConfig::loadFromBase64(const char* buf) {
  String data=taDecodeBase64(buf);
  return loadFromMemory(data.c_str());
}

bool DivConfig::getBool(String key, bool fallback) const {
  auto val=conf.find(key);
  if (val!=conf.cend()) {
    if (val->second=="true") {
      return true;
    } else if (val->second=="false") {
      return false;
    } else {
      try {
        int ret=std::stoi(val->second);
        return (ret!=0);
      } catch (std::out_of_range& e) {
      } catch (std::invalid_argument& e) {
      }
    }
  }
  return fallback;
}

int DivConfig::getInt(String key, int fallback) const {
  auto val=conf.find(key);
  if (val!=conf.cend()) {
    try {
      int ret=std::stoi(val->second);
      return ret;
    } catch (std::out_of_range& e) {
    } catch (std::invalid_argument& e) {
    }
  }
  return fallback;
}

float DivConfig::getFloat(String key, float fallback) const {
  auto val=conf.find(key);
  if (val!=conf.cend()) {
    try {
      float ret=std::stof(val->second);
      return ret;
    } catch (std::out_of_range& e) {
    } catch (std::invalid_argument& e) {
    }
  }
  return fallback;
}

double DivConfig::getDouble(String key, double fallback) const {
  auto val=conf.find(key);
  if (val!=conf.cend()) {
    try {
      double ret=std::stod(val->second);
      return ret;
    } catch (std::out_of_range& e) {
    } catch (std::invalid_argument& e) {
    }
  }
  return fallback;
}

String DivConfig::getString(String key, String fallback) const {
  auto val=conf.find(key);
  if (val!=conf.cend()) {
    return val->second;
  }
  return fallback;
}

std::vector<int> DivConfig::getIntList(String key, std::initializer_list<int> fallback) const {
  String next;
  std::vector<int> ret;
  auto val=conf.find(key);
  if (val!=conf.cend()) {
    try {
      for (char i: val->second) {
        if (i==',') {
          int num=std::stoi(next);
          ret.push_back(num);
          next="";
        } else {
          next+=i;
        }
      }
      if (!next.empty()) {
        int num=std::stoi(next);
        ret.push_back(num);
      }

      return ret;
    } catch (std::out_of_range& e) {
    } catch (std::invalid_argument& e) {
    }
  }
  return fallback;
}

bool DivConfig::has(String key) const {
  auto val=conf.find(key);
  return (val!=conf.cend());
}

void DivConfig::set(String key, bool value) {
  if (value) {
    conf[key]="true";
  } else {
    conf[key]="false";
  }
}

void DivConfig::set(String key, int value) {
  conf[key]=fmt::sprintf("%d",value);
}

void DivConfig::set(String key, float value) {
  conf[key]=fmt::sprintf("%f",value);
}

void DivConfig::set(String key, double value) {
  conf[key]=fmt::sprintf("%f",value);
}

void DivConfig::set(String key, const char* value) {
  conf[key]=String(value);
}

void DivConfig::set(String key, String value) {
  conf[key]=value;
}

void DivConfig::set(String key, const std::vector<int>& value) {
  String val;
  bool comma=false;
  for (int i: value) {
    if (comma) val+=',';
    val+=fmt::sprintf("%d",i);
    comma=true;
  }
  conf[key]=val;
}

bool DivConfig::remove(String key) {
  return conf.erase(key);
}

void DivConfig::clear() {
  conf.clear();
}
