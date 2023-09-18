// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

#include <stdarg.h>

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

// NOTE(Ryan): Allow for simple runtime debugger attachment
GLOBAL b32 global_debugger_present;

#if defined(DEBUG_BUILD)
  #define BP() \
  do \
  { \
    if (global_debugger_present) \
    { \
      raise(SIGTRAP); \
    } \
  } while (0)
#else
  #define BP()
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define FATAL_ERROR(fmt, ...) \
  __fatal_error(SOURCE_LOC, fmt, __VA_ARGS__)

#include <execinfo.h>
#include <linkmap.h>

#define NUM_ADDRESSES 64
INTERNAL void
linux_print_stacktrace(void)
{
// https://stackoverflow.com/a/63855266/9960107
  void *callstack_addr[NUM_ADDRESSES] = ZERO_STRUCT;
  u32 num_backtrace_frames = backtrace(callstack_addr, NUM_ADDRESSES);

  for (u32 i = 0; i < num_backtrace_frames; i += 1) 
  {
    char location[256] = ZERO_STRUCT;
    Dl_info info = ZERO_STRUCT; 
    if (dladdr(callstack_addr[i], &info))
    {
      link_map *map = NULL;
      Dl_info extra_info = ZERO_STRUCT;
      dladdr1(callstack_addr[i], &extra_info, (void **)&link_map, RTLD_DL_LINKMAP);
      memory_index vma = (memory_index)callstack_addr[i] - map->l_addr;
      // x86 PC one after current instruction
      vma -= 1;
      
      char cmd[256] = ZERO_STRUCT;
      snprintf(cmd, sizeof(cmd), "addr2line -e %s -Ci %zx", info.dli_frame, vma);
      system(cmd);
    }
  }

  /*
   * 1. Assume PIC so address subtraction required. Could check by inspecting elf file
   * 2. Load vaddr base with /proc/<PID>/maps 
   * 3. dl_iterate_phdr() to get .dlpi_addr field for base for shared objects
   * 4. Pass subtracted addresses to addr2line
   */
}

INTERNAL void
__fatal_error(SourceLoc source_loc, const char *fmt, ...)
{ 
  linux_print_stacktrace();

  va_list args;
  va_start(args, fmt);

  printf(ASC_RED); fflush(stdout);
  fprintf(stderr, "%s:%ld:0: FATAL_ERROR: ", source_loc.file_name, source_loc.line_num);
  vfprintf(stderr, fmt, args);
  printf(ASC_CLEAR); fflush(stdout);

  va_end(args);


  BP();

  kill(getpid(), SIGKILL); // pthread_exit()?
}

#if defined(DEBUG_BUILD)
#define DBG(fmt, ...) \
  do \
  { \
    printf(ASC_GREEN); fflush(stdout); \
    fprintf(stderr, "%s:%d:0: DBG: ", __FILE__, __LINE__); \
    fprintf(stderr, fmt, __VA_ARGS__); \
    printf(ASC_CLEAR); fflush(stdout); \
  } while (0)
#else
#define DBG(fmt, ...)
#endif

#define WARN(fmt, ...) \
  do \
  { \
    BP(); \
    printf(ASC_YELLOW); fflush(stdout); \
    fprintf(stderr, "%s:%d:0: WARN: ", __FILE__, __LINE__); \
    fprintf(stderr, fmt, __VA_ARGS__); \
    printf(ASC_CLEAR); fflush(stdout); \
  } while (0)

#if defined(DEBUG_BUILD)
  #define ASSERT(c) do { if (!(c)) { FATAL_ERROR("Assertion Error\n\t%s\n", STRINGIFY(c)); } } while (0)
  #define UNREACHABLE_CODE_PATH ASSERT(!"UNREACHABLE_CODE_PATH")
  #define UNREACHABLE_DEFAULT_CASE default: { UNREACHABLE_CODE_PATH }
#else
  #define ASSERT(c)
  #define UNREACHABLE_CODE_PATH UNREACHABLE() 
  #define UNREACHABLE_DEFAULT_CASE default: { UNREACHABLE() }
#endif

#define STATIC_ASSERT(cond, line) typedef u8 PASTE(line, __LINE__) [(cond)?1:-1]
#define NOT_IMPLEMENTED ASSERT(!"NOT_IMPLEMENTED")

#include <time.h>
#include <sys/types.h>
#include <unistd.h>

// perhaps change to get_walltime_ms()
INTERNAL u64
linux_get_ms(void)
{
  u64 result = 0;

  struct timespec time_spec = {0};
  // not actually time since epoch, 1 jan 1970
  // rather time since some unspecified period in past
  clock_gettime(CLOCK_MONOTONIC_RAW, &time_spec);

  result = ((u32)time_spec.tv_sec * 1000) + (u32)((f32)time_spec.tv_nsec / 1000000.0f);

  return result;
}

INTERNAL u32
linux_get_seed_u32(void)
{
  u32 result = 0;

  if (getentropy(&result, sizeof(result)) == -1)
  {
    WARN("getentropy failed\n\t%s", strerror(errno));
  }

  return result;
}

