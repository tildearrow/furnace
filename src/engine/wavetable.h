struct DivWavetable {
  int len;
  int data[32];

  DivWavetable():
    len(32) {
    for (int i=0; i<32; i++) {
      data[i]=i;
    }
  }
};
