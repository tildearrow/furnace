struct DivWavetable {
  int len, min, max;
  int data[32];

  DivWavetable():
    len(32),
    min(0),
    max(31) {
    for (int i=0; i<32; i++) {
      data[i]=i;
    }
  }
};
