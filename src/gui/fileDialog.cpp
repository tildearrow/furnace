#include "fileDialog.h"
#include "ImGuiFileDialog.h"
#include "../ta-log.h"

#ifdef USE_NFD
#include <nfd.h>
#elif defined(ANDROID)
#include <SDL.h>
#else
#include "../../extern/pfd-fixed/portable-file-dialogs.h"
#endif

#ifdef USE_NFD
struct NFDState {
  bool isSave, allowMultiple;
  String header;
  std::vector<String> filter;
  String path;
  FileDialogSelectCallback clickCallback;
  NFDState(bool save, String h, std::vector<String> filt, String pa, FileDialogSelectCallback cc, bool multi):
    isSave(save),
    allowMultiple(multi),
    header(h),
    filter(filt),
    path(pa),
    clickCallback(cc) {
  }
};

// TODO: filter
void _nfdThread(const NFDState state, std::atomic<bool>* ok, std::vector<String>* result, bool* errorOutput) {
  nfdchar_t* out=NULL;
  nfdresult_t ret=NFD_CANCEL;
  (*errorOutput)=false;
  nfdpathset_t paths;

  result->clear();
  
  if (state.isSave) {
    ret=NFD_SaveDialog(state.filter,state.path.c_str(),&out,state.clickCallback);
  } else {
    if (state.allowMultiple) {
      ret=NFD_OpenDialogMultiple(state.filter,state.path.c_str(),&paths,state.clickCallback);
    } else {
      ret=NFD_OpenDialog(state.filter,state.path.c_str(),&out,state.clickCallback);
    }
  }

  switch (ret) {
    case NFD_OKAY:
      if (state.allowMultiple) {
        logD("pushing multi path");
        for (size_t i=0; i<NFD_PathSet_GetCount(&paths); i++) {
          result->push_back(String(NFD_PathSet_GetPath(&paths,i)));
        }
        NFD_PathSet_Free(&paths);
      } else {
        logD("pushing single path");
        if (out!=NULL) {
          logD("we have it");
          result->push_back(String(out));
        }
      }
      break;
    case NFD_CANCEL:
      break;
    case NFD_ERROR:
      logE("NFD error! %s\n",NFD_GetError());
      (*errorOutput)=true;
      break;
    default:
      logE("NFD unknown return code %d!\n",ret);
      break;
  }
  (*ok)=true;
}
#endif

bool FurnaceGUIFileDialog::openLoad(String header, std::vector<String> filter, const char* noSysFilter, String path, double dpiScale, FileDialogSelectCallback clickCallback, bool allowMultiple) {
  if (opened) return false;
  saving=false;
  curPath=path;

  // strip excess directory separators
  while (!curPath.empty()) {
    if (curPath[curPath.size()-1]!=DIR_SEPARATOR) break;
    curPath.erase(curPath.size()-1);
  }
  curPath+=DIR_SEPARATOR;

  logD("opening load file dialog with curPath %s",curPath.c_str());
  if (sysDialog) {
#ifdef USE_NFD
    dialogOK=false;
#ifdef NFD_NON_THREADED
    _nfdThread(NFDState(false,header,filter,path,clickCallback,allowMultiple),&dialogOK,&nfdResult,&hasError);
#else
    dialogO=new std::thread(_nfdThread,NFDState(false,header,filter,path,clickCallback,allowMultiple),&dialogOK,&nfdResult,&hasError);
#endif
#elif defined(ANDROID)
    hasError=false;
    if (jniEnv==NULL) {
      jniEnv=(JNIEnv*)SDL_AndroidGetJNIEnv();
      if (jniEnv==NULL) {
        hasError=true;
        logE("could not acquire JNI env!");
        return false;
      }
    }

    jobject activity=(jobject)SDL_AndroidGetActivity();
    if (activity==NULL) {
      hasError=true;
      logE("the Activity is NULL!");
      return false;
    }

    jclass class_=jniEnv->GetObjectClass(activity);
    jmethodID showFileDialog=jniEnv->GetMethodID(class_,"showFileDialog","()V");

    if (showFileDialog==NULL) {
      logE("method showFileDialog not found!");
      hasError=true;
      jniEnv->DeleteLocalRef(class_);
      jniEnv->DeleteLocalRef(activity);
      return false;
    }

    jniEnv->CallVoidMethod(activity,showFileDialog);

    /*if (!(bool)mret) {
      hasError=true;
      logW("could not open Android file picker...");
    }*/

    jniEnv->DeleteLocalRef(class_);
    jniEnv->DeleteLocalRef(activity);
    return true;
#else
    dialogO=new pfd::open_file(header,path,filter,allowMultiple?(pfd::opt::multiselect):(pfd::opt::none));
    hasError=!pfd::settings::available();
#endif
  } else {
    hasError=false;

#ifdef ANDROID
    if (!SDL_AndroidRequestPermission("android.permission.READ_EXTERNAL_STORAGE")) {
      return false;
    }
#endif

    ImGuiFileDialog::Instance()->singleClickSel=mobileUI;
    ImGuiFileDialog::Instance()->DpiScale=dpiScale;
    ImGuiFileDialog::Instance()->mobileMode=mobileUI;
    ImGuiFileDialog::Instance()->OpenModal("FileDialog",header,noSysFilter,path,allowMultiple?999:1,nullptr,0,clickCallback);
  }
  opened=true;
  return true;
}

