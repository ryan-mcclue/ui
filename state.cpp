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
// Effectively, table conversion from giant switch

// C++ allows in-order index designated initialiser (NOT OUT-OF-ORDER)
// This is can be cumbersome to create, so often compiled down to this (could just use xmacros?)
IGNORE_WARNING_PEDANTIC()
State fsm[STATE_COUNT][EVENT_COUNT] = {
  [STATE_LOCKED] = {STATE_LOCKED, STATE_UNLOCKED},
  [STATE_UNLOCKED] = {STATE_LOCKED, STATE_UNLOCKED}, 
};
// in reality though, elements could be struct {next_state, action_to_take}
char *state_str[STATE_COUNT] = {
  [STATE_LOCKED] = "LOCKED",
  [STATE_UNLOCKED] = "UNLOCKED",
};
IGNORE_WARNING_POP()

INTERNAL char *
state_to_str(State s) { return state_str[s]; }

INTERNAL State
next_state(State state, Event ev) { return fsm[state][ev]; }

typedef struct State
{
  u32 event_to_state[127];
} State;

INTERNAL void
fill_state(State *state, Range1U32 event_range, u32 state_index)
{

}

enum {
  STATE_ERROR,
  STATE_START,
  STATE_SECOND,
};

INTERNAL b32
match_fsm(Fsm fsm, String8 str)
{
  STATE state_id = STATE_START;
  for (u32 i = 0; i < str.size; i += 1)
  {
    if (state_id == STATE_ERROR) return false;

    state_id = fsm[state_id][str.content[i]];
  }
}

// the regex string constructs a particular fsm
INTERNAL void
init_fsm(String8 regex) // regex_compile(str8_lit("abc"));
{
  State fsm[100] = ZERO_STRUCT;
  // NOTE(Ryan): Failed state is index 0

  for (u32 i = 0; i < regex.size; i += 1)
  {
    char ch = regex.content[i];
    if (ch == '$') fill_state(&fsm[i], NEWLINE_CHAR, i + 1);
    if (ch == '.') fill_state(&fsm[i], range1_u32('a', 'z'), i + 1);
  }

  // TODO(Ryan): rect and ranges in math
}

INTERNAL void
dump_state(State state)
{
  for (u32 i = 0; i < NUM_EVENTS; i += 1)
  {
    printf("%s => %d", event_type, state.event_to_state[i]);
    // index returned will be next state
  }
}

// state = fsm_update[state][EVENT]();

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
