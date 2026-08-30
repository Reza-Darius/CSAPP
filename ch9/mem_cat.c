#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  char *map;
  int fd, nread, read;
  struct stat file_stats;

  if (argc < 2) {
    err(EXIT_FAILURE, "please provide a path to a file");
  }

  if ((fd = open(argv[1], O_RDONLY)) < 0) {
    perror("open\n");
    err(EXIT_FAILURE, "open");
  }

  if (fstat(fd, &file_stats) < 0) {
    perror("file stats");
    err(EXIT_FAILURE, "file stats");
  }

  if ((map = mmap(NULL, file_stats.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) ==
      NULL) {
    perror("mapp");
    err(EXIT_FAILURE, "mmap");
  }

  nread = 0;
  read = 0;
  while (nread < file_stats.st_size) {
    if ((read = write(STDOUT_FILENO, map + nread, file_stats.st_size - nread)) < 0) {
      perror("write");
      err(EXIT_FAILURE, "write");
    }

    nread += read;
  }

  close(fd);
  munmap(map, file_stats.st_size);
  return EXIT_SUCCESS;
}
