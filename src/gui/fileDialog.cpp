#include "fileDialog.h"
#include "ImGuiFileDialog.h"
#include "../ta-log.h"

#include "../../extern/pfd-fixed/portable-file-dialogs.h"

bool FurnaceGUIFileDialog::openLoad(String header, std::vector<String> filter, const char* noSysFilter, String path, double dpiScale) {
  if (opened) return false;
  saving=false;
  curPath=path;
  logD("opening load file dialog with curPath %s",curPath.c_str());
  if (sysDialog) {
    dialogO=new pfd::open_file(header,path,filter);
  } else {
    ImGuiFileDialog::Instance()->DpiScale=dpiScale;
    ImGuiFileDialog::Instance()->OpenModal("FileDialog",header,noSysFilter,path);
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
    dialogS=new pfd::save_file(header,path,filter);
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
        delete dialogS;
        dialogS=NULL;
      }
    } else {
      if (dialogO!=NULL) {
        delete dialogO;
        dialogO=NULL;
      }
    }
  } else {
    ImGuiFileDialog::Instance()->Close();
  }
  opened=false;
}

bool FurnaceGUIFileDialog::render(const ImVec2& min, const ImVec2& max) {
  if (sysDialog) {
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
  } else {
    return ImGuiFileDialog::Instance()->Display("FileDialog",ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoMove,min,max);
  }
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
