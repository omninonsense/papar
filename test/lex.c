#include <stdio.h>
#include "papar.h"
#include "test.h"

#define TEST_NAME "lexing"

int main(int argc, char const *argv[]) {

  papar_tokenlist *tokenlist = papar_tokenlist_new(0);
  papar_lex(tokenlist, "M3.14e0,-6.28e-1 33,47");

  int i = 0;
  int errors = 0;

  printf(TEST_NAME ": ");

  if (tokenlist->error_type) {
    puts(" " FAILED);
    return tokenlist->error_type;
  }

  if (!token_assert(tokenlist->tokens[i++], PAPAR_TOK_COMMAND, "M")) errors++;
  if (!token_assert(tokenlist->tokens[i++], PAPAR_TOK_NUMBER, "3.14e0")) errors++;
  if (!token_assert(tokenlist->tokens[i++], PAPAR_TOK_COMMA, ",")) errors++;
  if (!token_assert(tokenlist->tokens[i++], PAPAR_TOK_NUMBER, "-6.28e-1")) errors++;
  if (!token_assert(tokenlist->tokens[i++], PAPAR_TOK_NUMBER, "33")) errors++;
  if (!token_assert(tokenlist->tokens[i++], PAPAR_TOK_COMMA, ",")) errors++;
  if (!token_assert(tokenlist->tokens[i++], PAPAR_TOK_NUMBER, "47")) errors++;
  if (!token_assert(tokenlist->tokens[i++], PAPAR_TOK_EOF, "")) errors++;

  papar_tokenlist_free(tokenlist);

  if (errors || tokenlist->error_type) puts(" " FAILED);
  else        puts(" " PASSED);

  return errors;
}
