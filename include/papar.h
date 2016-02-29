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

  bool has_error;
  const char *error_start;
  const char *error_end;
  ssize_t error_offset;

  size_t position;
  size_t capacity;
  size_t size;
} papar_tokenlist;


void papar_lex(const char *d, papar_tokenlist *tokenlist);

papar_token papar_token_new(const char* start, size_t length, enum papar_token_type type);

papar_tokenlist *papar_tokenlist_new(unsigned int initial_capacity);
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
