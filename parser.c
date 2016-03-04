#include "papar.h"

/* [M, m] [Z, z] [L, l] [H, h] [V, v] [C, c] [S, s] [Q, q] [T, t] [A, a] */

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

void papar_parse(const papar_tokenlist *tokenlist, papar_state *state)
{
  // @TODO
}
