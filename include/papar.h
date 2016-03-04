#ifndef __PAPAR__H
#define __PAPAR__H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>


#define PAPAR_NUM_HAS_DOT       0x01
#define PAPAR_NUM_HAS_E         0x02

#ifndef PAPAR_TOKENLIST_MIN_CAP
#define PAPAR_TOKENLIST_MIN_CAP 20
#endif

#ifndef PAPAR_STATECMD_MIN_CAP
#define PAPAR_STATECMD_MIN_CAP 10
#endif

#ifndef PAPAR_STATECMD_GRWOTH_RATE
#define PAPAR_STATECMD_GRWOTH_RATE 0.3
#endif

#ifndef PAPAR_TOKENLIST_GRWOTH_RATE
#define PAPAR_TOKENLIST_GRWOTH_RATE 0.3
#endif

enum papar_token_type {
  PAPAR_TOK_UNDEF = 0,
  PAPAR_TOK_COMMAND,
  PAPAR_TOK_NUMBER,
  PAPAR_TOK_COMMA,
  PAPAR_TOK_WHITESPACE, // Unused, but reserved
  PAPAR_TOK_EOF,
  PAPAR_TOK_LAST // For custom lexing
};

enum papar_error_types {
  PAPAR_ERR_OK = 0,
  PAPAR_ERR_TOK,
  PAPAR_ERR_PAR,
  PAPAR_ERR_MEM
};

extern const char *const papar_token_type_lookup[];

typedef struct papar_token {
  const char *start;
  size_t length;
  enum papar_token_type type;
} papar_token;

typedef struct papar_tokenlist {
  papar_token *tokens;

  enum papar_error_types error_type;
  const char *error_start;
  const char *error_end;

  const char *src;

  size_t capacity;
  size_t size;
} papar_tokenlist;

typedef struct papar_command {
  char type;
                 // Alternate usage in Arc commands:
  double x,  y;  // x, y
  double x1, y1; // rx, ry
  double x2, y2; // x-axis-rotation, [unused]
  uint8_t flags;
} papar_command;

typedef struct papar_state {
  papar_command *commands;
  const papar_tokenlist *tokenlist;

  enum papar_error_types error_type;

  size_t position;
  size_t size;
  size_t capacity;
} papar_state;

void papar_lex(const char *d, papar_tokenlist *tokenlist);
void papar_parse(const papar_tokenlist *tokenlist, papar_state *state);

papar_state *papar_state_new(size_t initial_capacity);
void papar_state_free(papar_state *self);
int papar_state_push(papar_state *self, papar_command command);
papar_command *papar_state_pop(papar_state *self);

int papar__state_grow(papar_state *self, size_t amount);

void papar__parser_expect_cmd(papar_tokenlist *tl, char *c);
void papar__parser_expect_flag(papar_tokenlist *tl, bool *f);
void papar__parser_expect_point(papar_tokenlist *tl, double *x, double *y);

void papar__parser_parse_cmd(papar_state *self, papar_tokenlist *tl, char c);

papar_token papar_token_new(const char* start, size_t length, enum papar_token_type type);

papar_tokenlist *papar_tokenlist_new(size_t initial_capacity);
void papar_tokenlist_free(papar_tokenlist *self);
int papar_tokenlist_push(papar_tokenlist *self, papar_token token);
papar_token *papar_tokenlist_pop(papar_tokenlist *self);

int papar__tokenlist_grow(papar_tokenlist *self, size_t amount);
bool papar__isws(char c);
const char *papar__lexer_consume_whitespace(const char *c, papar_tokenlist *tokenlist);
const char *papar__lexer_consume_command(const char *c, papar_tokenlist *tokenlist);
const char *papar__lexer_consume_number(const char *c, papar_tokenlist *tokenlist);
const char *papar__lexer_consume_comma(const char *c, papar_tokenlist *tokenlist);

#endif
