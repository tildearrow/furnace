#include <dirent.h>

int main(int argc, char** argv) {
  struct dirent deTest = { };
  unsigned char deType = deTest.d_type;
  return 0;
}
