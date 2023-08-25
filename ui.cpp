// SPDX-License-Identifier: zlib-acknowledgement

#include "base-inc.h"

#include "raylib.h"

// TODO(Ryan): ranger previewing (probably have to reinstall python to get ueberzug!)

#if 0
INTERNAL memory_index
ring_write(u8 *base, memory_index size, memory_index write_pos, U8Arr arr)
{
  memory_index first_part_write_off = write_pos % size;
  memory_index second_part_write_off = 0;

  U8Arr first_part = arr;
  U8Arr second_part = u8_arr("");

  if(first_part_write_off + string.size > size)
  {
    first_part.size = size - first_part_write_off;
    second_part = str8_advance(arr, first_part.size);
  }
  if(first_part.size != 0)
  {
    // IMPORTANT(Ryan): Confusion with stdlib bytes, elements, etc. Encouraging buffer overflows!
    MEMORY_COPY(base + first_part_write_off, first_part.str, first_part.size);
  }
  if(second_part.size != 0)
  {
    MemoryCopy(base + second_part_write_off, second_part.str, second_part.size);
  }
  return arr.size;
}

INTERNAL memory_index
ring_read(void *dst, U8 *base, memory_index buffer_size, memory_index read_pos, memory_index read_size)
{
 memory_index first_part_read_off = read_pos % buffer_size;
 memory_index second_part_read_off = 0;
 memory_index first_part_read_size = read_size;
 memory_index second_part_read_size = 0;
 if(first_part_read_off + read_size > buffer_size)
 {
  first_part_read_size = buffer_size - first_part_read_off;
  second_part_read_size = read_size - first_part_read_size;
 }
 if(first_part_read_size != 0)
 {
  MemoryCopy(dst, base + first_part_read_off, first_part_read_size);
 }
 if(second_part_read_size != 0)
 {
  MemoryCopy((U8 *)dst + first_part_read_size, base + second_part_read_off, second_part_read_size);
 }
 return read_size;
}
#endif

GLOBAL s32 global_frames[1024];
GLOBAL u32 global_frames_count;

INTERNAL void
callback(void *bufferData, unsigned int frames)
{
  CLAMP(0, frames, ARRAY_COUNT(global_frames));

  // could be in a different thread!
  MEMORY_COPY(global_frames, bufferData, sizeof(s32) * frames);
  global_frames_count = frames;
}

int
main(int argc, char *argv[])
{
  global_debugger_present = linux_was_launched_by_gdb();

  InitWindow(800, 600, "visualiser");
  SetTargetFPS(60);

  InitAudioDevice();
  Music music = LoadMusicStream("the-tower-of-dreams.ogg");
  ASSERT(music.stream.sampleSize == 16);
  ASSERT(music.stream.channels == 2);

  PlayMusicStream(music);
  AttachAudioStreamProcessor(music.stream, callback);

  while (!WindowShouldClose())
  {
    UpdateMusicStream(music);

    if (IsKeyPressed(KEY_SPACE))
    {
      if (IsMusicStreamPlaying(music)) PauseMusicStream(music);
      else ResumeMusicStream(music);
    }

    BeginDrawing();
    Color smoke = {0x18, 0x18, 0x18, 0xff};
    ClearBackground(smoke);

    u32 middle = GetRenderWidth();
    for (u32 frame_i = 0; frame_i < global_frames_count; frame_i += 1)
    {
      s16 *samples = (s16 *)&global_frames[frame_i];
      s16 left_sample = *samples; 
      s16 right_sample = *(samples + 1);
    }

    EndDrawing();
  }

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
