#include "fileDialog.h"
#include "ImGuiFileDialog.h"
#include "../ta-log.h"

#ifdef USE_NFD
#include <nfd.h>
#else
#include "../../extern/pfd-fixed/portable-file-dialogs.h"
#endif

#ifdef USE_NFD
struct NFDState {
  bool isSave;
  String header;
  std::vector<String> filter;
  String path;
  FileDialogSelectCallback clickCallback;
  NFDState(bool save, String h, std::vector<String> filt, String pa, FileDialogSelectCallback cc):
    isSave(save),
    header(h),
    filter(filt),
    path(pa),
    clickCallback(cc) {
  }
};

// TODO: filter
void _nfdThread(const NFDState state, std::atomic<bool>* ok, String* result) {
  nfdchar_t* out=NULL;
  nfdresult_t ret=NFD_CANCEL;
  
  if (state.isSave) {
    ret=NFD_SaveDialog(NULL,state.path.c_str(),&out);
  } else {
    ret=NFD_OpenDialog(NULL,state.path.c_str(),&out);
  }

  switch (ret) {
    case NFD_OKAY:
      if (out!=NULL) {
        (*result)=out;
      } else {
        (*result)="";
      }
      break;
    case NFD_CANCEL:
      (*result)="";
      break;
    case NFD_ERROR:
      (*result)="";
      logE("NFD error! %s\n",NFD_GetError());
      break;
    default:
      logE("NFD unknown return code %d!\n",ret);
      (*result)="";
      break;
  }
  (*ok)=true;
}
#endif

bool FurnaceGUIFileDialog::openLoad(String header, std::vector<String> filter, const char* noSysFilter, String path, double dpiScale, FileDialogSelectCallback clickCallback) {
  if (opened) return false;
  saving=false;
  curPath=path;
  logD("opening load file dialog with curPath %s",curPath.c_str());
  if (sysDialog) {
#ifdef USE_NFD
    dialogOK=false;
    dialogO=new std::thread(_nfdThread,NFDState(false,header,filter,path,clickCallback),&dialogOK,&nfdResult);
#else
    dialogO=new pfd::open_file(header,path,filter);
#endif
  } else {
    ImGuiFileDialog::Instance()->DpiScale=dpiScale;
    ImGuiFileDialog::Instance()->OpenModal("FileDialog",header,noSysFilter,path,1,nullptr,0,clickCallback);
  }
  opened=true;
  return true;
}

bool FurnaceGUIFileDialog::openSave(String header, std::vector<String> filter, const char* noSysFilter, String path, double dpiScale) {
  if (opened) return false;
  saving=true;
  curPath=path;
  logD("opening save file dialog with curPath %s",curPath.c_str());
  if (sysDialog) {
#ifdef USE_NFD
    dialogOK=false;
    dialogS=new std::thread(_nfdThread,NFDState(true,header,filter,path,NULL),&dialogOK,&nfdResult);
#else
    dialogS=new pfd::save_file(header,path,filter);
#endif
  } else {
    ImGuiFileDialog::Instance()->DpiScale=dpiScale;
    ImGuiFileDialog::Instance()->OpenModal("FileDialog",header,noSysFilter,path,1,nullptr,ImGuiFileDialogFlags_ConfirmOverwrite);
  }
  opened=true;
  return true;
}

bool FurnaceGUIFileDialog::accepted() {
  if (sysDialog) {
    return (fileName!="");
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
        delete dialogS;
        dialogS=NULL;
      }
    } else {
      if (dialogO!=NULL) {
#ifdef USE_NFD
        dialogO->join();
#endif
        delete dialogO;
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
      fileName=nfdResult;
      logD("returning %s",fileName.c_str());
      dialogOK=false;
      return true;
    }
    return false;
#else
    if (saving) {
      if (dialogS!=NULL) {
        if (dialogS->ready(0)) {
          fileName=dialogS->result();
          size_t dsPos=fileName.rfind(DIR_SEPARATOR);
          if (dsPos!=String::npos) curPath=fileName.substr(0,dsPos);
          logD("returning %s",fileName.c_str());
          return true;
        }
      }
    } else {
      if (dialogO!=NULL) {
        if (dialogO->ready(0)) {
          if (dialogO->result().empty()) {
            fileName="";
            logD("returning nothing");
          } else {
            fileName=dialogO->result()[0];
            size_t dsPos=fileName.rfind(DIR_SEPARATOR);
            if (dsPos!=String::npos) curPath=fileName.substr(0,dsPos);
            logD("returning %s",fileName.c_str());
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

String FurnaceGUIFileDialog::getFileName() {
  if (sysDialog) {
    return fileName;
  } else {
    return ImGuiFileDialog::Instance()->GetFilePathName();
  }
}
