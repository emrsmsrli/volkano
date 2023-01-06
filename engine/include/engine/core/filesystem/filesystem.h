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

#include "engine/core/platform.h"
#include "engine/core/int_types.h"

namespace volkano::fs {

using std::filesystem::path;

std::vector<u8> read_file(const path& path);
void write_file(const path& path, const std::vector<u8>& data);
void write_file(const path& path, std::span<u8> data);

} // namespace volkano::fs
