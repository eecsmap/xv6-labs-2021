#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
// To implement the design described here: https://swtch.com/~rsc/thread/

// This solution requires 1 process for each filter.
// there are 168 prime numbers in 1000, therefore the solution brings up 169
// processes. make sure you config xv6 param.h to allow at least 171 processes:
//  1 for init
//  1 for sh
//  1 for primes main process
//  1 for each filter * 168
// Otherwise, without modifying system configuration. the last prime number
// printed will be the 61st (64-3): 283. To print this one, you still need to
// use value_t as short.

typedef char value_t; // short
int limit = 256;      // 284

int main(int argc, char *argv[]) {
  int pd[2];
  if (pipe(pd) < 0) {
    fprintf(2, "pipe error\n");
    exit(1);
  };

  int pid = fork();
  if (pid == -1) {
    fprintf(2, "fork error\n");
    exit(1);
  };
  if (pid) {
    // main process prepare input data
    // it has to be a separate process since write() might block on kernel buff
    // consumption. to make sure the data in pipe flows, we need the reader to
    // be a child process.
    close(pd[0]);
    for (int i = 2; i < limit; i++) {
      if (write(pd[1], &i, sizeof(value_t)) < 0) {
        fprintf(2, "write error\n");
        exit(1);
      };
    }
    close(pd[1]);
    wait(0);
    exit(0);
  }

  // filters starting ...
  close(pd[1]);
  int input = pd[0];
  int k;
  int output;
  value_t n;
start:
  k = 0;
  output = 0;
  while (sizeof(value_t) == read(input, &n, sizeof(value_t))) {
    if (k == 0) {
      k = n;
      printf("prime %d\n", k);
      // a. either create next filter on receiving, waste 1 more process
    }
    if (n % k) {
      // b. or create next filter on writing
      if (output == 0) {
        if (pipe(pd) < 0) {
          fprintf(2, "pipe error\n");
          exit(1);
        };
        pid = fork();
        if (pid == -1) {
          fprintf(2, "fork error\n");
          exit(1);
        };
        if (pid) {
          // current filter setup
          close(pd[0]);   // close read end
          output = pd[1]; // target output to the pipe write end; assert(5 ==
                          // output);
        } else {
          // new filter setup
          close(input);  // close the input fd copied by fork
          input = pd[0]; // retarget input to the pipe input end
          close(pd[1]);  // close write end
          goto start;    // start new filter
        }
      }
      if (write(output, &n, sizeof(value_t)) < 0) {
        fprintf(2, "write error\n");
        exit(1);
      };
    }
  } // get here when input is closed by upstream filter
  close(input);
  close(output); // signal next filter to quick while
  wait(0);       // wait for the next filter to exit
  exit(0);
}

/* vim: set ts=2 sts=2 sw=2 et: */