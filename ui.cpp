// SPDX-License-Identifier: zlib-acknowledgement

#include "base-inc.h"

#include "raylib.h"

// TODO(Ryan): ranger previewing (probably have to reinstall python to get ueberzug!)
// TODO(Ryan): git blame and git content inside vim
// git blame -w -M3 -- file | grep line-num) --> obtain commit-hash
// !hash=$(git --no-pager blame -w -M3 -L 17,17 -- ui.cpp | awk '{print $1}'); git difftool "$hash"^.."$hash" -- ui.cpp
// git difftool 0173b443^..0173b443 -- <file>
// git show commit-hash

int
main(int argc, char *argv[])
{
  global_debugger_present = linux_was_launched_by_gdb();

  InitAudioDevice();
  Sound sound = LoadSound("the-tower-of-dreams.ogg");
  PlaySound(sound);

  sleep(1);

#if 0
  MEM_ARENA_TEMP_BLOCK(some_arena, scratch_arena)
  {

  }
#endif

  return 0;
}

// as recieves arena, will be allocating permanent results to caller
//func(MemArena *arena) -> Node *
// so, if doing temp work, won't pass in, rather use global temp?

// linear allocator
// dynamic with linked list of arenas?

// IMPORTANT(Ryan): If need variable lifetimes, can build atop of arena with a free-list/pool allocator
