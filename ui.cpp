// SPDX-License-Identifier: zlib-acknowledgement

#include "base-inc.h"

#include <stdio.h>

int
main(int argc, char *argv[])
{
  MEM_ARENA_TEMP_BLOCK(some_arena, scratch_arena)
  {

  }
  printf("%ld\n", sizeof(memory_index));
  return 0;
}

// as recieves arena, will be allocating permanent results to caller
func(MemArena *arena) -> Node *
// so, if doing temp work, won't pass in, rather use global temp?

// linear allocator
// dynamic with linked list of arenas?

// IMPORTANT(Ryan): If need variable lifetimes, can build atop of arena with a free-list/pool allocator
