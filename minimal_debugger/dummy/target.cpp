#include <cstdio>
int main(int argc, char *argv[]) {
  int x = 0;
  while (true) {
    x++;
    printf("%d", x);
    if (x > 100)
      x = 0;
  }
  return 0;
}
