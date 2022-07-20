#include <dirent.h>

int main(int, char**) {
  struct dirent deTest = { };
  unsigned char deType = deTest.d_type;
  return 0;
}
