struct DivOrders {
  unsigned char ord[32][128];

  DivOrders() {
    memset(ord,0,32*128);
  }
};
