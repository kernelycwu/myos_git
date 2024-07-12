#include <stdlib.h>
#include <stdio.h>

int main() {
  extern const char myfile[];
  extern const size_t myfile_len;
  fwrite(myfile, 1, myfile_len, stdout);
  return 0;
}
