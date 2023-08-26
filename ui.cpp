// SPDX-License-Identifier: zlib-acknowledgement

#include "base-inc.h"

#include "raylib.h"

// TODO(Ryan): ranger previewing (probably have to reinstall python to get ueberzug!)

GLOBAL RingBuf global_frame_buf;

INTERNAL void
callback(void *bufferData, unsigned int frames)
{
  // could be in a different thread!
  // audio samples are float normalised!
  U8Buf frame_buf = u8buf(bufferData, frames * sizeof(f64));
  memory_index bytes_written = ring_buf_write(&global_frame_buf, frame_buf);

#if 0
  for (u32 frame_i = 0; frame_i < frames; frame_i += 1)
  {
    printf("sample: %f\n", *(f32 *)((f64 *)bufferData + frame_i)); 
  }
#endif
  //printf("cursor: %lu, bytes: %lu, size: %lu\n", global_frame_buf.write_pos, bytes_written, global_frame_buf.size); 
}

int
main(int argc, char *argv[])
{
  global_debugger_present = linux_was_launched_by_gdb();

  MemArena *perm_arena = mem_arena_allocate(GB(1), GB(1));
  
  // TODO(Ryan): Change to passing type, and having name be Buf to avoid u8 -> f64 type confusions with size etc. 
  global_frame_buf = ring_buf_create(perm_arena, (memory_index)(44800 * 0.1f) * sizeof(f64));

  InitWindow(800, 600, "visualiser");
  SetTargetFPS(60);

  InitAudioDevice();
  Music music = LoadMusicStream("the-tower-of-dreams.ogg");
  ASSERT(music.stream.sampleSize == 16);
  ASSERT(music.stream.channels == 2);
  SetMusicVolume(music, 0.25f);

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

    s32 mid_y = GetRenderHeight() / 2.0;
    s32 bar_w = CLAMP(1, GetRenderWidth() / ((s32)global_frame_buf.size / sizeof(f64)), GetRenderWidth());
    for (u32 frame_i = 0; frame_i < global_frame_buf.size / sizeof(f64); frame_i += 1)
    {
      //if (frame_i > GetRenderWidth()) break;
      f32 *samples = (f32 *)(((f64 *)global_frame_buf.content) + frame_i);
      f32 left_sample = *samples;

      // slows program down immensely
      // if (left_sample != 0.0f) printf("sample: %f\n", left_sample);
      //printf("sample: %f\n", left_sample);

      s32 bar_x = frame_i * bar_w;
      s32 bar_h = left_sample * (mid_y / 2.0f); 
      //printf("x: %d, y: %d, w: %d, h: %d\n", bar_x, mid_y, bar_w, bar_h);
      DrawRectangle(bar_x, mid_y - bar_h, bar_w, bar_h, RED);
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
