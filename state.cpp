// SPDX-License-Identifier: zlib-acknowledgement

// TODO(Ryan): Replace style compatible for C/C++
typedef enum State
{
  STATE_LOCKED,
  STATE_UNLOCKED,
  STATE_COUNT,
} State;

typedef enum Event
{
  EVENT_PUSH,
  EVENT_COIN,
  EVENT_COUNT,
} Event;

// A simple transition function(state, event) that switches becomes cumbersome with more parameters
// Instead 2D array[state][event]; value will be transition to next state

// C++ allows in-order index designated initialiser (NOT OUT-OF-ORDER)
// This is can be cumbersome to create, so often compiled down to this (could just use xmacros?)
IGNORE_WARNING_PEDANTIC()
State fsm[STATE_COUNT][EVENT_COUNT] = {
  [STATE_LOCKED] = {STATE_LOCKED, STATE_UNLOCKED},
  [STATE_UNLOCKED] = {STATE_LOCKED, STATE_UNLOCKED}, 
};
IGNORE_WARNING_POP()
// in reality though, elements could be struct {next_state, action_to_take}

INTERNAL char *
state_to_str(State s)
{
  switch (s)
  {

  }
}

INTERNAL State
next_state(State state, Event ev)
{
  return fsm[state][ev];
}

#if 0
INTERNAL State
next_state(State state, Event ev)
{
  switch (state)
  {
    NO_DEFAULT_CASE;
    case STATE_LOCKED:
    {
      if (ev == EVENT_PUSH) return STATE_LOCKED;
      if (ev == EVENT_COIN) return STATE_UNLOCKED;
    } break;
    case STATE_UNLOCKED:
    {
      if (ev == EVENT_PUSH) return STATE_LOCKED;
      if (ev == EVENT_COIN) return STATE_UNLOCKED;
    } break;
  }

  return STATE_LOCKED;
}
#endif
