#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

/*
We need to save arguments for exec
Put the data into data section help avoid blowing away the stack.

As in the lab description, our xargs read arguments line by line
and for each line of arguments, it will call command once.
So we need a buf with enough space to hold the line.
1024 is just an arbitrary value.
*/
#define BUF_SIZE 512
char buf[BUF_SIZE];

void run(char **args) {
  int pid;
  if (-1 == (pid = fork())) {
    fprintf(2, "fork error\n");
    exit(1);
  }
  if (pid == 0) {
    exec(args[0], args);
  } else { // else is necessary since exec might return -1
    wait(0);
  }
}

int read_args(char *args[], int limit) {
  char c;
  int nargs = 0;
  int pos = 0;
  int inword = 0;
  args[nargs] = buf;
  while (nargs < limit) {
    int nbytes = read(0, &c, 1);
    if (nbytes < 0) {
      fprintf(2, "read error\n");
      exit(1);
    }
    if (nbytes == 0 || c == '\n' || c == ' ' || c == '\t') {
      if (inword) {
        buf[pos++] = '\0';
        args[++nargs] = buf + pos;
        inword = 0;
      }
      if (nbytes == 0 || c == '\n') {
        // EOF or NEWLINE
        break;
      }
    } else {
      if (!inword) {
        inword = 1;
      }
      if (pos >= BUF_SIZE - 1) {
        // we need one more byte to write '\0'
        fprintf(2, "arguments overflow buf\n");
        exit(1);
      }
      buf[pos++] = c;
    }
  }
  return nargs;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(2, "usage: xargs program [args...]\n");
    exit(1);
  }
  char *exec_argv[MAXARG + 1]; // we need to put a NULL argument at args[MAXARG]
  int exec_argc = 0;
  for (int i = 1; i < argc; ++i) {
    exec_argv[exec_argc++] = argv[i];
  }
  int nargs;
  while (0 < (nargs = read_args(exec_argv + exec_argc, MAXARG - exec_argc))) {
    exec_argv[exec_argc + nargs] = 0;
    run(exec_argv);
  }
  exit(0);
}
