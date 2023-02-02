/*
 * Copyright (C) 2023 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#if _MSVC_LANG
  #pragma warning(push)
  #pragma warning(disable:4100) // unreferenced formal parameter
  #pragma warning(disable:4127) // conditional expression is constant
  #pragma warning(disable:4189) // local variable is initialized but not referenced
  #pragma warning(disable:4324) // structure was padded due to alignment specifier
#elif __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-compare" // comparison of unsigned expression < 0 is always false
  #pragma clang diagnostic ignored "-Wunused-private-field"
  #pragma clang diagnostic ignored "-Wunused-parameter"
  #pragma clang diagnostic ignored "-Wmissing-field-initializers"
  #pragma clang diagnostic ignored "-Wnullability-completeness"
  // todo disable gcc warnings
#endif

#define VMA_IMPLEMENTATION
#include <vulkan_mem_alloc/vk_mem_alloc.h>

#if _MSVC_LANG
  #pragma warning(pop)
#elif __clang__
  #pragma clang diagnostic pop
  // todo reenable gcc warnings
#endif // PLATFORM