bool FurnaceGUIFileDialog::openSave(String header, std::vector<String> filter, const char* noSysFilter, String path, double dpiScale) {
  if (opened) return false;

#ifdef ANDROID
    if (!SDL_AndroidRequestPermission("android.permission.WRITE_EXTERNAL_STORAGE")) {
      return false;
    }
#endif

  saving=true;
  curPath=path;

  // strip excess directory separators
  while (!curPath.empty()) {
    if (curPath[curPath.size()-1]!=DIR_SEPARATOR) break;
    curPath.erase(curPath.size()-1);
  }
  curPath+=DIR_SEPARATOR;

  logD("opening save file dialog with curPath %s",curPath.c_str());
  if (sysDialog) {
#ifdef USE_NFD
    dialogOK=false;
#ifdef NFD_NON_THREADED
    _nfdThread(NFDState(true,header,filter,path,NULL,false),&dialogOK,&nfdResult,&hasError);
#else
    dialogS=new std::thread(_nfdThread,NFDState(true,header,filter,path,NULL,false),&dialogOK,&nfdResult,&hasError);
#endif
#elif defined(ANDROID)
    hasError=false;
    if (jniEnv==NULL) {
      jniEnv=(JNIEnv*)SDL_AndroidGetJNIEnv();
      if (jniEnv==NULL) {
        hasError=true;
        logE("could not acquire JNI env!");
        return false;
      }
    }

    jobject activity=(jobject)SDL_AndroidGetActivity();
    if (activity==NULL) {
      hasError=true;
      logE("the Activity is NULL!");
      return false;
    }

    jclass class_=jniEnv->GetObjectClass(activity);
    jmethodID showSaveFileDialog=jniEnv->GetMethodID(class_,"showSaveFileDialog","()V");

    if (showSaveFileDialog==NULL) {
      logE("method showSaveFileDialog not found!");
      hasError=true;
      jniEnv->DeleteLocalRef(class_);
      jniEnv->DeleteLocalRef(activity);
      return false;
    }

    jniEnv->CallVoidMethod(activity,showSaveFileDialog);

    /*if (!(bool)mret) {
      hasError=true;
      logW("could not open Android file picker...");
    }*/

    jniEnv->DeleteLocalRef(class_);
    jniEnv->DeleteLocalRef(activity);
    return true;
#else
    dialogS=new pfd::save_file(header,path,filter);
    hasError=!pfd::settings::available();
#endif
  } else {
    hasError=false;

    ImGuiFileDialog::Instance()->singleClickSel=false;
    ImGuiFileDialog::Instance()->DpiScale=dpiScale;
    ImGuiFileDialog::Instance()->mobileMode=mobileUI;
    ImGuiFileDialog::Instance()->OpenModal("FileDialog",header,noSysFilter,path,1,nullptr,ImGuiFileDialogFlags_ConfirmOverwrite);
  }
  opened=true;
  return true;
}

