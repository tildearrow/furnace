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
#include "../baseutils.h"
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

bool DivConfig::loadFromBase64(const char* buf) {
  String data=taDecodeBase64(buf);
  return loadFromMemory(data.c_str());
}

bool DivConfig::getBool(String key, bool fallback) const {
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

int DivConfig::getInt(String key, int fallback) const {
  try {
    String val=conf.at(key);
    int ret=std::stoi(val);
    return ret;
  } catch (std::out_of_range& e) {
  } catch (std::invalid_argument& e) {
  }
  return fallback;
}

float DivConfig::getFloat(String key, float fallback) const {
  try {
    String val=conf.at(key);
    float ret=std::stof(val);
    return ret;
  } catch (std::out_of_range& e) {
  } catch (std::invalid_argument& e) {
  }
  return fallback;
}

double DivConfig::getDouble(String key, double fallback) const {
  try {
    String val=conf.at(key);
    double ret=std::stod(val);
    return ret;
  } catch (std::out_of_range& e) {
  } catch (std::invalid_argument& e) {
  }
  return fallback;
}

String DivConfig::getString(String key, String fallback) const {
  try {
    String val=conf.at(key);
    return val;
  } catch (std::out_of_range& e) {
  }
  return fallback;
}

bool DivConfig::has(String key) {
  try {
    String test=conf.at(key);
  } catch (std::out_of_range& e) {
    return false;
  }
  return true;
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

bool DivConfig::remove(String key) {
  return conf.erase(key);
}

void DivConfig::clear() {
  conf.clear();
}
