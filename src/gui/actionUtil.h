#define DETERMINE_FIRST \
  int firstChannel=0; \
  for (int i=0; i<e->getTotalChannelCount(); i++) { \
    if (e->song.chanShow[i]) { \
      firstChannel=i; \
      break; \
    } \
  } \

#define DETERMINE_LAST \
  int lastChannel=0; \
  for (int i=e->getTotalChannelCount()-1; i>=0; i--) { \
    if (e->song.chanShow[i]) { \
      lastChannel=i+1; \
      break; \
    } \
  }

#define DETERMINE_FIRST_LAST \
  DETERMINE_FIRST \
  DETERMINE_LAST