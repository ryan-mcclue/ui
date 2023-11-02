// SPDX-License-Identifier: zlib-acknowledgement

// TODO(Ryan): inkscape inspect logs on suspected gpu crash

// TODO(UI):
// https://mattiasgustavsson.itch.io/yarnspin/devlog/544215/coding-an-ad-hoc-ui   
//  -- templates/code/app.cpp (ui stuff)
//  1. Region drawing; padding/margins/heights off region values
//  2. Smooth scrolling; offset in axis with dampening velocity
//  2. Mouse over/released;
//
//  3. Image button creation; draw background with rect and scale relative to that size
//
//  4. Inactivity timers
//  2. Panning/Zooming
//  1. Animation
//  2. 
//  4. Text input
//  5. ...


// tree iterations: https://www.youtube.com/watch?v=QkuNmL7tz08 

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


typedef struct RFont RFont;
struct RFont
{
  Font font;
  u32 size;
};

INTERNAL RFont
load_font(String8 name, u32 size)
{
  RFont result = ZERO_STRUCT;
  result.font = LoadFontEx((const char *)name.content, size, NULL, 0);
  result.size = size;
  return result;
}

INTERNAL Vec2F32
measure_text(RFont font, String8 text)
{
  char text_buf[256] = ZERO_STRUCT;
  str8_to_cstr(text, text_buf, sizeof(text_buf));

  Vector2 text_dim = MeasureTextEx(font.font, text_buf, font.size, 0.0f);

  return vec2_f32(text_dim.x, text_dim.y);
}

// NOTE(Ryan): Solarized light colours
GLOBAL Vec4F32 WHITE_COLOUR = vec4_f32(1.0f, 1.0f, 1.0f, 1.0f);
GLOBAL Vec4F32 OFFWHITE_COLOUR = vec4_f32_hex(0xfdf6e3);
GLOBAL Vec4F32 BLACK_COLOUR = vec4_f32(0.0f, 0.0f, 0.0f, 1.0f);
GLOBAL Vec4F32 OFFBLACK_COLOUR = vec4_f32_hex(0x002b36);
GLOBAL Vec4F32 LIGHTGREY_COLOUR = vec4_f32(0.93f, 0.91f, 0.88f, 1.0f);
GLOBAL Vec4F32 DARKGREY_COLOUR = vec4_f32(0.35f, 0.38f, 0.40f, 1.0f);
GLOBAL Vec4F32 YELLOW_COLOUR = vec4_f32(0.71f, 0.54f, 0.00f, 1.0f);
GLOBAL Vec4F32 ORANGE_COLOUR = vec4_f32(0.80f, 0.29f, 0.09f, 1.0f);
GLOBAL Vec4F32 RED_COLOUR = vec4_f32(0.86f, 0.20f, 0.18f, 1.0f);
GLOBAL Vec4F32 MAGENTA_COLOUR = vec4_f32(0.83f, 0.21f, 0.05f, 1.0f);
GLOBAL Vec4F32 VIOLET_COLOUR =vec4_f32(0.42f, 0.44f, 0.77f, 1.0f);
GLOBAL Vec4F32 BLUE_COLOUR = vec4_f32(0.15f, 0.55f, 0.82f, 1.0f);
GLOBAL Vec4F32 CYAN_COLOUR = vec4_f32(0.16f, 0.63f, 0.60f, 1.0f);
GLOBAL Vec4F32 GREEN_COLOUR = vec4_f32(0.52f, 0.60f, 0.00f, 1.0f);

INTERNAL Color
vec4_f32_to_raylib_color(Vec4F32 vec)
{
  Color c = ZERO_STRUCT;

  c.r = F32_ROUND_S32(255.0f * vec.r);
  c.g = F32_ROUND_S32(255.0f * vec.g);
  c.b = F32_ROUND_S32(255.0f * vec.b);
  c.a = F32_ROUND_S32(255.0f * vec.a);

  return c;
}

