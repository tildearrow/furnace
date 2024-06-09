#include <locale.h>

int main(int, char**) {
  setlocale(LC_CTYPE,"");
  setlocale(LC_MESSAGES,"");
  return 0;
}
