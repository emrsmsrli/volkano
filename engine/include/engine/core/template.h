/*
 * Copyright (C) 2021 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#ifndef VOLKANO_TEMPLATES_H
#define VOLKANO_TEMPLATES_H

#include <type_traits>

namespace volkano {

template<typename T, typename... Candidates>
struct is_one_of {
    static constexpr inline bool value = (std::is_same_v<T, Candidates> || ...);
};

template<typename T, typename... Candidates>
constexpr inline bool is_one_of_v = typename is_one_of<T, Candidates...>::value;

} // namespace volkano

#endif // VOLKANO_TEMPLATES_H
