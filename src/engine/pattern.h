struct DivPattern {
  short data[256][16];
  DivPattern();
};

struct DivChannelData {
  unsigned char effectRows;
  // data goes as follows: data[ROW][TYPE]
  // TYPE is:
  // 0: note
  // 1: octave
  // 2: instrument
  // 3: volume
  // 4-5+: effect/effect value
  DivPattern* data[128];
  DivPattern* getPattern(int index, bool create);
  DivChannelData();
};
