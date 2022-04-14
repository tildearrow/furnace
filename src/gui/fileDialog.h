#include "../ta-utils.h"
#include "imgui.h"
#include <vector>

namespace pfd {
  class open_file;
  class save_file;
}

class FurnaceGUIFileDialog {
  bool sysDialog;
  bool opened;
  bool saving;
  String curPath;
  String fileName;
  pfd::open_file* dialogO;
  pfd::save_file* dialogS;
  public:
    bool openLoad(String header, std::vector<String> filter, const char* noSysFilter, String path, double dpiScale);
    bool openSave(String header, std::vector<String> filter, const char* noSysFilter, String path, double dpiScale);
    bool accepted();
    void close();
    bool render(const ImVec2& min, const ImVec2& max);
    String getPath();
    String getFileName();
    explicit FurnaceGUIFileDialog(bool system):
      sysDialog(system),
      opened(false),
      saving(false),
      dialogO(NULL),
      dialogS(NULL) {}
};