INTERNAL b32
linux_was_launched_by_gdb(void)
{
  b32 result = false;

  pid_t ppid = getppid(); 
  char path[64] = {0};
  char buf[64] = {0};
  snprintf(path, sizeof(path), "/proc/%d/exe", ppid);
  if (readlink(path, buf, sizeof(buf)) == -1)
  {
    WARN("Unable to readlink\n\t%s", strerror(errno));
  }
  else if (strncmp(buf, "/usr/bin/gdb", sizeof("/usr/bin/gdb")) == 0)
  {
    result = true;
  }

  return result;
}



#if 0
INTERNAL u64 
linux_get_file_mod_time(String8 file_name)
{
  u64 result = 0;

  struct stat file_stat = ZERO_STRUCT;
  if (stat((char *)file_name.str, &file_stat) == 0)
  {
    result = (u64)file_stat.st_mtime;
  }

  return result;
}

INTERNAL b32
char_is_shell_safe(char ch)
{
  String8 safe_chars = s8_lit("@%+=:,./-_");
  for (u32 i = 0; i < safe_chars.size; i += 1)
  {
    if (ch == safe_chars[i]) return true;
  }

  // NOTE(Ryan): Is a number
  if (ch >= 48 && ch <= 57) return true;

  // NOTE(Ryan): Is a character
  ch &= 0x20;
  if (ch >= 65 && ch <= 90) return true;

  return false;
}

INTERNAL b32
str8_is_shell_safe(String8 str)
{
  for (u32 i = 0; i < str.size; i += 1)
  {
    if (!char_is_shell_safe(str.content[i])) return false;
  }

  return true;
}

INTERNAL String8
str8_shell_escape(MemArena *arena, String8 str)
{
  if (str.size == 0) return str8_lit("''");

  if (str8_is_shell_safe(str)) return str;

  String8List list = ZERO_STRUCT;
  str8_list_push(arena, &list, str8_lit("'"));

  u32 cursor = 0;
  while (cursor < str.size)
  {
    memory_index quote = str8_find_substring(str, str8_lit("'"), cursor, 0); 
    str8_list_push(arena, &list, str8_up_to(str.content + cursor, str.content + quote));
    str8_list_push(arena, &list, str8_lit("'\"'\"'"));
    cursor += quote;
  }

  str8_list_push(arena, &list, str8_lit("'"));
  
  String8Join join = ZERO_STRUCT;
  String8 escaped_str = str8_list_join(arena, &list, &join);

  return escaped_str;
}

INTERNAL void
echo_cmd(char **argv)
{
  printf("[CMD]");
  for (; *argv != NULL; argv++)
  {
    printf(" ");
    printf("%s", str8_shell_escape(*argv));
  }
  printf("\n");
}

INTERNAL void
cmd_background(const char *cmd)
{
  NOT_IMPLEMENTED();

  char *args[] = {
    "dot",
    "-Tsvg",
    output_filepath,
    NULL
  };

  // want this to echo the cmd invocation such that can be copied and run seperately and work
  // we don't have to shellescape internally, only for output
  LOG(args);
  // escape single quotes: ' -> '"'"'

  execvp() v for vector of args, p for will look in $PATH if cannot find
  execvp(args[0], args);

  int status;
  pid_t = wait(&status); returns pid in case of forking multiple children 

  pid_t pid = fork();
  if (pid == -1) 
  {
    WARN("Failed to fork to run background command");
    return;
  }
  else if (pid == 0)
  {
    // child
    if (prctl(PR_SET_PDEATHSIG, SIGTERM) != -1)
    {
      execl("/bin/bash", "bash", "-c", cmd, NULL);

      EBP("Failed to execute background command in fork");
    }
    else
    {
      EBP("Failed to set death of background command when parent process terminates");
    }

    exit(127);
  }
  else
  {

  }
}

INTERNAL String8 
read_entire_command(MemArena *arena, String8 command)
{
  String8 result = str8_allocate(arena, 4096);
  int stdout_pair[2] = ZERO_STRUCT;

    if (pipe(stdout_pair) != -1)
    {
      // NOTE(Ryan): With forks, can also used shared memory...
      pid_t pid = vfork();
      if (pid != -1)
      {
        if (pid == 0)
        {
          dup2(stdout_pair[1], STDOUT_FILENO);
          close(stdout_pair[1]);
          close(stdout_pair[0]);

          execl("/bin/bash", "bash", "-c", command_str, NULL);

          EBP("Execl failed");
          exit(127);
        }
        else
        {
          wait(NULL);

          u32 bytes_read = read(stdout_pair[0], result, MAX_COMMAND_RESULT_COUNT);
          if (bytes_read != -1)
          {
            result[bytes_read] = '\0';
          }
          else
          {
            EBP("Reading from pipe failed");
          }

          close(stdout_pair[0]);
          close(stdout_pair[1]);
        }
      }
      else
      {
        close(stdout_pair[0]);
        close(stdout_pair[1]);
        EBP("Forking failed");
      }
    }
    else
    {
      EBP("Creating pipes failed");
    }
  }
  else
  {
    EBP("Calloc failed");
  }


  return result;
}

#endif
