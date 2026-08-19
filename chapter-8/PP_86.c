#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[]) {
  int i;

  printf("Command-line arguments:\n");

  for (i = 0; i < argc; i++) {
    printf("\targv[%2d]: %s\n", i, argv[i]);
  }

  printf("Enviroment variables:\n");

  for (i = 0; envp[i] != NULL; i++) {
    printf("\tenvp[%3d]: %s\n", i, envp[i]);
  }
  return EXIT_SUCCESS;
}
