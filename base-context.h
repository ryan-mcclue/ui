// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

// NOTE(Ryan): GCC
#if defined(__GNUC__)
  #define COMPILER_GCC 1

  // NOTE(Ryan): Version
  #if __GNUC__ < 10
    #warning GCC 10+ required for builtin static analysis
  #endif

  // NOTE(Ryan): Architecture
  #if defined(__x86_64__)
    #define ARCH_X86_64 1
  #elif defined(__arm__) 
    #if defined(__aarch64__)
      #define ARCH_ARM64 1
    #else
      #define ARCH_ARM32 1
      #if defined(__ARM_ARCH_7A__)
        #define ARCH_ARM7A 1
      #endif
    #endif
  #else
    #error Arch not supported
  #endif

  // NOTE(Ryan): Platform
  #if defined(__gnu_linux__)
    #define PLATFORM_LINUX 1
  #elif defined(ARCH_ARM)
    #define PLATFORM_ARM_EABI 1
    // TODO(Ryan): Are there cortex specific defines?
  #else
    #error Platform not supported
  #endif

  // NOTE(Ryan): Language
  #if defined(__cplusplus)
    #define LANG_CPP 1
    #if __cplusplus <= 199711L
      #define CPP_VERSION 98
    #elif __cplusplus <= 201103L
      #define CPP_VERSION 11
    #endif
  #else
    #define LANG_C 1
    #if __STDC_VERSION__ <= 199901L
      #define C_VERSION 99
    #elif __STDC_VERSION__ <= 201112L
      #define C_VERSION 11
    #endif
  #endif
  
  #if LANG_CPP
    // NOTE(Ryan): Avoid confusing auto-indenter
    #define EXPORT_BEGIN extern "C" {
    #define EXPORT_END }
    #define EXPORT extern "C"
    #define ZERO_STRUCT {}
    #define RESTRICT __restrict__
  #else
    #define EXPORT_BEGIN
    #define EXPORT_END
    #define EXPORT
    #define RESTRICT restrict
    #define ZERO_STRUCT {0}
  #endif

  // NOTE(Ryan): Sanitiser
  #if defined(__SANITIZE_ADDRESS__)
    #define NO_ASAN __attribute__((__no_sanitize_address__))
    #include <sanitizer/lsan_interface.h>
    // NOTE(Ryan): Prevent unreachables resulting from sanitiser being run after main
    #define LSAN_RUN() __lsan_do_leak_check(); __lsan_disable()
  #else
    #define NO_ASAN
    #define LSAN_RUN()
  #endif

  // NOTE(Ryan): Extensions
  #define CASE_FALLTHROUGH __attribute__((fallthrough))
  #define NEVER_INLINE   __attribute__((noinline))
  #define USED __attribute__((used,noinline))
  #define ALWAYS_INLINE __attribute__((optimize("inline-functions"),always_inline))
  #define PACKED __attribute__((packed))
  #define NORETURN __attribute__((noreturn))
  #define UNUSED __attribute__((unused))
  #define UNREACHABLE() __builtin_unreachable()
  #define WEAK __attribute__((weak))
  #define LIKELY(x)   __builtin_expect(!!(x), 1) 
  #define UNLIKELY(x) __builtin_expect(!!(x), 0)
  #define ISO_EXTENSION __extension__
  // NOTE(Ryan): Used to ensure .text/program-memory aligned on ARM
  #define PROGMEM const __attribute__ ((aligned(4)))
  // TODO(Ryan): tail cail compiler macro?
  // https://blog.reverberate.org/2021/04/21/musttail-efficient-interpreters.html
  
  // TODO(Ryan): Synchronisation atomics, e.g:
  // #define AtomicAdd64(ptr, v) _InterlockedExchangeAdd64((ptr), (v))
  // #define MEMORY_BARRIER()
  #define THREAD_LOCAL __thread

  // NOTE(Ryan): 
  #define PUSH_OPTIMISATION_MODE() \
    _Pragma("GCC push_options") \
    _Pragma("GCC optimize (\"O3\")")

  #define POP_OPTIMISATION_MODE() \
    _Pragma("GCC pop_options")

  #define IGNORE_WARNING_PADDED() \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wpadded\"")

  #define IGNORE_WARNING_PEDANTIC() \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wpedantic\"")

  #define IGNORE_WARNING_POP() \
    _Pragma("GCC diagnostic pop")
#else
  #error Compiler not supported
#endif