INTERNAL void
draw_text(RFont font, String8 text, Vec2F32 pos, Vec4F32 colour)
{
  char text_buf[256] = ZERO_STRUCT;
  str8_to_cstr(text, text_buf, sizeof(text_buf));

  Vector2 text_pos = {pos.x, pos.y};
  Color text_colour = vec4_f32_to_raylib_color(colour);
  DrawTextEx(font.font, text_buf, text_pos, font.size, 0.0f, text_colour);
}

INTERNAL void
draw_char(RFont font, char ch, Vec2F32 pos, Vec4F32 colour)
{
  char text_buf[2] = {ch, '\0'};

  Vector2 text_pos = {pos.x, pos.y};
  Color text_colour = vec4_f32_to_raylib_color(colour);
  DrawTextEx(font.font, text_buf, text_pos, font.size, 0.0f, text_colour);
}

INTERNAL void
draw_rect(RectF32 rect, Vec4F32 colour)
{
  Rectangle rectangle = {rect.x, rect.y, rect.w, rect.h};
  Color rect_colour = vec4_f32_to_raylib_color(colour);
  DrawRectangleRec(rectangle, rect_colour);
}

INTERNAL void
draw_rect_rounded(RectF32 rect, f32 roundness, u32 segments, Vec4F32 colour)
{
  Rectangle rectangle = {rect.x, rect.y, rect.w, rect.h};
  Color rect_colour = vec4_f32_to_raylib_color(colour);
  DrawRectangleRounded(rectangle, roundness, segments, rect_colour);
}

INTERNAL void
draw_texture(Texture2D tex, Vec2F32 pos, f32 rotation, f32 scale, Vec4F32 colour) {
  Vector2 position = {pos.x, pos.y};
  Color tex_colour = vec4_f32_to_raylib_color(colour);
  DrawTextureEx(tex, position, rotation, scale, tex_colour);
}

INTERNAL void
draw_texture_atlas(Texture2D tex, RectF32 source, RectF32 dest, f32 rotation, Vec4F32 colour)
{
  Vector2 origin = {0, 0};
  Rectangle src = {source.x, source.y, source.w, source.h};
  Rectangle dst = {dest.x, dest.y, dest.w, dest.h};
  Color tex_colour = vec4_f32_to_raylib_color(colour);
  DrawTexturePro(tex, src, dst, origin, rotation, tex_colour);
}

// draw_rect_outline(), draw_circle_outline(), draw_char(c, scale)
INTERNAL b32
mouse_over_rect(RectF32 rec)
{
  Vector2 mouse = GetMousePosition();
  Vec2F32 mouse_vec = {mouse.x, mouse.y};
  return rect_f32_contains(rec, mouse_vec);
}

GLOBAL String8 global_text_buffer;

INTERNAL void
text_edit(RectF32 region, RFont font)
{
  if (mouse_over_rect(region))
  {
    SetMouseCursor(MOUSE_CURSOR_IBEAM);
    int key = GetCharPressed();
    while (key > 0)
    {
      global_text_buffer.content[global_text_buffer.size++] = (char)key;
      key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE))
    {
      if (global_text_buffer.size != 0) global_text_buffer.size -= 1;
    }

    // if (IsKeyPressed(KEY_LEFT) cursor -= 1;
    // if (IsKeyPressed(KEY_RIGHT) cursor += 1;

  }
  else
  {
    SetMouseCursor(MOUSE_CURSOR_DEFAULT);
  }

  draw_rect(region, LIGHTGREY_COLOUR); 

  Vec2F32 text_pos = {region.x, region.y};
  draw_text(font, global_text_buffer, text_pos, WHITE_COLOUR);

  Vec2F32 text_dim = measure_text(font, global_text_buffer);

  f32 cursor_size = 40.0f;
  RectF32 cursor = {region.x + text_dim.x, region.y, cursor_size, cursor_size*3.0f};
  draw_rect(cursor, WHITE_COLOUR);
}

