/*
 * Copyright (C) 2020  emrsmsrli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <algorithm>
#include <fstream>
#include <iterator>
#include <ranges>

#include "engine/core/filesystem/filesystem.h"
#include "engine/core/assert.h"

VKE_DEFINE_LOG_CATEGORY_STATIC(fs, warning);

namespace volkano::fs {

std::vector<u8> read_file(const path& path)
{
    const fs::path actual_path = path.is_absolute() ? path : absolute(path);
    std::ifstream stream{actual_path, std::ios::binary};
    VKE_ASSERT_MSG(stream.is_open(), "input file stream could not be opened: {}", actual_path.string());

    std::vector<u8> bytes;
    {
        const auto size = file_size(actual_path);
        bytes.resize(size);
    }

    std::copy(
      std::istreambuf_iterator<char>{stream},
      std::istreambuf_iterator<char>{},
      bytes.begin());

    VKE_LOG(fs, verbose, "read {} bytes from {}", bytes.size(), actual_path.string());
    return bytes;
}

void write_file(const path& path, const std::span<const u8> data)
{
    const fs::path actual_path = path.is_absolute() ? path : std::filesystem::current_path() / path;
    std::ofstream stream{actual_path, std::ios::binary};
    VKE_ASSERT_MSG(stream.is_open(), "output file stream could not be opened: {}", actual_path.string());

    std::ranges::copy(data, std::ostreambuf_iterator<char>(stream));
    VKE_LOG(fs, verbose, "wrote {} bytes to {}", data.size(), actual_path.string());
}

void write_file(const path& path, const std::vector<u8>& data)
{
    write_file(path, std::span<const u8>{data.data(), data.size()});
}

} // namespace volkano::fs
