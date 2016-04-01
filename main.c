#include "papar.h"

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

int main(int argc, char const *argv[])
{

  if (argc < 2)
    return 1;

  papar_tokenlist *tokenlist = papar_tokenlist_new(0);
  papar_state *state = papar_state_new(0);
  papar_lex(argv[1], tokenlist);

  if (tokenlist->error_type) {
    if (tokenlist->error_type == PAPAR_ERR_MEM) {
      PAPAR_ERROR("%s", "Papar ran out of memory during lexing");
      return 3;
    }

    PAPAR_ERROR("Encountered error while tokenizing, couldn't process token `%.*s` at offset %zd", (int)(tokenlist->error_end - tokenlist->error_start), tokenlist->error_start, tokenlist->error_start - tokenlist->src);
    return 2;
  }

  for (size_t i = 0; i < tokenlist->size; ++i) {
    PAPAR_DEBUG("%s(\"%.*s\")", papar_token_type_lookup[tokenlist->tokens[i].type], (int)(tokenlist->tokens[i].length), tokenlist->tokens[i].start);
  }

  papar_parse(tokenlist, state);

  if (state->error_type) {
    if (state->error_type == PAPAR_ERR_MEM) {
      PAPAR_ERROR("%s", "Papar ran out of memory during parsing");
      return 3;
    }

    return 4;
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

  // Previous point
  double px = 0.0,
         py = 0.0;

  int    pw = 780,   ph = 638;    // Portview dimensions
  double dw = 220.0, dh = 180.0;  // Document dimensions
  double tx = 0.0,   ty = -414.6; // Translation vector

  for (size_t i = 0; i < state->size; ++i) {
    papar_command cmd = state->commands[i];
    if (isupper(state->commands[i].type)) {
      px = cmd.x;
      py = cmd.y;
    } else {
      px += cmd.x;
      py += cmd.y;
    }

    PAPAR_DEBUG("%f, %f", (px - tx) * dw/pw, (ph - py - ty) * dh/ph);
  }

  papar_tokenlist_free(tokenlist);
  papar_state_free(state);
  return 0;
}
