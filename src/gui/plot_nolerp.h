#include "imgui.h"

void PlotNoLerp(const char* label, const float* values, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0, 0), int stride = sizeof(float));
void PlotBitfield(const char* label, const int* values, int values_count, int values_offset = 0, const char** overlay_text = NULL, int bits = 8, ImVec2 graph_size = ImVec2(0, 0), int stride = sizeof(float));