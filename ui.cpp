// SPDX-License-Identifier: zlib-acknowledgement

// tree iterations: https://www.youtube.com/watch?v=QkuNmL7tz08 

// gui: https://www.youtube.com/watch?v=-e_yyggsh_o 

// tcc: https://www.youtube.com/watch?v=4vSyqK3SK-0 

// graphs: https://www.youtube.com/watch?v=QVTcXGC2NMk&t=794s  

// ai series
// text editor: shaders etc.

// #if defined(FEATURE_FLAG) || defined(COMPATIBLE_FEATURE_FLAG)

// !cmd (execute last command starting with 'cmd')

// TODO(Ryan): specific memory arenas for caching?
// as recieves arena, will be allocating permanent results to caller
//func(MemArena *arena) -> Node *
// so, if doing temp work, won't pass in, rather use global temp?
// linear allocator
// dynamic with linked list of arenas?
// IMPORTANT(Ryan): If need variable lifetimes, can build atop of arena with a free-list/pool allocator

#include "base-inc.h"

#include "raylib.h"

// TODO(Ryan): For networking applications: https://github.com/Jai-Community/awesome-jai 
// https://github.com/ValveSoftware/GameNetworkingSockets/blob/master/examples/trivial_signaling_client.cpp

// vectorised fft: https://github.com/ValveSoftware/ffts

// TODO(Ryan): ranger previewing (probably have to reinstall python to get ueberzug!)

#define EACH_ELEMENT(arr, it) memory_index it = 0; it < ARRAY_COUNT(arr); it += 1

typedef struct Frame Frame;
struct Frame
{
  f32 left, right;
};

typedef struct Sample Sample;
struct Sample
{
  Sample *next, *prev;
  f32 value;
};

GLOBAL Sample *global_sample_first, *global_sample_last;
GLOBAL u32 global_num_samples;
GLOBAL f32 *global_fft_in;
GLOBAL f32z *global_fft_out;

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
// Cooley-Tukey, ported from https://rosettacode.org/wiki/Fast_Fourier_transform#Python
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
  // written in different thread, however mutex not really necessary
  // audio samples are float normalised!
  
  // IMPORTANT(Ryan): raylib audio creates stereo even if mono or more
  // cdecl.org to help out for statically, weakly typed C!
  // u32 channels = 2;
  // f32 (*fs)[channels] = (f32(*)[channels])bufferData;
  // accessed with fs[frame_i][0/1]
  
  // TODO(Ryan): Verify rolling buffer working with gdb pretty printers
  // Also investigate gdb multithreaded
  // BP();

  Frame *frame_data = (Frame *)bufferData;
  for (u32 i = 0; i < frames; i += 1)
  {
    // NOTE(Ryan): Obtain new memory from last and update last
    global_sample_last->prev->next = NULL;
    Sample *new_sample = global_sample_last;
    global_sample_last = global_sample_last->prev;

    new_sample->value = frame_data[i].left;

    DLL_PUSH_FRONT(global_sample_first, global_sample_last, new_sample);
  }

  u32 i = 0;
  for (Sample *sample = global_sample_first;
       sample != NULL;
       sample = sample->next)
  {
    f32 t = (f32)i / (global_num_samples - 1);
    f32 hann = 0.5f - 0.5f * F32_COS(F32_TAU * t);
    // hann normalised, will produce a bell curve, e.g. hann * global_num_samples;
    // Hann window is a bell curve
    // tapers edges of samples to reduce spectral leakage, i.e. jumps in non-periodic signal
    // will remove phantom frequencies
    global_fft_in[i++] = sample->value * hann;
  }

  // TODO(Ryan): Move fft into main thread as this probably called more than 60fps
  fft(global_fft_in, 1, global_fft_out, global_num_samples);
}

// IMPORTANT(Ryan): Fragment shader from library would call say, draw_texture() library function.
// This will pass specific coordinates to our fragment shader
// A pure circle in shader is possible as otherwise tesselation of triangles.

// IMPORTANT(Ryan): Create a gradient by putting 't' value on alpha

// order of importance by number of trailing Os
// TODO(
// TODOO(

// f32 dt = GetFrameTime();
// f32 rate = 2.0f;

// smoothed_samples_out[N]
// smoothed_samples_out += (log_samples_out - smoothed_samples_out) * 2 * dt;
// smeared_samples_out[N];
// smeared_samples_out += (smoothed_samples_out - smeared_samples_out) * dt; (where smear ends)
// so, DrawTexturePro(smear, smooth);

// offset = font_height/40; 

// hsv (base colour, e.g. blue) + (how much of that colour) + (how bright/dark)
// f32 hue = (f32)i / num_samples;
// Color color = ColorFromHSV(hue * 360, 1.0f, 1.0f);  get colour wheel
// f32 thickness = cell_width / 3.0f; f32 radius = cell_width;
// DrawLineEx();

