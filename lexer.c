#include "papar.h"

const char *const papar_token_type_lookup[] = {
  "PAPAR_TOK_UNDEF",
  "PAPAR_TOK_COMMAND",
  "PAPAR_TOK_NUMBER",
  "PAPAR_TOK_COMMA",
  "PAPAR_TOK_WHITESPACE",
  "PAPAR_TOK_EOF",
  "PAPAR_TOK_LAST"
};

papar_token papar_token_new(const char* start, size_t length, enum papar_token_type type)
{
  papar_token token;

  token.start = start;
  token.length = length;
  token.type = type;

  return token;
}

papar_tokenlist *papar_tokenlist_new(unsigned int initial_capacity)
{

  if (initial_capacity < 10)
    initial_capacity = 10;

  papar_tokenlist *self = (papar_tokenlist*) malloc(sizeof(papar_tokenlist));

  self->has_error = false;
  self->error_start = NULL;
  self->error_end = NULL;
  self->error_offset = 0;

  self->position = 0;
  self->size = 0;
  self->capacity = initial_capacity;

  self->tokens = (papar_token*) calloc(self->capacity, sizeof(papar_token));

  if (self->tokens == NULL) {
    free(self);
    return NULL;
  }

  return self;
}

void papar_tokenlist_free(papar_tokenlist *self)
{
  if (self != NULL) {
    if (self->tokens != NULL)
      free(self->tokens);

    free(self);
  }
}

int papar_tokenlist_push(papar_tokenlist *self, papar_token token)
{
  if (self->size >= self->capacity) {
    size_t amount = self->capacity * PAPAR_TOKENLIST_GRWOTH_RATE;
    if (papar__tokenlist_grow(self, amount)) {
      self->has_error = true;
      self->error_offset = -1;
      return 1;
    }
  }

  self->tokens[self->size++] = token;
  return 0;
}

papar_token *papar_tokenlist_pop(papar_tokenlist *self) {
  if (self->size <= 0)
    return NULL;

  return &self->tokens[self->size--];
}

int papar__tokenlist_grow(papar_tokenlist *self, size_t amount)
{
  size_t new_cap = amount + self->capacity;
  papar_token* tmp = (papar_token*) realloc(self->tokens, new_cap * sizeof(papar_token));

  if (tmp == NULL)
    return 1;

  self->capacity = new_cap;
  self->tokens = tmp;

  return 0;
}

void papar_lex(const char *d, papar_tokenlist *tokenlist) {
  const char *c = d;

  do {
    if (*c == 0) {
      papar_tokenlist_push(tokenlist, papar_token_new(c, 0, PAPAR_TOK_EOF));
      break;
    }

    if (papar__isws(*c))
      c = papar__lexer_consume_whitespace(c, tokenlist);
    else if (isalpha(*c))
      c = papar__lexer_consume_command(c, tokenlist);
    else if (isdigit(*c) || *c == '-' || *c == '+')
      c = papar__lexer_consume_number(c, tokenlist);
    else if (*c == ',')
      c = papar__lexer_consume_comma(c, tokenlist);
    else {
      tokenlist->has_error = true;
      tokenlist->error_start = c;
      tokenlist->error_end = c+1;
    }

    if (tokenlist->has_error) {
      if (tokenlist->error_offset < 0) // Not enough memory.
        return;

      tokenlist->error_offset = c - d + 1;
      break;
    }
  } while(true);
}

bool papar__isws(char c)
{
  // Not sure if we can use `isspace()` since it also includes '\v' and '\f'.
  switch (c) {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
      return true;

    default:
      return false;
  }
}

const char *papar__lexer_consume_whitespace(const char *c, papar_tokenlist *tokenlist)
{
  while (papar__isws(*c)) { c++; };
  return c;
}


const char *papar__lexer_consume_command(const char *c, papar_tokenlist *tokenlist)
{
  papar_tokenlist_push(tokenlist, papar_token_new(c, 1, PAPAR_TOK_COMMAND));
  return ++c;
}

const char *papar__lexer_consume_number(const char *c, papar_tokenlist *tokenlist)
{
  const char *n = c;
  uint8_t flags = 0;
  do {
    if (*n == '-' || *n == '+') {
      if (n == c) {
        ++n;
        continue;
      } else goto papar_err_jump;
    }

    if (isdigit(*n)) {
      ++n;
      continue;
    }

    if (*n == '.') {
      if (flags & PAPAR_NUM_HAS_DOT || flags & PAPAR_NUM_HAS_E) {
        goto papar_err_jump;
      } else {
        flags |= PAPAR_NUM_HAS_DOT;
        ++n;

        if (!isdigit(*n)) goto papar_err_jump;

        continue;
      }
    }

    if (*n == 'e' || *n == 'E') {
      if (flags & PAPAR_NUM_HAS_E)
        goto papar_err_jump;
      else {
        flags |= PAPAR_NUM_HAS_E;
        ++n;

        if (isdigit(*n)) continue;

        if (*n == '-' || *n == '+') {
          ++n;
          if (!isdigit(*n)) goto papar_err_jump;
        }

        continue;
      }
    }

    break;
  } while(true);

  papar_tokenlist_push(tokenlist, papar_token_new(c, n - c, PAPAR_TOK_NUMBER));
  return n;

  papar_err_jump:
    tokenlist->has_error = true;
    tokenlist->error_start = c;
    tokenlist->error_end = n+1;
    return n;
}

const char *papar__lexer_consume_comma(const char *c, papar_tokenlist *tokenlist)
{
  papar_tokenlist_push(tokenlist, papar_token_new(c, 1, PAPAR_TOK_COMMA));
  return ++c;
}
