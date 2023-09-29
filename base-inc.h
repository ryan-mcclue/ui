// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

#include "base-context.h"
#include "base-types.h"
#include "base-math.h"
#include "base-memory.h"
#include "base-string.h"

#if defined(PLATFORM_ARM_EABI)
  // TODO(Ryan): Change to cortex specific
  //  --cpu=Cortex-M0plus defines __TARGET_CPU_CORTEX_M0PLUS 
  //  TO SEE: armcc --cpu=xx --list_macros /dev/null/
  #include "base-dev-arm-none-eabi.h"
#else
  #include "base-dev-linux.h"
#endif

#include "base-file.h"
#include "base-map.h"

/*

// TODO(Ryan):
// #include "base-thread.h"
*/