INTERNAL void
render_fft(RectF32 region, f32 nyquist, f32 step, f32 sample_rate, f32 *bars, Music *music)
{
  u32 num_bars = 0;
  f32 max_fft_mag = 0.0f;
  // IMPORTANT(Ryan): If displayed linearly, will mirror
  for (f32 freq = 20.0f; freq < nyquist; freq *= step)
  {
    u32 fft_index = (u32)((freq * global_num_samples) / nyquist);
    f32 fft_mag = f32z_mag(global_fft_out[fft_index]);
    fft_mag = F32_LN(fft_mag);
    // logarithmic dB is defined as 10 * log10f()

    if (max_fft_mag < fft_mag) max_fft_mag = fft_mag;

    num_bars += 1;
  }

  RectF32 bar = ZERO_STRUCT;
  s32 bar_w = F32_CEIL_S32(CLAMP(1.0f, (f32)region.w / num_bars, (f32)region.w));
  bar.w = bar_w;

  u32 i = 0;
  // NOTE(Ryan): freq = ceilf(freq * step) to view lower frequencies better
  for (f32 freq = 20.0f; freq < nyquist; freq *= step)
  {
    u32 fft_index = (u32)((freq * global_num_samples) / nyquist);

    // TODO(Ryan): Compute max amplitude from: f <--> f*step
    f32 fft_mag = f32z_mag(global_fft_out[fft_index]);
    fft_mag = F32_LN(fft_mag);

    f32 t = f32_noz(fft_mag, max_fft_mag);

    // hsv (base colour, e.g. blue) + (how much of that colour) + (how bright/dark)
    f32 hue = (f32)i / num_bars;
    Vec3F32 bar_hsv = vec3_f32(hue, 1.0f, 1.0f);
    Vec4F32 bar_colour = vec4_f32_rgb_from_hsv(bar_hsv); 

    bar.x = region.x + i * bar_w;

    f32 target_h = t * region.h;
    bars[i] += (target_h - bars[i]) * 2 * GetFrameTime();

    bar.h = bars[i];
    bar.y = region.y + region.h - bar.h;

    draw_rect(bar, bar_colour);

    i += 1;
  }

  f32 played = GetMusicTimePlayed(*music); 
  f32 len = GetMusicTimeLength(*music); 
  f32 x = played / len;
  // t = (mouse_x - item_x)/item_w;
}


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

  // TODO(Ryan): Needed to continue if read_entire_command() pipe breaking?
    // NOTE: This is needed because if the pipe between Musializer and FFmpeg breaks
    // Musializer will receive SIGPIPE on trying to write into it. While such behavior
    // makes sense for command line utilities, Musializer is a relatively friendly GUI
    // application that is trying to recover from such situations.
    // struct sigaction act = {0};
    // act.sa_handler = SIG_IGN;
    // sigaction(SIGPIPE, &act, NULL);

  ThreadContext tctx = thread_context_allocate();
  tctx.is_main_thread = 1;
  thread_context_set(&tctx);

  // TODO(Ryan): Only have this for release
  // During development, have root project folder as where binary would be
  // So, have a top-level resources folder
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

  f32 step = 1.06f;
  f32 sample_rate = 44100.0f;
  f32 nyquist = sample_rate / 2.0f; 
  // power of step to reach nyquist
  u32 num_bars = F32_LOG(step, nyquist / 20.0f) + 1;
  f32 *bars = MEM_ARENA_PUSH_ARRAY_ZERO(perm_arena, f32, num_bars);

  // reload_plug(&plug);
  // dlclose();
  // dlopen("app.so", RTLD_NOW);
  // platform-specific of same functionality is to vendor-lock you, much like planned obscelence
  // dlsym();

  s32 window_factor = 80;
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(window_factor * 16, window_factor * 9, "visualiser");
  SetTargetFPS(60);

  Image logo = LoadImage("logo-256.png");
  SetWindowIcon(logo);

  // NOTE(Ryan): anti-aliasing, i.e. averaging around pixel
  //SetConfigFlags(FLAG_MSAA_4X_HINT);

  InitAudioDevice();

  // supply size, otherwise probably use low-resolution default in pre-rendered atlas
  // default font uses atlas, so if want to look good at various resolutions, use SDF font generation
  // SetTextureFilter(font.texture, bilinear); might help for rendering at different sizes, e.g. boundary.w*0.4f;
  // $(fc-list)
  RFont alegraya = load_font(str8_lit("Alegreya-Regular.ttf"), 64);

  // TODO(Ryan): ffmpeg, i.e. start external command and display progress?

  Music music = ZERO_STRUCT;
  b32 music_loaded = false;

  Image btn_img = LoadImage("button.png");
  Texture2D btn_tex = LoadTextureFromImage(btn_img);

  global_text_buffer = str8_allocate(perm_arena, KB(1));

  u32 active_item = 0;
  while (!WindowShouldClose())
  {
    f32 h = (f32)GetRenderHeight();
    f32 w = (f32)GetRenderWidth();
    f32 dt = GetFrameTime();

    if (!music_loaded)
    {
      String8 text = str8_lit("Drag & Drop Music");
      Vec2F32 text_dim = measure_text(alegraya, text);
      Vec2F32 text_pos = {
        w/2.0f - text_dim.x/2.0f,
        h/2.0f - text_dim.y/2.0f,
      };
      Vec4F32 text_color = vec4_f32(241.0f/255.0f, 95.0f/255.0f, 0.0f, 1.0f);

      Vec4F32 white = vec4_f32(1.0f, 1.0f, 1.0f, 1.0f);

      f64 now = GetTime();
      f32 freq = 5.0f;
      f32 t = cos(now * 5.0f);
      t *= t;
      t = 0.4f + 0.58f * t;
      Vec4F32 backing_color = vec4_f32_lerp(text_color, white, t);

      u32 offset = alegraya.font.baseSize / 40;
      Vec2F32 offset_text_pos = {
        text_pos.x - offset,
        text_pos.y + offset
      };
      f32 highlight_freq = 20.0f;
      draw_text(alegraya, text, offset_text_pos, backing_color);

      draw_text(alegraya, text, text_pos, text_color);
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

    f32 panel_height = h * 0.25f;
    RectF32 preview_region = {0.0f, 0.0f, w, h - panel_height};
    render_fft(preview_region, nyquist, step, sample_rate, bars, &music);

    RectF32 panel_region = {0.0f, preview_region.h, w, panel_height};
    draw_rect(panel_region, vec4_f32(0.3f, 0.5f, 0.2f, 1.0f));

    // useful for scrolling in say middle of screen to not have to rendering bounds checking
    // BeginScissorMode(panel_region.x, ...);
    // EndScissorMode();

    u32 num_items = 8;

    f32 scroll_bar_height = panel_region.h * 0.1f;
    f32 entire_scrollable_width = num_items * panel_height;

    // IMPORTANT(Ryan): local_persist useful in UI without having to have whole structure etc.
    LOCAL_PERSIST f32 panel_scroll = 0.0f;
    LOCAL_PERSIST f32 panel_velocity = 0.0f; 
    panel_velocity *= 0.9f; // for smooth scrolling, slow down over time
    panel_velocity += GetMouseWheelMove() * panel_height * 4.0f;
    panel_scroll -= panel_velocity * dt;

    f32 min_scroll = 0.0f;
    if (panel_scroll < min_scroll) panel_scroll = min_scroll;
    f32 max_scroll = num_items * panel_height - panel_region.w; 
    if (max_scroll < 0) max_scroll = 0;
    if (panel_scroll > max_scroll) panel_scroll = max_scroll;

    f32 item_padding = panel_height * 0.1f; 

    Vec4F32 item_default = vec4_f32(0.3f, 0.8f, 0.9f, 1.0f);
    Vec4F32 item_hover = vec4_f32_whiten(item_default, 0.2f);
    Vec4F32 item_active = vec4_f32_whiten(item_default, 0.6f);

    for (u32 i = 0; i < num_items; i += 1)
    {
      RectF32 item_region = {-panel_scroll + i * panel_height + panel_region.x + item_padding, 
                             panel_region.y + item_padding, 
                             panel_height - 2*item_padding, panel_height - 2*item_padding};

      Vec4F32 item_colour = item_default;
      if (i == active_item)
      {
        item_colour = item_active; 
      }
      else if (mouse_over_rect(item_region))
      {
        item_colour = item_hover; 
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
          active_item = i;
        }
        draw_rect(item_region, item_colour);
      }

      draw_rect(item_region, item_colour);
    }

    // only show if scrollable
    if (entire_scrollable_width > panel_region.w)
    {
      f32 t = panel_region.w / entire_scrollable_width;
      f32 q = panel_scroll / entire_scrollable_width; // as panel_scroll if offset into entire scrollable region

      RectF32 scroll_bar_region = {panel_region.x + panel_region.w*q, panel_region.y, panel_region.w*t, scroll_bar_height};
      draw_rect(scroll_bar_region, vec4_f32(1.0f, 0.0f, 0.0f, 1.0f));
    }

    f32 btn_size = preview_region.w * 0.2f;
    RectF32 btn_region = {
      preview_region.x + preview_region.w/2.0f - btn_size/2.0f,
      preview_region.y + preview_region.h/2.0f - btn_size/2.0f,
      btn_size,
      btn_size
    };
    // save an if with: if vec2_f32_len(mouse) > 0

    // IMPORTANT(Ryan): Writing messy code first that gets job done
    // Then, refactor, as messy code allows to see what components can be extracted
    // IMPORTANT(Ryan): Can just have state in a global struct


    // NOTE(Ryan): Conversion of nested if index assignment
    // u32 i = (is_fullscreen << 1 | are_hovering);

    f32 icon_size = 380.0f;

    draw_rect_rounded(btn_region, 0.5f, 20, DARKGREY_COLOUR);
    f32 btn_scale = btn_region.w / btn_tex.width * 0.5f;
    Vec2F32 btn_tex_pos = {btn_region.x + btn_region.w/2.0f - btn_tex.width * btn_scale/2.0f, 
                           btn_region.y + btn_region.h/2.0f - btn_tex.height * btn_scale/2.0f};
    draw_texture(btn_tex, btn_tex_pos, 0.0f, btn_scale, WHITE_COLOUR);

    // TODO(Ryan): Refactor to: 
    // b32 clicked = draw_btn(is_active);
    // f32 slider_val = draw_slider();
    // u32 selected_index = draw_checkbox/radios/tabs();
    // UI_LineEdit(TxtPt *cursor, TxtPt *mark, U64 buffer_size, U8 *buffer, U64 *string_size, String8 string)
    
    // if (draw_btn_tex(region, tex, index)) {}
    
    //draw_texture_atlas(btn_tex);

    //draw_rect_rounded(button);
    //if (mouse_in_btn) {}

    f32 slider_x = 100.0f;
    f32 slider_y = 100.0f;
    f32 slider_len = 300.0f;
    f32 slider_thickness = 10.0f;

    // IMPORTANT(Ryan): slider_y is in affect slider_cen_y, i.e. centre of slider bar
    RectF32 slider_bar = {
      slider_x,
      slider_y - slider_thickness/2.0f,
      slider_len, slider_thickness
    };
    draw_rect(slider_bar, RED_COLOUR);

    LOCAL_PERSIST b32 dragging_grip = false;
    f32 slider_size = 50.0f;
    f32 grip_off = slider_len * 1.0f;
    RectF32 slider_grip = {
      slider_x - slider_size + grip_off,
      slider_y - slider_size/2.0f, slider_size, slider_size
    };

    if (!dragging_grip)
    {
      if (mouse_over_rect(slider_grip) && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
      {
        dragging_grip = true;
      } 
    }
    else 
    {
      if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
      {
        dragging_grip = false;
      }
      else
      {
        Vector2 mouse = GetMousePosition();
        f32 slider_val = slider_bar.x + (mouse.x - slider_bar.x);
        slider_val = CLAMP(slider_bar.x, slider_val, slider_bar.x + slider_bar.w - slider_grip.w);
        slider_grip.x = slider_val; 
      }
    }

    draw_rect(slider_grip, GREEN_COLOUR);

    f32 text_size = 200.0f;
    RectF32 text_region = {text_size, text_size, w, text_size};
    text_edit(text_region, alegraya);

    EndDrawing();
  }

  return 0;
}

