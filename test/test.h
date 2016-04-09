#ifndef __TEST_H__
#define __TEST_H__

#include <stdio.h>

#define BASH_RED   "\e[0;31m"
#define BASH_GREEN "\e[0;32m"
#define BASH_RESET "\e[0m"

#define PASSED BASH_GREEN "passed" BASH_RESET
#define FAILED BASH_RED   "failed" BASH_RESET

#define BYTETOBINARYPATTERN "%d%d%d%d%d%d%d%d"
#define BYTETOBINARY(byte) \
  (byte & 0x80 ? 1 : 0),   \
  (byte & 0x40 ? 1 : 0),   \
  (byte & 0x20 ? 1 : 0),   \
  (byte & 0x10 ? 1 : 0),   \
  (byte & 0x08 ? 1 : 0),   \
  (byte & 0x04 ? 1 : 0),   \
  (byte & 0x02 ? 1 : 0),   \
  (byte & 0x01 ? 1 : 0)

bool token_assert(const papar_token token, enum papar_token_type type, const char *str)
{
  if (token.type != type) {
    printf("x");
    fprintf(stderr, "expected `%s`, got: `%s`\n", papar_token_type_lookup[type], papar_token_type_lookup[token.type]);
    return false;
  }

  if (strncmp(str, token.start, strlen(str))) {
    printf("x");
    fprintf(stderr, "expected `%s`, got: `%.*s`\n", str, (int)token.length, token.start);
    return false;
  }

  printf(".");
  return true;
}

#endif
