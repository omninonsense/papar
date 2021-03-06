#include "papar.h"

papar_token papar_token_new(const char* start, size_t length, enum papar_token_type type)
{
  papar_token token;

  token.start = start;
  token.length = length;
  token.type = type;

  return token;
}

papar_tokenlist *papar_tokenlist_new(size_t initial_capacity)
{
  if (initial_capacity < 10)
    initial_capacity = 10;

  papar_tokenlist *self = (papar_tokenlist*) malloc(sizeof(papar_tokenlist));

  self->error_type = PAPAR_ERR_OK;
  self->error_start = NULL;
  self->error_end = NULL;

  self->src = NULL;

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
      PAPAR_ERROR("Could not allocate %zd memory for token list.", self->capacity+amount);
      self->error_type = PAPAR_ERR_MEM;
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

void papar_lex(papar_tokenlist *self, const char *d) {
  const char *c = d;
  self->src = d;

  do {
    if (*c == 0) {
      papar_tokenlist_push(self, papar_token_new(c, 0, PAPAR_TOK_EOF));
      break;
    }

    if (papar__isws(*c))
      c = papar__lexer_consume_whitespace(self, c);
    else if (isalpha(*c))
      c = papar__lexer_consume_command(self, c);
    else if (isdigit(*c) || *c == '-' || *c == '+')
      c = papar__lexer_consume_number(self, c);
    else if (*c == ',')
      c = papar__lexer_consume_comma(self, c);
    else {
      self->error_type = PAPAR_ERR_TOK;
      self->error_start = c;
      self->error_end = c+1;
    }
  } while(!self->error_type);

  if (self->error_type == PAPAR_ERR_TOK)
    PAPAR_ERROR("Encountered error while tokenizing, couldn't process token `%.*s` at offset %zd", (int)(self->error_end - self->error_start), self->error_start, self->error_start - self->src);
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

const char *papar__lexer_consume_whitespace(papar_tokenlist *self, const char *c)
{
  while (papar__isws(*c)) { c++; };
  return c;
}


const char *papar__lexer_consume_command(papar_tokenlist *self, const char *c)
{
  papar_tokenlist_push(self, papar_token_new(c, 1, PAPAR_TOK_COMMAND));
  return ++c;
}

const char *papar__lexer_consume_number(papar_tokenlist *self, const char *c)
{
  const char *n = c;
  uint8_t flags = 0;

  if (*n == '-' || *n == '+')
    ++n;

  if (!isdigit(*n)) goto papar_lexnumerr_jump;

  do {
    if (isdigit(*n)) {
      ++n;
      continue;
    } else if (*n == '.') {
      if (flags & PAPAR_NUM_HAS_DOT || flags & PAPAR_NUM_HAS_E) {
        goto papar_lexnumerr_jump;
      } else {
        flags |= PAPAR_NUM_HAS_DOT;
        ++n;

        if (!isdigit(*n)) goto papar_lexnumerr_jump;

        continue;
      }
    } else if (*n == 'e' || *n == 'E') {
      if (flags & PAPAR_NUM_HAS_E)
        goto papar_lexnumerr_jump;
      else {
        flags |= PAPAR_NUM_HAS_E;
        ++n;

        if (isdigit(*n)) continue;
        else if (*n == '-' || *n == '+') {
          ++n;
          if (!isdigit(*n)) goto papar_lexnumerr_jump;
          continue;
        } else goto papar_lexnumerr_jump;
      }
    } else if (papar__isws(*n) || *n == ',' || *n == 0) // Stop reading number on next token or end of stream
      break;
    else goto papar_lexnumerr_jump;

  } while(true);

  papar_tokenlist_push(self, papar_token_new(c, n - c, PAPAR_TOK_NUMBER));
  return n;

  papar_lexnumerr_jump:
    self->error_type = PAPAR_ERR_TOK;
    self->error_start = c;
    self->error_end = n+1;
    return n;
}

const char *papar__lexer_consume_comma(papar_tokenlist *self, const char *c)
{
  papar_tokenlist_push(self, papar_token_new(c, 1, PAPAR_TOK_COMMA));
  return ++c;
}
