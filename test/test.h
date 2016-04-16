#ifndef __TEST_H__
#define __TEST_H__

#include <stdio.h>
#include <stdarg.h>
#include <float.h>
#include <math.h>

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

bool dblcmp(double a, double b)
{
  if (fabs(a - b) >= DBL_EPSILON) {
    printf("x");
    fprintf(stderr, "expected `%f`, got: `%f`\n", a, b);
    return  false;
  }

  return true;
}

bool cmd_assert(const papar_command cmd, char type, ...)
{
  va_list vargs;

  if (cmd.type != type) {
    printf("x");
    fprintf(stderr, "expected `%c`, got: `%c`\n", type, cmd.type);
    return false;
  }

  va_start(vargs, type);

  switch (type) {
    case 'm':
    case 'M':
    case 'l':
    case 'L':
      if (!dblcmp(cmd.x, va_arg(vargs, double))) goto assert_fail;
      if (!dblcmp(cmd.y, va_arg(vargs, double))) goto assert_fail;
    break;
  }

  printf(".");
  va_end(vargs);
  return true;

assert_fail:
  va_end(vargs);
  return false;

}

#endif
