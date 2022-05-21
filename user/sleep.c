#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(2, "usage: sleep {n}\n");
    exit(1);
  }
  // man 3 sleep
  // sleep returns the number of units to sleep
  // so it should be 0 if the requested time has elapsed
  exit(sleep(atoi(argv[1])));
}
