#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

void find(char *dir_path, char *name) {

  int fd;
  if ((fd = open(dir_path, 0)) < 0) {
    fprintf(2, "cannot open %s\n", dir_path);
    return;
  }

  struct stat st;
  if (fstat(fd, &st) < 0) {
    fprintf(2, "cannot stat %s\n", dir_path);
    close(fd);
    return;
  }

  char buf[512], *p;
  int path_len = strlen(dir_path);
  if (T_DIR == st.type && path_len + 1 < sizeof buf) {
    strcpy(buf, dir_path);
    p = buf + path_len;
    *p++ = '/';

    struct dirent de;

    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
      // de.name is 14 bytes yet might not end with '\0', make a safe one
      char de_name[DIRSIZ + 1];
      memmove(de_name, de.name, DIRSIZ);
      de_name[DIRSIZ] = 0;
      if (de.inum == 0)
        continue;
      if (0 == strcmp(de_name, ".") || 0 == strcmp(de_name, ".."))
        continue;
      if (0 == strcmp(de_name, name)) {
        printf("%s/%s\n", dir_path, name);
      }
      int name_len = strlen(de_name);
      if (sizeof buf - name_len > p - buf) {
        // memmove(p, de.name, DIRSIZ); // refer to ls.c
        strcpy(p, de_name); // de.name might not end with '\0'
        find(buf, name);
      }
    }
  }
  close(fd);
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(2, "usage: find dir name\n");
    exit(1);
  }

  char *dir_path = argv[1];
  char *name = argv[2];

  find(dir_path, name);
  exit(0);
}
