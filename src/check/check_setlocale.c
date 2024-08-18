#include <locale.h>

int main(int argc, char** argv) {
  setlocale(LC_CTYPE,"");
  //setlocale(LC_MESSAGES,"");
  return 0;
}
