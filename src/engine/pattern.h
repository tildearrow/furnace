struct DivPattern {
  char data[256][16];
};

struct DivChannelData {
  char effectRows;
  // data goes as follows: data[ROW][TYPE]
  // TYPE is:
  // 0: note
  // 1: octave
  // 2: instrument
  // 3: volume
  // 4-5+: effect/effect value
  std::vector<DivPattern*> data;
};