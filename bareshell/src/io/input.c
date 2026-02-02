#include "input.h"
#include <stdio.h>
#include <string.h>

int multiline_input(char *buffer, int max_size, FILE *stream) {
  char line_buffer[1024];
  char last_quote;
  int quote_count = 0;
  int pos = 0;
  printf(">> ");
  while (1) {
    if (fgets(line_buffer, sizeof(line_buffer), stream) == NULL) {
      break; // EOF Condition
    }
    int len = strlen(line_buffer);
    for (int i = 0; i < len && line_buffer[i] != '\0'; i++) {
      char ch = line_buffer[i];
      if (ch != '"' && ch != '\'') {
        continue;
      }
      if (i > 0 && line_buffer[i - 1] == '\\') { // Escaped quotes handling
        continue;
      };
      if (quote_count > 0 && last_quote == ch) {
        quote_count--;
      } else {
        quote_count++;
        last_quote = ch;
      }
    }
    if (pos + len >= max_size) {
      len = max_size - pos - 1;
      line_buffer[len] = '\0';
      strcpy(buffer + pos, line_buffer);
      pos += len;
      return 1; // "Buffer Overflow"
    }
    strcpy(buffer + pos, line_buffer);
    pos += len;

    line_buffer[0] = '\0';
    if (line_buffer[len - 2] != '\\' && quote_count == 0) {
      break;
    }
    if (line_buffer[len - 2] == '\\' && quote_count == 0) {
      pos -= 2;
    }
    printf(".  ");
  }
  return 0;
}
