// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

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

/* NOTE(Ryan): Example error message:
 * attempt_msg: "Couldn’t parse config file: /etc/sample-config.properties"
 * reason_msg: "Failed to create an initial snapshot of the data; database user 'snapper' is lacking the required permissions 'SELECT', 'REPLICATION'"
 * resolution_msg: "Please see https://example.com/knowledge-base/snapshot-permissions/ for the complete set of necessary permissions"
 */
#define FATAL_ERROR(attempt_msg, reason_msg) \
  __fatal_error(SOURCE_LOC, attempt_msg, reason_msg)

INTERNAL void
__fatal_error(SourceLoc source_loc, const char *attempt_msg, const char *reason_msg)
{ 
#if defined(RELEASE_BUILD)
  /* TODO(Ryan): Add stack trace to message
#include <execinfo.h>
  void *callstack_addr[128] = ZERO_STRUCT;
  int num_backtrace_frames = backtrace(callstack_addr, 128);

  // TODO(Ryan): addr2line could convert addresses to names
  char **backtrace_strs = backtrace_symbols(callstack_addr, num_backtrace_frames);

  u32 max_backtrace_str_len = 255;
  int message_size = sizeof(backtrace_strs) * max_backtrace_str_len;

  for (int i = 0; i < num_backtrace_frames; ++i) {
      printf("%s\n", strs[i]);
  }
  free(strs); 
  */
#endif

  fprintf(stderr, "(%s:%s():%ld)\n %s\n\t%s\n", 
         source_loc.file_name, source_loc.func_name, source_loc.line_num, 
         attempt_msg, reason_msg);

  BP();

  kill(getpid(), SIGKILL); // pthread_exit()?
}

/* NOTE(Ryan): Example:
 * what_msg: Initialised logging  
 * why_msg: To provide trace information to understand program flow in the event of a bug
 */
#define DBG(fmt, ...) \
  fprintf(stderr, fmt, ##__VA_ARGS__);

#define TRACE(what_msg, why_msg) \
  do \
  { \
    printf("\x1B[91m"); fflush(stdout); \
    printf("%s():\n\tWHAT: %s\n\tWHY: %s", __func__, what_msg, why_msg); \
    printf("\033[0m"); fflush(stdout); \
  } while (0)

#if defined(DEBUG_BUILD)
#define WARN(fmt, ...) \
  do \
  { \
    BP(); \
    fprintf(stderr, "%s:%d:0: WARN: ", __FILE__, __LINE__); \
    fprintf(stderr, fmt, __VA_ARGS__); \
  } while (0)
#else
#define WARN(what_msg, why_msg) \
  printf("%s():\n%s\n%s", __func__, what_msg, why_msg);
#endif

#if defined(DEBUG_BUILD)
  #define ASSERT(c) do { if (!(c)) { FATAL_ERROR(STRINGIFY(c), "Assertion error"); } } while (0)
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
#endif
