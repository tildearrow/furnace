#include "../ta-utils.h"
#include "imgui.h"
#include <functional>
#include <vector>

#if defined(_WIN64) || defined(__APPLE__)
#define USE_NFD
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
}
#endif

typedef std::function<void(const char*)> FileDialogSelectCallback;

class FurnaceGUIFileDialog {
  bool sysDialog;
  bool opened;
  bool saving;
  bool hasError;
  String curPath;
  std::vector<String> fileName;
#ifdef USE_NFD
  std::thread* dialogO;
  std::thread* dialogS;
  std::atomic<bool> dialogOK;
  std::vector<String> nfdResult;
#elif defined(ANDROID)
  JNIEnv* jniEnv;
  void* dialogO;
  void* dialogS;
#else
  pfd::open_file* dialogO;
  pfd::save_file* dialogS;
#endif
  public:
    bool mobileUI;
    bool openLoad(String header, std::vector<String> filter, const char* noSysFilter, String path, double dpiScale, FileDialogSelectCallback clickCallback=NULL, bool allowMultiple=false);
    bool openSave(String header, std::vector<String> filter, const char* noSysFilter, String path, double dpiScale);
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
      saving(false),
      hasError(false),
#ifdef ANDROID
      jniEnv(NULL),
#endif
      dialogO(NULL),
      dialogS(NULL),
      mobileUI(false) {}
};
