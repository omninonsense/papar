#include "papar.h"

papar_state *papar_state_new(size_t initial_capacity)
{
  if (initial_capacity < PAPAR_STATECMD_MIN_CAP)
    initial_capacity = PAPAR_STATECMD_MIN_CAP;

  papar_state *self = (papar_state*) malloc(sizeof(papar_state));

  self->tokenlist = NULL;
  self->position = 0;
  self->size = 0;
  self->capacity = initial_capacity;

  self->commands = (papar_command*) calloc(self->capacity, sizeof(papar_command));
  if (self->commands == NULL) {
    free(self);
    return NULL;
  }

  return self;
}

void papar_state_free(papar_state *self)
{
  if (self != NULL) {
    if (self->commands != NULL)
      free(self->commands);

    free(self);
  }
}

int papar_state_push(papar_state *self, papar_command command)
{
  if (self->size >= self->capacity) {
    size_t amount = self->capacity * PAPAR_STATECMD_GRWOTH_RATE;
    if (papar__state_grow(self, amount)) {
      self->error_type = PAPAR_ERR_MEM;
      return 1;
    }
  }

  self->commands[self->size++] = command;
  return 0;
}

papar_command *papar_state_pop(papar_state *self) {
  if (self->size <= 0)
    return NULL;

  return &self->commands[self->size--];
}

int papar__state_grow(papar_state *self, size_t amount)
{
  size_t new_cap = amount + self->capacity;
  papar_command *tmp = (papar_command*) realloc(self->commands, new_cap * sizeof(papar_command));

  if (tmp == NULL)
    return 1;

  self->capacity = new_cap;
  self->commands = tmp;

  return 0;
}

/* [M, m] [Z, z] [L, l] [H, h] [V, v] [C, c] [S, s] [Q, q] [T, t] [A, a] */
void papar_parse(papar_state *self, const papar_tokenlist *tokenlist)
{
  char cmd;
  if (papar__parser_expect(self, tokenlist, 0, 1, PAPAR_TOK_COMMAND)) return;

  cmd = *PAPAR_TOK_BEG(self, tokenlist, 0);

  ++self->position;
  papar__parser_parse_command(self, tokenlist, cmd);

  while(!self->error_type) {
    enum papar_token_type ct = papar__parser_peek(self, tokenlist, 0);
    const char *ctname = papar_token_type_lookup[ct];

    PAPAR_DEBUG("Encountered token: %s", papar_token_type_lookup[ct]);

    switch (ct) {
      case PAPAR_TOK_COMMAND:
        cmd =  *PAPAR_TOK_BEG(self, tokenlist, 0);
        ++self->position;
        papar__parser_parse_command(self, tokenlist, cmd);
        continue;
      break;

      case PAPAR_TOK_NUMBER:
        if (self->last_command == 'M')
          papar__parser_parse_command(self, tokenlist, 'L');
        else if (self->last_command == 'm')
          papar__parser_parse_command(self, tokenlist, 'l');
        else if (strchr("Zz", self->last_command))
          papar__parser_expect(self, tokenlist, 0, 1, PAPAR_TOK_COMMAND);
        else
          papar__parser_parse_command(self, tokenlist, self->last_command);

        continue;
      break;

      // Stop parsing at the end
      case PAPAR_TOK_EOF:
      return;

      default:
        PAPAR_ERROR("Unexpected token %s at position %zd", ctname, PAPAR_TOK_BEG(self, tokenlist, 0) - tokenlist->src);
        self->error_type = PAPAR_ERR_PAR;
        return;
      break;
    }

    ++self->position;
  }
}

