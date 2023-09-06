// SPDX-License-Identifier: zlib-acknowledgement

#include "base-inc.h"

#include "raylib.h"

// TODO(Ryan): For networking applications: https://github.com/Jai-Community/awesome-jai 
// https://github.com/ValveSoftware/GameNetworkingSockets/blob/master/examples/trivial_signaling_client.cpp

// vectorised fft: https://github.com/ValveSoftware/ffts

// TODO(Ryan): ranger previewing (probably have to reinstall python to get ueberzug!)

#define EACH_ELEMENT(arr, it) memory_index it = 0; it < ARRAY_COUNT(arr); it += 1

#define N 256 // 1 << 14; roughly 20kHz
GLOBAL f32 global_in[N];
GLOBAL f32z global_out[N];
GLOBAL f32 global_max_amp;

typedef struct Frame Frame;
struct Frame
{
  f32 left, right;
};

INTERNAL f32
ft_amp(f32z in)
{
  f32 cos_amp = f32_abs(f32z_real(in));
  f32 sin_amp = f32_abs(f32z_imaginary(in));

  return MAX(cos_amp, sin_amp);
}

// NOTE(Ryan): O(n^2)
INTERNAL void
dft(f32 input[], f32z output[], u32 n)
{
  for (u32 f = 0; f <= n; f += 1)
  {
    output[f] = 0;
    for (u32 i = 0; i < n; i += 1)
    {
      f32 t = (f32)i / n;
      // NOTE(Ryan): Use Eulers formula to simulataneously compute sin and cos 
      output[f] += input[i] * f32z_exp(F32_TAU * f * t * f32z_I);
    }
  }
}

// NOTE(Ryan): O(nlogn)
// Ported from https://rosettacode.org/wiki/Fast_Fourier_transform#Python
INTERNAL void
fft(f32 input[], u32 stride, f32z output[], u32 n)
{
  ASSERT(IS_POW2(n));

  if (n == 1) 
  {
    output[0] = input[0];
    return;
  }

  fft(input, stride*2, output, n/2);
  fft(input + stride, stride*2, output + n/2, n/2);

  for (u32 k = 0; k < n/2; ++k)
  {
    f32 t = (f32)k/n;
    f32z v = f32z_exp(-F32_TAU*f32z_I*t)*output[k + n/2];
    f32z e = output[k];
    output[k] = e + v;
    output[k + n/2] = e - v;
  }
}

INTERNAL void
callback(void *bufferData, unsigned int frames)
{
  // could be in a different thread!
  // audio samples are float normalised!
  
  if (frames < N) return;

  for (u32 i = 0; i < N; i += 1)
  {
    Frame frame = *((Frame *)bufferData + i);
    global_in[i] = frame.left; 
  }

  fft(global_in, 1, global_out, N);

  global_max_amp = 0.0f;
  for (u32 i = 0; i < N; i += 1)
  {
    f32 amp = ft_amp(global_out[i]);
    if (global_max_amp < amp) global_max_amp = amp;
  }
}

INTERNAL void
linux_set_cwd_to_self(MemArena *arena)
{
  MEM_ARENA_TEMP_BLOCK(arena, scratch)
  {
    String8 binary_path = str8_allocate(scratch.arena, 128);
    s32 binary_path_size = readlink("/proc/self/exe", (char *)binary_path.content, binary_path.size);
    if (binary_path_size == -1)
    {
      WARN("Failed to get binary path\n\t%s\n", strerror(errno));
    }
    else
    {
      // IMPORTANT(Ryan): readlink() won't append NULL byte, so no need to subtract
      binary_path.size = binary_path_size; 

      memory_index last_slash = str8_find_substring(binary_path, str8_lit("/"), 0, MATCH_FLAG_FIND_LAST);
      binary_path.size = last_slash;

      char *binary_folder = str8_to_cstr(scratch.arena, binary_path);

      if (chdir(binary_folder) == -1)
      {
        WARN("Failed to set cwd to %s\n\t%s\n", binary_folder, strerror(errno));
      }
      else
      {
        String8List ld_library_path_list = ZERO_STRUCT;

        char *ld_library_path = getenv("LD_LIBRARY_PATH");
        if (ld_library_path != NULL)
        {
          str8_list_push(scratch.arena, &ld_library_path_list, str8_cstr(ld_library_path));
        }

        str8_list_push(scratch.arena, &ld_library_path_list, str8_lit("./build"));

        String8Join join = ZERO_STRUCT;
        join.mid = str8_lit(":");
        join.post = str8_lit("\0");
        String8 ld_library_path_final = str8_list_join(scratch.arena, ld_library_path_list, &join);

        if (setenv("LD_LIBRARY_PATH", (char *)ld_library_path_final.content, 1) == -1)
        {
          WARN("Failed to set $LD_LIBRARY_PATH\n\t%s\n", strerror(errno));
        }
      }
    }
  }
}

int
main(int argc, char *argv[])
{
  global_debugger_present = linux_was_launched_by_gdb();

  MemArena *perm_arena = mem_arena_allocate(GB(1), GB(1));

  // TODO(Ryan): Remove run/ folder
  linux_set_cwd_to_self(perm_arena);

  // reload_plug(&plug);
  // dlclose();
  // dlopen("app.so", RTLD_NOW);
  // platform-specific of same functionality is to vendor-lock you, much like planned obscelence
  // dlsym();

  // TODO(Ryan): Add read size to allow for windowed viewing
  // global_frame_buf = ring_buf_create(perm_arena, (memory_index)(44800 * 0.1f) * sizeof(Frame));

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

    s32 h = GetRenderHeight();
    s32 w = GetRenderWidth();

    f32 step = 1.06f;
    u32 m = 0;
    for (f32 f = 20.0f; (u32)f < N; f *= step)
    {
      m += 1;
    }
    /* now number of samples is m
    m = 0;
    for (f32 f = 20.0f; (u32)f < N; f *= step)
    {
      compute average amplitude from: f <--> f*step
      f32 t = f32_noz(ft_amp(global_out[(u32)m]), global_max_amp);
      m += 1;
    }
    */

    // instead of a ring buffer, have a rolling buffer, e.g. just pushing to the front

    s32 mid_y = h / 2.0;
    s32 bar_w = F32_CEIL_S32(CLAMP(1.0f, (f32)w / N, (f32)w));
    for (u32 i = 0; i < N; i += 1)
    {
      f32 t = f32_noz(ft_amp(global_out[i]), global_max_amp);
      
      // Vec4F32 c = vec4_f32_lerp(green, red, t);

      f32 bar_h = t * mid_y;
      s32 bar_x = i * bar_w;
      // float drawing to fill correctly
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
