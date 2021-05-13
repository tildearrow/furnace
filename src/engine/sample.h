struct DivSample {
  String name;
  int length, rate;
  signed char vol, pitch;
  unsigned char depth;
  short* data;
  int rendLength;
  short* rendData;

  DivSample():
    name(""),
    length(0),
    rate(0),
    vol(0),
    pitch(0),
    depth(16),
    data(NULL),
    rendLength(0),
    rendData(NULL) {}
};
