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

enum papar_command_type {
  // *
  PAPAR_CMD_UNKNOWN = 0,

  // M, m
  PAPAR_CMD_MOVETO_ABSOLUTE,
  PAPAR_CMD_MOVETO_RELATIVE,

  // Z, z
  PAPAR_CMD_CLOSEPATH,

  // L, l
  PAPAR_CMD_LINETO_ABSOLUTE,
  PAPAR_CMD_LINETO_RELATIVE,

  // H, h
  PAPAR_CMD_LINETOHORIZONTAL_ABSOLUTE,
  PAPAR_CMD_LINETOHORIZONTAL_RELATIVE,

  // V, v
  PAPAR_CMD_LINETOVERTICAL_ABSOLUTE,
  PAPAR_CMD_LINETOVERTICAL_RELATIVE,

  // C, c
  PAPAR_CMD_3CURVETO_ABSOLUTE,
  PAPAR_CMD_3CURVETO_RELATIVE,

  // S, s
  PAPAR_CMD_3CURVETOSMOOTH_ABSOLUTE,
  PAPAR_CMD_3CURVETOSMOOTH_RELATIVE,

  // Q, q
  PAPAR_CMD_2CURVETO_ABSOLUTE,
  PAPAR_CMD_2CURVETO_RELATIVE,

  // T, t
  PAPAR_CMD_2CURVETOSMOOTH_ABSOLUTE,
  PAPAR_CMD_2CURVETOSMOOTH_RELATIVE,

  // A, a
  PAPAR_CMD_ELLIPTICALARC_ABSOLUTE,
  PAPAR_CMD_ELLIPTICALARC_RELATIVE,

  PAPAR_CMD_LAST
};

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
  enum papar_command_type type;
                 // Alternate usage in Arc commands:
  double x,  y;  // x, y
  double x1, y1; // rx, ry
  double x2, y2; // x-axis-rotation, [unused]
  uint8_t flags; // large-arc-flag and sweep-flag
} papar_command;

typedef struct papar_state {
  papar_command *commands;
  const papar_tokenlist *tokenlist;

  size_t position;
  size_t size;
  size_t capacity;

  bool needs_more;
} papar_state;

void papar_lex(const char *d, papar_tokenlist *tokenlist);
void papar_parse(papar_tokenlist *tokenlist, papar_state *state);

papar_command papar_command_new(enum papar_command_type type);

papar_state *papar_state_new(size_t initial_capacity);

void papar__parser_expect_cmd(papar_tokenlist *tl, char *c);
void papar__parser_expect_flag(papar_tokenlist *tl, bool *f);
void papar__parser_expect_point(papar_tokenlist *tl, double *x, double *y);

void papar__parser_parse_cmd(papar_state* self, enum papar_command_type type);

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