bool FurnaceGUIFileDialog::accepted() {
  if (sysDialog) {
    return (!fileName.empty());
  } else {
    return ImGuiFileDialog::Instance()->IsOk();
  }
}

void FurnaceGUIFileDialog::close() {
  if (sysDialog) {
    if (saving) {
      if (dialogS!=NULL) {
#ifdef USE_NFD
        dialogS->join();
#endif
#ifndef ANDROID
        delete dialogS;
#endif
        dialogS=NULL;
      }
    } else {
      if (dialogO!=NULL) {
#ifdef USE_NFD
        dialogO->join();
#endif
#ifndef ANDROID
        delete dialogO;
#endif
        dialogO=NULL;
      }
    }
#ifdef USE_NFD
    dialogOK=false;
#endif
  } else {
    ImGuiFileDialog::Instance()->Close();
  }
  opened=false;
}

bool FurnaceGUIFileDialog::render(const ImVec2& min, const ImVec2& max) {
  if (sysDialog) {
#ifdef USE_NFD
    if (dialogOK) {
      fileName.clear();
      fileName=nfdResult;
      if (!fileName.empty()) {
        size_t dsPos=fileName[0].rfind(DIR_SEPARATOR);
        if (dsPos!=String::npos) curPath=fileName[0].substr(0,dsPos);
      }
      for (String& i: fileName) {
        logD("- returning %s",i);
      }
      dialogOK=false;
      return true;
    }
    return false;
#elif defined(ANDROID)
    // TODO: detect when file picker is closed
    return false;
#else
    if (saving) {
      if (dialogS!=NULL) {
        if (dialogS->ready(0)) {
          fileName.clear();
          fileName.push_back(dialogS->result());
          size_t dsPos=fileName[0].rfind(DIR_SEPARATOR);
          if (dsPos!=String::npos) curPath=fileName[0].substr(0,dsPos);
          logD("returning %s",fileName[0]);
          return true;
        }
      }
    } else {
      if (dialogO!=NULL) {
        if (dialogO->ready(0)) {
          if (dialogO->result().empty()) {
            fileName.clear();
            logD("returning nothing");
          } else {
            fileName=dialogO->result();
            if (fileName.empty()) {
              // don't touch
            } else {
              size_t dsPos=fileName[0].rfind(DIR_SEPARATOR);
              if (dsPos!=String::npos) curPath=fileName[0].substr(0,dsPos);
              for (String& i: fileName) {
                logD("- returning %s",i);
              }
            }
          }
          return true;
        }
      }
    }
    return false;
#endif
  } else {
    return ImGuiFileDialog::Instance()->Display("FileDialog",ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoMove,min,max);
  }
}

bool FurnaceGUIFileDialog::isOpen() {
  return opened;
}

bool FurnaceGUIFileDialog::isError() {
  return hasError;
}

String FurnaceGUIFileDialog::getPath() {
  if (sysDialog) {
    if (curPath.size()>1) {
      if (curPath[curPath.size()-1]==DIR_SEPARATOR) {
        curPath=curPath.substr(0,curPath.size()-1);
      }
    }
    logD("curPath: %s",curPath.c_str());
    return curPath;
  } else {
    return ImGuiFileDialog::Instance()->GetCurrentPath();
  }
}

std::vector<String>& FurnaceGUIFileDialog::getFileName() {
  if (sysDialog) {
    return fileName;
  } else {
    fileName.clear();
    if (saving) {
      fileName.push_back(ImGuiFileDialog::Instance()->GetFilePathName());
    } else {
      for (auto& i: ImGuiFileDialog::Instance()->GetSelection()) {
        fileName.push_back(i.second);
      }
    }
    //
    return fileName;
  }
}
