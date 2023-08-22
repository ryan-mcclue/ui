// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

// IMPORTANT(Ryan): Although using memory arenas restricts arbitrary lifetimes, this provides more benefits than negatives.
// In majority of cases, a large number of allocations can be bucketed into same arena

// IMPORTANT(Ryan): In essence, OS is ultimate garbage collector as it releases page table for us.
// So, we generally shouldn't have to perform manual garbage collection.
// That is, no periodic code interuptions to determine lifetimes and possibly free

#include <string.h>
#include <stdlib.h>

#define MEMORY_ZERO(p, n) memset((p), 0, (n))
#define MEMORY_ZERO_STRUCT(p) MEMORY_ZERO((p), sizeof(*(p)))
#define MEMORY_ZERO_ARRAY(a) MEMORY_ZERO((a), sizeof(a[0]))

#define MEMORY_COPY(d, s, n) memmove((d), (s), (n))
#define MEMORY_COPY_STRUCT(d, s, n) MEMORY_COPY((d), (s), sizeof(*(s)))
#define MEMORY_COPY_ARRAY(d, s, n) MEMORY_COPY((d), (s), sizeof((s)))

#define MEMORY_MATCH(a, b, n) (memcmp((a), (b), (n)) == 0)

#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)
#define GB(x) (((u64)x) << 30)
#define TB(x) (((u64)x) << 40)

typedef struct MemArena MemArena;
struct MemArena
{
  void *memory;
  memory_index commit_pos;
  memory_index max;
  memory_index pos;
  memory_index align;
};

INTERNAL MemArena *
mem_arena_allocate(memory_index cap, memory_index roundup_granularity)
{
  u64 rounded_size = memory_index_round_to_nearest(cap, roundup_granularity);
  MemArena *result = (MemArena *)malloc(rounded_size);
  if (result == NULL)
  {
    FATAL_ERROR("Result", strerror(errno), "restart");
  }

  result->memory = result + sizeof(MemArena);
  result->max = cap;
  result->pos = sizeof(MemArena);
  result->align = sizeof(memory_index);

  return result;
}

INTERNAL void
mem_arena_deallocate(MemArena *arena)
{
  free(arena);
}
 
#define MEM_ARENA_PUSH_ARRAY(a,T,c) (T*)mem_arena_push((a), sizeof(T)*(c))
#define MEM_ARENA_PUSH_ARRAY_ZERO(a,T,c) (T*)mem_arena_push_zero((a), sizeof(T)*(c))
#define MEM_ARENA_POP_ARRAY(a,T,c) mem_arena_pop((a), sizeof(T)*(c))

#define MEM_ARENA_PUSH_STRUCT(a,T) (T*)mem_arena_push((a), sizeof(T))
#define MEM_ARENA_PUSH_STRUCT_ZERO(a,T) (T*)mem_arena_push_zero((a), sizeof(T))

#define MEM_ARENA_TEMP_BLOCK(arena, name) \
  MemArenaTemp name = ZERO_STRUCT; \
  DEFER_LOOP(name = mem_arena_temp_begin(arena), mem_arena_temp_end(name))


INTERNAL void *
mem_arena_push_aligned(MemArena *arena, memory_index size, memory_index align)
{
  void *result = NULL;

  memory_index clamped_align = CLAMP_BOTTOM(align, arena->align);

  memory_index pos = arena->pos;

  memory_index pos_address = INT_FROM_PTR(arena) + pos;
  memory_index aligned_pos = ALIGN_POW2_UP(pos_address, clamped_align);
  memory_index alignment_size = aligned_pos - pos_address;

  if (pos + alignment_size + size <= arena->max)
  {
    u8 *mem_base = (u8 *)arena;
    result = mem_base + pos + alignment_size;
    memory_index new_pos = pos + alignment_size + size;
    arena->pos = new_pos;
  }

  return result;
}

INTERNAL void *
mem_arena_push(MemArena *arena, memory_index size)
{
  return mem_arena_push_aligned(arena, size, arena->align);
}

INTERNAL void *
mem_arena_push_zero(MemArena *arena, memory_index size)
{
  void *memory = mem_arena_push(arena, size);

  MEMORY_ZERO(memory, size);

  return memory;
}

INTERNAL void
mem_arena_set_pos_back(MemArena *arena, memory_index pos)
{
  memory_index clamped_pos = CLAMP_BOTTOM(sizeof(*arena), pos);

  if (arena->pos > clamped_pos)
  {
    arena->pos = clamped_pos;
  }
}

INTERNAL void
mem_arena_pop(MemArena *arena, memory_index size)
{
  mem_arena_set_pos_back(arena, arena->pos - size);
}

INTERNAL void
mem_arena_clear(MemArena *arena)
{
  mem_arena_pop(arena, arena->pos);
}

typedef struct MemArenaTemp MemArenaTemp;
struct MemArenaTemp
{
  MemArena *arena;
  memory_index pos;
};

INTERNAL MemArenaTemp
mem_arena_temp_begin(MemArena *arena)
{
  MemArenaTemp temp = ZERO_STRUCT;
  temp.arena = arena;
  temp.pos = arena->pos;
  return temp;
}

INTERNAL void
mem_arena_temp_end(MemArenaTemp temp)
{
  mem_arena_set_pos_back(temp.arena, temp.pos);
}
