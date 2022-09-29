/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
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
#include "../fileutils.h"
#include <fmt/printf.h>

bool DivConfig::save(const char* path) {
  FILE* f=ps_fopen(path,"wb");
  if (f==NULL) {
    logW("could not write config file! %s",strerror(errno));
    return false;
  }
  for (auto& i: conf) {
    String toWrite=fmt::sprintf("%s=%s\n",i.first,i.second);
    if (fwrite(toWrite.c_str(),1,toWrite.size(),f)!=toWrite.size()) {
      logW("could not write config file! %s",strerror(errno));
      fclose(f);
      return false;
    }
  }
  fclose(f);
  return true;
}

String DivConfig::toString() {
  String ret;
  for (auto& i: conf) {
    ret+=fmt::sprintf("%s=%s\n",i.first,i.second);
  }
  return ret;
}

void DivConfig::parseLine(const char* line) {
  String key="";
  String value="";
  bool keyOrValue=false;
  for (const char* i=line; *i; i++) {
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

bool DivConfig::loadFromFile(const char* path, bool createOnFail) {
  char line[4096];
  FILE* f=ps_fopen(path,"rb");
  if (f==NULL) {
    if (createOnFail) {
      logI("creating default config.");
      return save(path);
    } else {
      return false;
    }
  }
  logI("loading config.");
  while (!feof(f)) {
    if (fgets(line,4095,f)==NULL) {
      break;
    }
    parseLine(line);
  }
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

bool DivConfig::getConfBool(String key, bool fallback) {
  try {
    String val=conf.at(key);
    if (val=="true") {
      return true;
    } else if (val=="false") {
      return false;
    }
  } catch (std::out_of_range& e) {
  }
  return fallback;
}

int DivConfig::getConfInt(String key, int fallback) {
  try {
    String val=conf.at(key);
    int ret=std::stoi(val);
    return ret;
  } catch (std::out_of_range& e) {
  } catch (std::invalid_argument& e) {
  }
  return fallback;
}

float DivConfig::getConfFloat(String key, float fallback) {
  try {
    String val=conf.at(key);
    float ret=std::stof(val);
    return ret;
  } catch (std::out_of_range& e) {
  } catch (std::invalid_argument& e) {
  }
  return fallback;
}

double DivConfig::getConfDouble(String key, double fallback) {
  try {
    String val=conf.at(key);
    double ret=std::stod(val);
    return ret;
  } catch (std::out_of_range& e) {
  } catch (std::invalid_argument& e) {
  }
  return fallback;
}

String DivConfig::getConfString(String key, String fallback) {
  try {
    String val=conf.at(key);
    return val;
  } catch (std::out_of_range& e) {
  }
  return fallback;
}

void DivConfig::setConf(String key, bool value) {
  if (value) {
    conf[key]="true";
  } else {
    conf[key]="false";
  }
}

void DivConfig::setConf(String key, int value) {
  conf[key]=fmt::sprintf("%d",value);
}

void DivConfig::setConf(String key, float value) {
  conf[key]=fmt::sprintf("%f",value);
}

void DivConfig::setConf(String key, double value) {
  conf[key]=fmt::sprintf("%f",value);
}

void DivConfig::setConf(String key, const char* value) {
  conf[key]=String(value);
}

void DivConfig::setConf(String key, String value) {
  conf[key]=value;
}
