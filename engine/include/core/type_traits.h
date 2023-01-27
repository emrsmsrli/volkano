/*
 * Copyright (C) 2022 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <type_traits>

namespace volkano {

template<typename T, typename... Candidates>
struct is_one_of
{
    static constexpr inline bool value = (std::is_same_v<T, Candidates> || ...);
};

template<typename T, typename... Candidates>
constexpr inline bool is_one_of_v = is_one_of<T, Candidates...>::value;

template<typename... Type>
struct type_list
{
    using type = type_list;
    static constexpr auto size = sizeof...(Type);
};

template<std::size_t, typename>
struct type_list_element;

template<std::size_t Index, typename Type, typename... Other>
struct type_list_element<Index, type_list<Type, Other...>> : type_list_element<Index - 1u, type_list<Other...>> {};

template<typename Type, typename... Other>
struct type_list_element<0u, type_list<Type, Other...>>
{
    using type = Type;
};

template<std::size_t Index, typename List>
using type_list_element_t = typename type_list_element<Index, List>::type;

template<typename... Type, typename... Other>
constexpr type_list<Type..., Other...> operator+(type_list<Type...>, type_list<Other...>)
{
    return {};
}

template<typename To, typename From>
struct constness_as
{
    using type = std::remove_const_t<To>;
};

template<typename To, typename From>
struct constness_as<To, const From>
{
    using type = std::add_const_t<To>;
};

template<typename To, typename From>
using constness_as_t = typename constness_as<To, From>::type;

} // namespace volkano
