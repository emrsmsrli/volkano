/*
 * Copyright (C) 2022 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#if defined(_WIN64) || defined(_WIN32)
  #define PLATFORM_WINDOWS 1
  #define PLATFORM_UNIX 0
#elif defined(__linux) || defined(__unix)
  #define PLATFORM_WINDOWS 0
  #define PLATFORM_UNIX 1
#else
  #pragma error "unknown platform"
#endif // PLATFORMS
