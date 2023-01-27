/*
 * Copyright (C) 2022 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <filesystem>
#include <span>
#include <vector>

#include "core/platform.h"
#include "core/int_types.h"

namespace volkano::fs {

using namespace std::filesystem;

std::vector<u8> read_bytes_from_file(const path& path);
void write_bytes_to_file(const path& path, const std::vector<u8>& data);
void write_bytes_to_file(const path& path, std::span<const u8> data);

} // namespace volkano::fs
