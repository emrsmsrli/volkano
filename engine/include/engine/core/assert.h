/*
 * Copyright (C) 2022 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <cstdlib>

#include "debugbreak.h"

#include "engine/core/int_types.h"
#include "engine/core/logging/logging.h"

namespace volkano {

[[noreturn]] inline void unreachable()
{
    // from cppreference
    // Uses compiler specific extensions if possible.
    // Even if no extension is used, undefined behavior is still raised by
    // an empty function body and the noreturn attribute.
#if defined(__GNUC__) // GCC, Clang, ICC
    __builtin_unreachable();
#elif defined(_MSC_VER) // MSVC
    __assume(false);
#endif
}

#ifndef VKE_ASSERT_MSG
  #if DEBUG || defined(VKE_ENABLE_ASSERTIONS)
      #define VKE_ASSERT_MSG(predicate, ...)          \
        do {                                          \
            if(!(predicate)) {                        \
                VKE_LOG(general, critical,            \
                  "!(" #predicate "): " __VA_ARGS__); \
                std::fflush(stdout);                  \
                debug_break();                        \
                std::terminate();                     \
            }                                         \
        } while(0)
  #else
    #define VKE_ASSERT_MSG(...) void(0)
  #endif // DEBUG || defined(VKE_ENABLE_ASSERTIONS)
#endif // ifndef VKE_ASSERT_MSG

#define VKE_ASSERT(predicate) VKE_ASSERT_MSG(predicate, "assertion failed")

} // namespace volkano
