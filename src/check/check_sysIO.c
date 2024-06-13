#include <sys/io.h>

int main(int argc, char** argv) {
  inb(0x61);
  outb(0x00,0x61);
}
