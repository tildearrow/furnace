#include <stdio.h>

int main(int argc, char** argv) {
  if (argc<2) {
    printf("usage: %s text\n",argv[0]);
    return 1;
  }

  unsigned int checker=0x11111111;
  unsigned int checker1=0;
  int index=0;

  for (char* i=argv[1]; *i; i++) {
    checker^=((unsigned int)(*i))<<index;
    checker1+=(unsigned int)(*i);
    checker=(checker>>1|(((checker)^(checker>>2)^(checker>>3)^(checker>>5))&1)<<31);
    checker1<<=1;
    index=(index+1)&31;
  }

  printf("%.8x %x\n",checker,checker1);

  return 0;
}
