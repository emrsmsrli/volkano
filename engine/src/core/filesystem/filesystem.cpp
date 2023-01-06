/*
 * Copyright (C) 2020  emrsmsrli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <fstream>

#include "engine/core/filesystem/filesystem.h"
#include "engine/core/assert.h"

VKE_DEFINE_LOG_CATEGORY_STATIC(fs, warning);

namespace volkano::fs {

std::vector<u8> read_file(const path& path)
{
    std::ifstream stream{path, std::ios::binary | std::ios::ate};
    VKE_ASSERT_MSG(stream.is_open(), "input file stream could not be opened: {}", path.string());

    const std::ifstream::pos_type file_size = stream.tellg();

    std::vector<u8> bytes;
    bytes.reserve(file_size);

    stream.seekg(0, std::ios::beg);
    stream.read(reinterpret_cast<char*>(bytes.data()), file_size); // NOLINT

    VKE_LOG(fs, verbose, "read {} bytes from {}", bytes.size(), path.string());
    return bytes;
}

void write_file(const path& path, const std::span<const u8> data)
{
    std::ofstream stream{path, std::ios::binary};
    VKE_ASSERT_MSG(stream.is_open(), "output file stream could not be opened: {}", path.string());

    stream.write(reinterpret_cast<const char*>(data.data()), data.size()); // NOLINT
    VKE_LOG(fs, verbose, "wrote {} bytes to {}", data.size(), path.string());
}

void write_file(const path& path, const std::vector<u8>& data)
{
    write_file(path, std::span<const u8>{data.data(), data.size()});
}

} // namespace volkano::fs
