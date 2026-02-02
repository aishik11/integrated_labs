#include "repl/looper.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  while (1) {
    enum repl_error err = looper();
    if (err != REPL_SIGINT) {
      return err;
    }
    printf("\n");
  }
}