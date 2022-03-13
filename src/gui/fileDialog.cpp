#include "fileDialog.h"
#include "ImGuiFileDialog.h"
#include "../../extern/pfd/portable-file-dialogs.h"

bool FurnaceGUIFileDialog::openLoad(String header, std::vector<String> filter, String path, double dpiScale) {
  if (opened) return false;
  saving=false;
  curPath=path;
  if (sysDialog) {
    dialogO=new pfd::open_file(header,path,filter);
  } else {
    String parsedFilter;
    if (filter.size()&1) return false;

    for (size_t i=0; i<filter.size(); i+=2) {
      if (i!=0) parsedFilter+=",";
      parsedFilter+=filter[i]+"{"+filter[i+1]+"}";
    }

    ImGuiFileDialog::Instance()->DpiScale=dpiScale;
    ImGuiFileDialog::Instance()->OpenModal("FileDialog",header,parsedFilter.c_str(),path);
  }
  return true;
}

bool FurnaceGUIFileDialog::openSave(String header, std::vector<String> filter, String path, double dpiScale) {
  curPath=path;
  if (sysDialog) {
    // TODO
  } else {
    String parsedFilter;
    ImGuiFileDialog::Instance()->DpiScale=dpiScale;
    ImGuiFileDialog::Instance()->OpenModal("FileDialog",header,parsedFilter.c_str(),path,1,nullptr,ImGuiFileDialogFlags_ConfirmOverwrite);
  }
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
        printf("deleting\n");
      }
    }
  } else {
    ImGuiFileDialog::Instance()->Close();
  }
}

bool FurnaceGUIFileDialog::render(const ImVec2& min, const ImVec2& max) {
  if (sysDialog) {
    if (saving) {
      if (dialogS!=NULL) {
        if (dialogS->ready(1)) {
          fileName=dialogS->result();
          printf("returning %s\n",fileName.c_str());
          return true;
        }
      }
    } else {
      if (dialogO!=NULL) {
        if (dialogO->ready(1)) {
          if (dialogO->result().empty()) {
            fileName="";
            printf("returning nothing\n");
          } else {
            fileName=dialogO->result()[0];
            printf("returning %s\n",fileName.c_str());
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