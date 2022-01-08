struct DivOrders {
  unsigned char ord[DIV_MAX_CHANS][128];

  DivOrders() {
    memset(ord,0,DIV_MAX_CHANS*128);
  }
};
