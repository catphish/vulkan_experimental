#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "engine.h"

FileData readFile(char* path) {
  FileData fileData;
  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    printf("Failed to open file: %s\n", path);
    exit(1);
  }
  fileData.size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  fileData.data = malloc(fileData.size);
  int n = read(fd, fileData.data, fileData.size);
  if (n < fileData.size) {
    printf("Failed to read file %s!\n", path);
    exit(1);
  }
  close(fd);
  return fileData;
}
