/*
 * Copyright (C) 2022 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "volkano.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    volkano::engine engine;
    while (engine.tick()) {}
    return 0;
}
