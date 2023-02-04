/*
 * Copyright (C) 2020  emrsmsrli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <span>
#include <vector>

#include <range/v3/range/concepts.hpp>

#include "renderer/vertex.h"

namespace volkano {

template<typename T>
struct mesh_buffer {
    std::vector<T> buf;

    mesh_buffer() noexcept = default;
    explicit mesh_buffer(const ranges::range auto& r)
      : buf{ranges::begin(r), ranges::end(r)} {}

    [[nodiscard]] const T* data() const noexcept { return buf.data(); }
    [[nodiscard]] usize size() const noexcept { return buf.size(); }
    [[nodiscard]] usize size_in_bytes() const noexcept { return buf.size() * sizeof(T); }
};

class mesh {
    mesh_buffer<vertex> vertices_;
    mesh_buffer<u16> indices_;

public:
    mesh() noexcept = default;
    mesh(const ranges::range auto& vertices, const ranges::range auto& indices)
      : vertices_{vertices},
        indices_{indices} {}

    [[nodiscard]] const mesh_buffer<vertex>& get_vertex_buffer() const noexcept { return vertices_; }
    [[nodiscard]] const mesh_buffer<u16>& get_index_buffer() const noexcept { return indices_; }
};

} // namespace volkano
