#include "../ta-utils.h"
#include "imgui.h"
#include <functional>
#include "../pch.h"

#if defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
#ifndef SUPPORT_XP
#define USE_NFD
#endif
#endif

#ifdef USE_NFD
#include <atomic>
#include <thread>

#ifdef __APPLE__
#define NFD_NON_THREADED
#endif

#elif defined(ANDROID)
#include <jni.h>
#else
namespace pfd {
  class open_file;
  class save_file;
  class select_folder;
}
#endif

typedef std::function<void(const char*)> FileDialogSelectCallback;

class FurnaceGUIFileDialog {
  bool sysDialog;
  bool opened;
  unsigned char dialogType;
  bool hasError;
  char noSysFilter[4096];
  String curPath;
  std::vector<String> fileName;
#ifdef USE_NFD
  std::thread* dialogO;
  std::thread* dialogS;
  std::thread* dialogF;
  std::atomic<bool> dialogOK;
  std::vector<String> nfdResult;
#elif defined(ANDROID)
  JNIEnv* jniEnv;
  void* dialogO;
  void* dialogS;
  void* dialogF;
#else
  pfd::open_file* dialogO;
  pfd::save_file* dialogS;
  pfd::select_folder* dialogF;
#endif

  void convertFilterList(std::vector<String>& filter);
  public:
    bool mobileUI;
    bool openLoad(String header, std::vector<String> filter, String path, double dpiScale, FileDialogSelectCallback clickCallback=NULL, bool allowMultiple=false, String hint="");
    bool openSave(String header, std::vector<String> filter, String path, double dpiScale, String hint="");
    bool openSelectDir(String header, String path, double dpiScale, String hint="");
    bool accepted();
    void close();
    bool render(const ImVec2& min, const ImVec2& max);
    bool isOpen();
    bool isError();
    String getPath();
    std::vector<String>& getFileName();
    explicit FurnaceGUIFileDialog(bool system):
      sysDialog(system),
      opened(false),
      dialogType(0),
      hasError(false),
#ifdef ANDROID
      jniEnv(NULL),
#endif
      dialogO(NULL),
      dialogS(NULL),
      mobileUI(false) {}
};
