#include "fileDialog.h"
#include "ImGuiFileDialog.h"
#include "../ta-log.h"

#include "../../extern/pfd-fixed/portable-file-dialogs.h"

bool FurnaceGUIFileDialog::openLoad(String header, std::vector<String> filter, const char* noSysFilter, String path, double dpiScale) {
  if (opened) return false;
  saving=false;
  curPath=path;
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
