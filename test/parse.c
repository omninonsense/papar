#include <stdio.h>
#include "papar.h"
#include "test.h"

#define TEST_NAME "parsing"

int main(int argc, char const *argv[]) {

  papar_tokenlist *tokenlist = papar_tokenlist_new(0);
  papar_state *state = papar_state_new(0);

  int i = 0;
  int errors = 0;

  papar_lex(tokenlist, "M3.14e0,-6.28e-1 33,47");
  papar_parse(state, tokenlist);

  printf(TEST_NAME ": ");

  if (state->error_type) {
    puts(" " FAILED);
    return(state->error_type);
  }

  for (size_t i = 0; i < state->size; ++i) {
    PAPAR_DEBUG("%c("
        "x: %.2f, "
        "y: %.2f, "
        "x1/rx: %.2f, "
        "y1/ry: %.2f, "
        "x2/x-axis-rotation: %.2f, "
        "y2: %.2f, "
        "flags: "BYTETOBINARYPATTERN
      ")",
      state->commands[i].type,
      state->commands[i].x,
      state->commands[i].y,
      state->commands[i].x1,
      state->commands[i].y1,
      state->commands[i].x2,
      state->commands[i].y2,
      BYTETOBINARY(state->commands[i].flags)
    );
  }

  if (!cmd_assert(state->commands[i++], 'M', 3.14e0, -6.28e-1)) errors++;
  if (!cmd_assert(state->commands[i++], 'L', 33.0, 47.0)) errors++;

  papar_tokenlist_free(tokenlist);
  papar_state_free(state);

  if (errors || tokenlist->error_type || state->error_type) puts(" " FAILED);
  else        puts(" " PASSED);

  return errors;
}