void papar__parser_parse_command(papar_state *self, const papar_tokenlist *tl, char c)
{
  PAPAR_DEBUG("Processing command: %c", c);
  self->last_command = c;
  switch (c) {
    // Move to
    case 'M':
    case 'm':
      papar__parser_parse_goto(self, tl);
    break;

    // Close path
    case 'Z':
    case 'z':
      papar__parser_parse_closepath(self, tl);
    break;

    // Line to
    case 'L':
    case 'l':
      papar__parser_parse_lineto(self, tl);
    break;

    // Horizontal line to
    case 'H':
    case 'h':
      papar__parser_parse_horizontallineto(self, tl);
    break;

    // Vertival line to
    case 'V':
    case 'v':
      papar__parser_parse_verticallineto(self, tl);
    break;

    // Cubic Bézier
    case 'C':
    case 'c':
      papar__parser_parse_3bezierto(self, tl);
    break;

    // Smooth cubic Bézier
    case 'S':
    case 's':
      papar__parser_parse_smooth3bezierto(self, tl);
    break;

    // Quadratic Bézier
    case 'Q':
    case 'q':
      papar__parser_parse_2bezierto(self, tl);
    break;

    // Smooth quadratic Bézier
    case 'T':
    case 't':
      papar__parser_parse_smooth2bezierto(self, tl);
    break;

    // Elliptical arc
    // @NOTE: SVG 1.2 Tiny doesn't seem to support this
    case 'A':
    case 'a':
      papar__parser_parse_ellipticalarc(self, tl);
    break;

    default:
      PAPAR_ERROR("Unknown command: %c at position %zd", c, PAPAR_TOK_BEG(self, tl, 0) - tl->src);
      self->error_type = PAPAR_ERR_PAR;
    break;
  }
}

enum papar_token_type papar__parser_peek(papar_state *self, const papar_tokenlist *tl, ssize_t offset)
{
  if (tl->size <= self->position + offset)
    return PAPAR_TOK_EOF;

  return PAPAR_TOK_TYPE(self, tl, offset);
}

int papar__parser_expect(papar_state *self, const papar_tokenlist *tl, ssize_t offset, size_t count, ...)
{
  va_list vargs;
  size_t i;
  enum papar_token_type actual_type, expected_type;

  va_start(vargs, count);
  for (i = 0; i < count; ++i) {
    actual_type = papar__parser_peek(self, tl, offset+i);
    expected_type = va_arg(vargs, enum papar_token_type);

    // Flags are marked as numbers by the lexer
    // So we do additional checking here.
    if (expected_type == PAPAR_TOK_FLAG && actual_type == PAPAR_TOK_NUMBER) {
      if (PAPAR_TOK_LEN(self, tl, offset+i) != 1 && !(*PAPAR_TOK_BEG(self, tl, offset+i) == '0' || *PAPAR_TOK_BEG(self, tl, offset+i) == '1'))
        goto papar_unexpectedtokerr_jump;
    } else if (actual_type != expected_type)
      goto papar_unexpectedtokerr_jump;
  }

  va_end(vargs);
  return 0;

papar_unexpectedtokerr_jump:
  PAPAR_ERROR("Expected token type %s, but got %s (`%.*s`) at position %zd",
    papar_token_type_lookup[expected_type],                              // %s
    papar_token_type_lookup[actual_type],                                // %s
    (int)PAPAR_TOK_LEN(self, tl, offset+i), PAPAR_TOK_BEG(self, tl, offset+i), // %.*s
    PAPAR_TOK_BEG(self, tl, offset+i) - tl->src                             // %zd
  );

  self->error_type = PAPAR_ERR_PAR;
  va_end(vargs);
  return 1;
}

void papar__parser_parse_goto(papar_state *self, const papar_tokenlist *tl)
{
  papar_command cmd = {0};
  cmd.type = self->last_command;

  papar__parser_consume_point(self, tl, &cmd.x, &cmd.y);
  papar_state_push(self, cmd);
}

void papar__parser_parse_closepath(papar_state *self, const papar_tokenlist *tl)
{
  papar_command cmd = {0};
  cmd.type = self->last_command;

  papar_state_push(self, cmd);
}

void papar__parser_parse_lineto(papar_state *self, const papar_tokenlist *tl)
{
  papar_command cmd = {0};
  cmd.type = self->last_command;

  papar__parser_consume_point(self, tl, &cmd.x, &cmd.y);
  papar_state_push(self, cmd);
}

