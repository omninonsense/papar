#ifndef __PAPAR__H
#define __PAPAR__H

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

#ifndef PAPAR_REPORT_ERRORS
#define PAPAR_REPORT_ERRORS 1
#endif

#ifndef PAPAR_TOKENLIST_GRWOTH_RATE
#define PAPAR_TOKENLIST_GRWOTH_RATE 0.3
#endif

#if PAPAR_DEBUGGING
  #ifndef PAPAR_PRINT_DEBUG
  #define PAPAR_PRINT_DEBUG 1
  #endif
#else
  #ifndef PAPAR_PRINT_DEBUG
  #define PAPAR_PRINT_DEBUG 0
  #endif
#endif

#if PAPAR_DEBUGGING
  #define D(x) do { x } while(0)
#else
  #define D(x) do {   } while(0)
#endif

#ifndef PAPAR_PRINT_WARN
#define PAPAR_PRINT_WARN  1
#endif

#ifndef PAPAR_PRINT_ERROR
#define PAPAR_PRINT_ERROR 1
#endif

#define PAPAR_TOK(s, tl, offs) (tl->tokens[s->position+offs])
#define PAPAR_TOK_BEG(s, tl, offs) (tl->tokens[s->position+offs].start)
#define PAPAR_TOK_LEN(s, tl, offs) (tl->tokens[s->position+offs].length)
#define PAPAR_TOK_END(s, tl, offs) (tl->tokens[s->position+offs].start + PAPAR_TOK_LEN(tl, s, offs))
#define PAPAR_TOK_TYPE(s, tl, offs) (tl->tokens[s->position+offs].type)

#define PAPAR_DEBUG(fmt, ...)                                                                      \
  do {                                                                                             \
    if (PAPAR_PRINT_DEBUG)                                                                         \
      fprintf(stderr, "[DEBUG] %s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __func__, __VA_ARGS__); \
  } while (0)

#define PAPAR_WARN(fmt, ...)                                                                       \
  do {                                                                                             \
    if (PAPAR_PRINT_WARN)                                                                          \
      fprintf(stderr, "[WARN ] " fmt "\n", __VA_ARGS__);                                           \
  } while (0)

#define PAPAR_ERROR(fmt, ...)                                                                      \
  do {                                                                                             \
    if (PAPAR_PRINT_ERROR)                                                                         \
      fprintf(stderr, "[ERROR] " fmt "\n", __VA_ARGS__);                                           \
  } while (0)

#if PAPAR_PRINT_DEBUG || PAPAR_PRINT_WARN || PAPAR_PRINT_ERROR
#include <stdio.h>
#endif

#include <sys/types.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>

#define PAPAR_LARGE_ARC_FLAG  0x1
#define PAPAR_SWEEP_FLAG      0x2

enum papar_token_type {
  PAPAR_TOK_UNDEF = 0,
  PAPAR_TOK_COMMAND,
  PAPAR_TOK_NUMBER,
  PAPAR_TOK_COMMA,
  PAPAR_TOK_FLAG, // Used by parser, but not by lexer
  PAPAR_TOK_WHITESPACE, // Unused, but reserved
  PAPAR_TOK_EOF,
  PAPAR_TOK_LAST
};

static const char *const papar_token_type_lookup[] = {
  "PAPAR_TOK_UNDEF", "PAPAR_TOK_COMMAND",
  "PAPAR_TOK_NUMBER", "PAPAR_TOK_COMMA",
  "PAPAR_TOK_FLAG", "PAPAR_TOK_WHITESPACE",
  "PAPAR_TOK_EOF", "PAPAR_TOK_LAST"
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

  char last_command;

  size_t position;
  size_t size;
  size_t capacity;
} papar_state;

void papar_lex(papar_tokenlist *self, const char *d);
void papar_parse(papar_state *self, const papar_tokenlist *tokenlist);

papar_state *papar_state_new(size_t initial_capacity);
void papar_state_free(papar_state *self);
int papar_state_push(papar_state *self, papar_command command);
papar_command *papar_state_pop(papar_state *self);

int papar__state_grow(papar_state *self, size_t amount);

enum papar_token_type papar__parser_peek(papar_state *self, const papar_tokenlist *tl, ssize_t offset);
int papar__parser_expect(papar_state *self, const papar_tokenlist *tl, ssize_t offset, size_t count, ...);

void papar__parser_consume_number(papar_state *self, const papar_tokenlist *tl, double *number);
void papar__parser_consume_point(papar_state *self, const papar_tokenlist *tl, double *x, double *y);
void papar__parser_consume_flag(papar_state *self, const papar_tokenlist *tl, bool *flag);

void papar__parser_parse_command(papar_state *self, const papar_tokenlist *tl, char c);
void papar__parser_parse_goto(papar_state *self, const papar_tokenlist *tl);
void papar__parser_parse_closepath(papar_state *self, const papar_tokenlist *tl);
void papar__parser_parse_lineto(papar_state *self, const papar_tokenlist *tl);
void papar__parser_parse_horizontallineto(papar_state *self, const papar_tokenlist *tl);
void papar__parser_parse_verticallineto(papar_state *self, const papar_tokenlist *tl);
void papar__parser_parse_3bezierto(papar_state *self, const papar_tokenlist *tl);
void papar__parser_parse_smooth3bezierto(papar_state *self, const papar_tokenlist *tl);
void papar__parser_parse_2bezierto(papar_state *self, const papar_tokenlist *tl);
void papar__parser_parse_smooth2bezierto(papar_state *self, const papar_tokenlist *tl);
void papar__parser_parse_ellipticalarc(papar_state *self, const papar_tokenlist *tl);

papar_token papar_token_new(const char* start, size_t length, enum papar_token_type type);

papar_tokenlist *papar_tokenlist_new(size_t initial_capacity);
void papar_tokenlist_free(papar_tokenlist *self);
int papar_tokenlist_push(papar_tokenlist *self, papar_token token);
papar_token *papar_tokenlist_pop(papar_tokenlist *self);

int papar__tokenlist_grow(papar_tokenlist *self, size_t amount);
bool papar__isws(char c);
const char *papar__lexer_consume_whitespace(papar_tokenlist *tokenlist, const char *c);
const char *papar__lexer_consume_command(papar_tokenlist *tokenlist, const char *c);
const char *papar__lexer_consume_number(papar_tokenlist *tokenlist, const char *c);
const char *papar__lexer_consume_comma(papar_tokenlist *tokenlist, const char *c);

#endif
