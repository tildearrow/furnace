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

#ifdef _WIN32
#include "winStuff.h"
#define CONFIG_FILE "\\furnace.cfg"
#define LOG_FILE "\\furnace.log"
#else
#ifdef __HAIKU__
#include <support/SupportDefs.h>
#include <storage/FindDirectory.h>
#endif
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#define CONFIG_FILE "/furnace.cfg"
#endif

#ifdef IS_MOBILE
#ifdef HAVE_SDL2
#include <SDL.h>
#else
#error "Furnace mobile requires SDL2!"
#endif
#endif

void DivEngine::initConfDir() {
#ifdef _WIN32
  // maybe move this function in here instead?
  configPath=getWinConfigPath();
#elif defined(IS_MOBILE)
  configPath=SDL_GetPrefPath("tildearrow","furnace");
#else
#ifdef __HAIKU__
  char userSettingsDir[PATH_MAX];
  status_t findUserDir = find_directory(B_USER_SETTINGS_DIRECTORY,0,true,userSettingsDir,PATH_MAX);
  if (findUserDir==B_OK) {
    configPath=userSettingsDir;
  } else {
    logW("unable to find/create user settings directory (%s)!",strerror(findUserDir));
    configPath=".";
    return;
  }
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
    } else {
      configPath=entry->pw_dir;
    }
  } else {
    configPath=home;
  }
#ifdef __APPLE__
  configPath+="/Library/Application Support";
#else
  // FIXME this doesn't honour XDG_CONFIG_HOME *at all*
  configPath+="/.config";
#endif // __APPLE__
#endif // __HAIKU__
#ifdef __APPLE__
  configPath+="/Furnace";
#else
  configPath+="/furnace";
#endif // __APPLE__
  struct stat st;
  std::string pathSep="/";
  configPath+=pathSep;
  size_t sepPos=configPath.find(pathSep,1);
  while (sepPos!=std::string::npos) {
    std::string subpath=configPath.substr(0,sepPos++);
    if (stat(subpath.c_str(),&st)!=0) {
      logI("creating config path element %s ...",subpath.c_str());
      if (mkdir(subpath.c_str(),0755)!=0) {
        logW("could not create config path element %s! (%s)",subpath.c_str(),strerror(errno));
        configPath=".";
        return;
      }
    }
    sepPos=configPath.find(pathSep,sepPos);
  }
  configPath.resize(configPath.length()-pathSep.length());
#endif // _WIN32
}

bool DivEngine::saveConf() {
  configFile=configPath+String(CONFIG_FILE);
  return conf.save(configFile.c_str());
}

bool DivEngine::loadConf() {
  configFile=configPath+String(CONFIG_FILE);
  return conf.loadFromFile(configFile.c_str());
}

bool DivEngine::getConfBool(String key, bool fallback) {
  return conf.getBool(key,fallback);
}

int DivEngine::getConfInt(String key, int fallback) {
  return conf.getInt(key,fallback);
}

float DivEngine::getConfFloat(String key, float fallback) {
  return conf.getFloat(key,fallback);
}

double DivEngine::getConfDouble(String key, double fallback) {
  return conf.getDouble(key,fallback);
}

String DivEngine::getConfString(String key, String fallback) {
  return conf.getString(key,fallback);
}

void DivEngine::setConf(String key, bool value) {
  conf.set(key,value);
}

void DivEngine::setConf(String key, int value) {
  conf.set(key,value);
}

void DivEngine::setConf(String key, float value) {
  conf.set(key,value);
}

void DivEngine::setConf(String key, double value) {
  conf.set(key,value);
}

void DivEngine::setConf(String key, const char* value) {
  conf.set(key,value);
}

void DivEngine::setConf(String key, String value) {
  conf.set(key,value);
}
