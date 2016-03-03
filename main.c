#include "papar.h"
#include <stdio.h>

int main(int argc, char const *argv[])
{

  if (argc < 2)
    return 1;

  papar_tokenlist *tokenlist = papar_tokenlist_new(0);
  papar_lex(argv[1], tokenlist);

  if (tokenlist->error_type) {
    if (tokenlist->error_type == PAPAR_ERR_MEM) {
      fprintf(stderr, "Papar ran out of memory during lexing\n");
      return 3;
    }

    fprintf(stderr, "Encountered error while tokenizing, couldn't process token `%.*s` at offset %zd\n", (int)(tokenlist->error_end - tokenlist->error_start), tokenlist->error_start, tokenlist->error_start - tokenlist->src);
    return 2;
  }

  for (size_t i = 0; i < tokenlist->size; ++i) {
    printf("%s(\"%.*s\")\n", papar_token_type_lookup[tokenlist->tokens[i].type], (int)(tokenlist->tokens[i].length), tokenlist->tokens[i].start);
  }

  papar_tokenlist_free(tokenlist);
  return 0;
}
