#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  int pipe_p2c[2];
  int pipe_c2p[2];
  if (-1 == pipe(pipe_p2c)) {
    fprintf(2, "pipe failed.\n");
    exit(1);
  }
  if (-1 == pipe(pipe_c2p)) {
    fprintf(2, "pipe failed.\n");
    exit(1);
  }

  int pid;
  if (-1 == (pid = fork())) {
    fprintf(2, "fork failed\n");
    exit(1);
  }

  if (pid == 0) {
    // child
    pid = getpid();
    close(pipe_c2p[0]);
    close(pipe_p2c[1]);
    char c;
    if (read(pipe_p2c[0], &c, 1) < 1) {
      // return -1 on errors
      // return 0 on EOF
      fprintf(2, "read error\n");
      exit(1);
    }
    printf("%d: received ping\n", pid);
    if (write(pipe_c2p[1], &c, 1) < 1) {
      fprintf(2, "write error\n");
      exit(1);
    }
    close(pipe_p2c[0]); // can skip since exit(0) will do it
    close(pipe_c2p[1]); // can skip since exit(0) will do it
  } else {
    // parent
    pid = getpid();
    close(pipe_p2c[0]);
    close(pipe_c2p[1]);
    char c = 'a';
    if (write(pipe_p2c[1], &c, 1) < 1) {
      fprintf(2, "write error\n");
      exit(1);
    }
    if (read(pipe_c2p[0], &c, 1) < 1) {
      fprintf(2, "read error\n");
      exit(1);
    }
    printf("%d: received pong\n", pid);
    close(pipe_c2p[0]); // can skip since exit(0) will do it
    close(pipe_p2c[1]); // can skip since exit(0) will do it
  }
  exit(0);
}
