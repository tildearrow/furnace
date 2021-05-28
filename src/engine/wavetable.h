struct DivWavetable {
  int len;
  int data[32];

  DivWavetable():
    len(32) {
    memset(data,0,32*sizeof(int));
  }
};
