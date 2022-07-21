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

#include "engine.h"
#include "../ta-log.h"
#include "../fileutils.h"
#include <fmt/printf.h>

#ifdef _WIN32
#include "winStuff.h"
#define CONFIG_FILE "\\furnace.cfg"
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#define CONFIG_FILE "/furnace.cfg"
#endif

void DivEngine::initConfDir() {
#ifdef _WIN32
  // maybe move this function in here instead?
  configPath=getWinConfigPath();
#elif defined (IS_MOBILE)
  configPath=SDL_GetPrefPath();
#else
  // TODO this should check XDG_CONFIG_HOME first
  char* home=getenv("HOME");
  if (home==NULL) {
    int uid=getuid();
    struct passwd* entry=getpwuid(uid);
    if (entry==NULL) {
      logW("unable to determine home directory (%s)!",strerror(errno));
      configPath=".";
      return;
    }
  } else {
    configPath=home;
  }
#  ifdef __APPLE__
  configPath+="/Library/Application Support/Furnace";
#  elif defined (__HAIKU__)
  configPath+="/config/settings/furnace";
#  else
  // FIXME this doesn't honour XDG_CONFIG_HOME *at all*
  configPath+="/.config/furnace";
#  endif
  struct stat st;
  std::string pathSep="/";
  configPath+=pathSep;
  size_t sepPos=configPath.find(pathSep,1);
  while (sepPos!=std::string::npos) {
    std::string subpath=configPath.substr(0,sepPos++);
    logI("checking config path component %s ...",subpath.c_str());
    if (stat(subpath.c_str(),&st)!=0) {
      logI("creating config path component %s ...",subpath.c_str());
      if (mkdir(subpath.c_str(),0755)!=0) {
        logW("could not create config path component %s! (%s)",subpath.c_str(),strerror(errno));
        configPath=".";
        return;
      }
    }
    sepPos=configPath.find(pathSep,sepPos);
  }
  configPath.resize(configPath.length()-pathSep.length());
#endif
}

bool DivEngine::saveConf() {
  configFile=configPath+String(CONFIG_FILE);
  FILE* f=ps_fopen(configFile.c_str(),"wb");
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

bool DivEngine::loadConf() {
  char line[4096];
  configFile=configPath+String(CONFIG_FILE);
  FILE* f=ps_fopen(configFile.c_str(),"rb");
  if (f==NULL) {
    logI("creating default config.");
    return saveConf();
  }
  logI("loading config.");
  while (!feof(f)) {
    String key="";
    String value="";
    bool keyOrValue=false;
    if (fgets(line,4095,f)==NULL) {
      break;
    }
    for (char* i=line; *i; i++) {
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
  fclose(f);
  return true;
}

bool DivEngine::getConfBool(String key, bool fallback) {
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

int DivEngine::getConfInt(String key, int fallback) {
  try {
    String val=conf.at(key);
    int ret=std::stoi(val);
    return ret;
  } catch (std::out_of_range& e) {
  } catch (std::invalid_argument& e) {
  }
  return fallback;
}

float DivEngine::getConfFloat(String key, float fallback) {
  try {
    String val=conf.at(key);
    float ret=std::stof(val);
    return ret;
  } catch (std::out_of_range& e) {
  } catch (std::invalid_argument& e) {
  }
  return fallback;
}

double DivEngine::getConfDouble(String key, double fallback) {
  try {
    String val=conf.at(key);
    double ret=std::stod(val);
    return ret;
  } catch (std::out_of_range& e) {
  } catch (std::invalid_argument& e) {
  }
  return fallback;
}

String DivEngine::getConfString(String key, String fallback) {
  try {
    String val=conf.at(key);
    return val;
  } catch (std::out_of_range& e) {
  }
  return fallback;
}

void DivEngine::setConf(String key, bool value) {
  if (value) {
    conf[key]="true";
  } else {
    conf[key]="false";
  }
}

void DivEngine::setConf(String key, int value) {
  conf[key]=fmt::sprintf("%d",value);
}

void DivEngine::setConf(String key, float value) {
  conf[key]=fmt::sprintf("%f",value);
}

void DivEngine::setConf(String key, double value) {
  conf[key]=fmt::sprintf("%f",value);
}

void DivEngine::setConf(String key, String value) {
  conf[key]=value;
}