int
main(int argc, char *argv[])
{
  // if [] overloaded, could dereference empty array
  // FIXING ERRORS:
  //   - clone and work on that
  //   - once bugs found, create branch, e.g. fix-something. Then make individual commits for each problem fixed
  //   - fork on github and change origin to this. push to this
  //   - then pull request on github UI
  global_debugger_present = linux_was_launched_by_gdb();
  MemArena *perm_arena = mem_arena_allocate_default();

  ThreadContext tctx = thread_context_allocate();
  tctx.is_main_thread = 1;
  thread_context_set(&tctx);

  linux_set_cwd_to_self();

  String8List args_list = ZERO_STRUCT;
  for (s32 i = 1; i < argc; i += 1)
  {
    str8_list_push(perm_arena, &args_list, str8_cstr(argv[i]));
  }

  String8Node *arg = args_list.first; 
  u32 cursor = 0;
  u32 arg_count = args_list.node_count;
  while (cursor < arg_count)
  {
    String8 arg_string = arg->string; 
    if (str8_match(arg_string, str8_lit("-language"), 0))
    {
      if (cursor == arg_count - 1)
      {
        printf("Error: missing argument after '-language'\n");
        break;
      }
      // language_string = arg->next->string;
      arg = arg->next->next;
      cursor += 2;
    }
    else
    {
      printf("Error: unknown argument '%.*s'\n", str8_varg(arg_string));
      arg = arg->next;
      cursor += 1;
    }
  }

  global_num_samples = 2048;
  for (u32 i = 0; i < global_num_samples; i += 1)
  {
    Sample *sample = MEM_ARENA_PUSH_STRUCT_ZERO(perm_arena, Sample);
    DLL_PUSH_FRONT(global_sample_first, global_sample_last, sample);
  }

  global_fft_in = MEM_ARENA_PUSH_ARRAY_ZERO(perm_arena, f32, global_num_samples);
  global_fft_out = MEM_ARENA_PUSH_ARRAY_ZERO(perm_arena, f32z, global_num_samples);

  // reload_plug(&plug);
  // dlclose();
  // dlopen("app.so", RTLD_NOW);
  // platform-specific of same functionality is to vendor-lock you, much like planned obscelence
  // dlsym();

  s32 window_factor = 80;
  InitWindow(window_factor * 16, window_factor * 9, "visualiser");
  SetTargetFPS(60);

  // NOTE(Ryan): anti-aliasing, i.e. averaging around pixel
  SetConfigFlags(FLAG_MSAA_4X_HINT);

  InitAudioDevice();

  // supply size, otherwise probably use low-resolution default in pre-rendered atlas
  Font alegraya = LoadFontEx("Alegreya-Regular.ttf", 48, NULL, 0);

  Music music = ZERO_STRUCT;
  b32 music_loaded = false;

  s32 h = GetRenderHeight();
  s32 w = GetRenderWidth();

  while (!WindowShouldClose())
  {
    if (!music_loaded)
    {
      const char *text = "Drag & Drop Music";
      Vector2 text_dim = MeasureTextEx(alegraya, text, 48.0f, 0.0f);
      Vector2 text_pos = {
        w/2.0f - text_dim.x/2.0f,
        h/2.0f - text_dim.y/2.0f,
      };
      DrawTextEx(alegraya, text, text_pos, 48.0f, 0.0f, WHITE);
    }

    if (IsFileDropped())
    {
      FilePathList dropped_files = LoadDroppedFiles();
      char *first_file = dropped_files.paths[0];
      DBG("Dropped: %s\n", first_file);

      if (music_loaded)
      {
        StopMusicStream(music);
        UnloadMusicStream(music);
      }
      else
      {
        music = LoadMusicStream(first_file);
        ASSERT(music.stream.sampleSize == 16);
        ASSERT(music.stream.channels == 2 && "mono e no");
        SetMusicVolume(music, 0.25f);

        AttachAudioStreamProcessor(music.stream, callback);
        PlayMusicStream(music);

        music_loaded = true;
      }

      UnloadDroppedFiles(dropped_files);
    }

    UpdateMusicStream(music);

    if (IsKeyPressed(KEY_SPACE))
    {
      if (IsMusicStreamPlaying(music)) PauseMusicStream(music);
      else ResumeMusicStream(music);
    }

    BeginDrawing();
    Color smoke = {0x18, 0x18, 0x18, 0xff};
    ClearBackground(smoke);

    f32 step = 1.06f;
    f32 sample_rate = 44100.0f;
    f32 nyquist = sample_rate / 2.0f; 
    f32 max_fft_mag = 0.0f;

    // IMPORTANT(Ryan): If displayed linearly, will mirror
    u32 num_bars = 0;
    for (f32 freq = 20.0f; freq < nyquist; freq *= step)
    {
      u32 fft_index = (u32)((freq * global_num_samples) / nyquist);
      f32 fft_mag = f32z_mag(global_fft_out[fft_index]);
      fft_mag = F32_LN(fft_mag);
      // logarithmic dB is defined as 10 * log10f()

      if (max_fft_mag < fft_mag) max_fft_mag = fft_mag;

      // NOTE(Ryan): Could also compute with logarithms as geometric series
      num_bars += 1;
    }

    s32 mid_y = h / 2.0;
    s32 bar_w = F32_CEIL_S32(CLAMP(1.0f, (f32)w / num_bars, (f32)w));
    u32 i = 0;
    // NOTE(Ryan): freq = ceilf(freq * step) to view lower frequencies better
    for (f32 freq = 20.0f; freq < nyquist; freq *= step)
    {
      u32 fft_index = (u32)((freq * global_num_samples) / nyquist);

      // TODO(Ryan): Compute max amplitude from: f <--> f*step
      f32 fft_mag = f32z_mag(global_fft_out[fft_index]);
      fft_mag = F32_LN(fft_mag);

      f32 t = f32_noz(fft_mag, max_fft_mag);

      // Vec4F32 c = vec4_f32_lerp(green, red, t);
      f32 bar_h = t * mid_y;
      s32 bar_x = i * bar_w;
      // float drawing to fill correctly
      DrawRectangle(bar_x, mid_y - bar_h, bar_w, bar_h, RED);

      i += 1;
    }

    EndDrawing();
  }

  return 0;
}

