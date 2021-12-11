#include "gui.h"

void FurnaceGUI::bindEngine(DivEngine* eng) {
  e=eng;
}

FurnaceGUI::FurnaceGUI():
  e(NULL) {
}