void papar__parser_parse_horizontallineto(papar_state *self, const papar_tokenlist *tl)
{
  papar_command cmd = {0};
  cmd.type = self->last_command;

  papar__parser_consume_number(self, tl, &cmd.x);
  papar_state_push(self, cmd);
}

void papar__parser_parse_verticallineto(papar_state *self, const papar_tokenlist *tl)
{
  papar_command cmd = {0};
  cmd.type = self->last_command;

  papar__parser_consume_number(self, tl, &cmd.y);
  papar_state_push(self, cmd);
}

void papar__parser_parse_3bezierto(papar_state *self, const papar_tokenlist *tl)
{
  papar_command cmd = {0};
  cmd.type = self->last_command;

  papar__parser_consume_point(self, tl, &cmd.x1, &cmd.y1);
  papar__parser_consume_point(self, tl, &cmd.x2, &cmd.y2);
  papar__parser_consume_point(self, tl, &cmd.x, &cmd.y);
  papar_state_push(self, cmd);
}

void papar__parser_parse_smooth3bezierto(papar_state *self, const papar_tokenlist *tl)
{
  papar_command cmd = {0};
  cmd.type = self->last_command;

  papar__parser_consume_point(self, tl, &cmd.x2, &cmd.y2);
  papar__parser_consume_point(self, tl, &cmd.x, &cmd.y);
  papar_state_push(self, cmd);
}

void papar__parser_parse_2bezierto(papar_state *self, const papar_tokenlist *tl)
{
  papar_command cmd = {0};
  cmd.type = self->last_command;

  papar__parser_consume_point(self, tl, &cmd.x1, &cmd.y1);
  papar__parser_consume_point(self, tl, &cmd.x, &cmd.y);
  papar_state_push(self, cmd);
}

void papar__parser_parse_smooth2bezierto(papar_state *self, const papar_tokenlist *tl)
{
  papar_command cmd = {0};
  cmd.type = self->last_command;

  papar__parser_consume_point(self, tl, &cmd.x, &cmd.y);
  papar_state_push(self, cmd);
}

void papar__parser_parse_ellipticalarc(papar_state *self, const papar_tokenlist *tl)
{
  papar_command cmd = {0};
  bool large_arc = false, sweep = false;
  cmd.type = self->last_command;

  papar__parser_consume_point(self, tl, &cmd.x1, &cmd.y1); // rx, ry
  papar__parser_consume_number(self, tl, &cmd.x2);         // x-axis-rotation

  papar__parser_consume_flag(self, tl, &large_arc);
  papar__parser_consume_flag(self, tl, &sweep);

  if (large_arc) cmd.flags |= PAPAR_LARGE_ARC_FLAG;
  if (sweep)     cmd.flags |= PAPAR_SWEEP_FLAG;

  papar__parser_consume_point(self, tl, &cmd.x, &cmd.y);

  papar_state_push(self, cmd);
}

void papar__parser_consume_number(papar_state *self, const papar_tokenlist *tl, double *number)
{
  if (papar__parser_expect(self, tl, 0, 1, PAPAR_TOK_NUMBER)) return;

  *number = strtod(PAPAR_TOK_BEG(self, tl, 0), NULL);

  ++self->position;
}

void papar__parser_consume_flag(papar_state *self, const papar_tokenlist *tl, bool *flag)
{

  if (papar__parser_expect(self, tl, 0, 1, PAPAR_TOK_FLAG)) return;

  *flag = (*PAPAR_TOK_BEG(self, tl, 0) == '1');

  ++self->position;
}

void papar__parser_consume_point(papar_state *self, const papar_tokenlist *tl, double *x, double *y)
{
  if (papar__parser_expect(self, tl, 0, 3, PAPAR_TOK_NUMBER, PAPAR_TOK_COMMA, PAPAR_TOK_NUMBER)) return;

  //@IDEA: Maybe check how much characters strtod() consumed
  //       to double-check if the lexer read the number properly?
  *x = strtod(PAPAR_TOK_BEG(self, tl, 0), NULL);
  *y = strtod(PAPAR_TOK_BEG(self, tl, 2), NULL);

  self->position += 3;
}
