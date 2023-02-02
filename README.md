# volkano

volkano is a Vulkan-based educational general purpose engine. 
Currently, it's heavily WIP, and it doesn't really do anything useful.

# Building

Building is as simple as just invoking CMake like below:
```shell
cmake --config Release -GNinja
  -DVKE_ENABLE_TESTS=ON 
  -DVKE_LOG_VERBOSITY=VERBOSE 
  -S /path/to/volkano 
  -B /path/to/volkano/cmake-build-release
```

### CMake Arguments
- **VKE_ENABLE_TESTS**: Enables tests if _ON_
- **VKE_LOG_VERBOSITY**: Sets the compile-time verbosity of log calls, can be one of:\
  _OFF_, _CRITICAL_, _ERROR_, _WARNING_, _INFO_, _DEBUG_, _VERBOSE_

for more arguments check `cmake/` directory.

# Dependencies

volkano depends on following libraries:

- Vulkan SDK 1.3
- libfmt
- magic_enum
- SDL2

These dependencies are included in the repository:
- Dear ImGui
- VulkanMemoryAllocator-Hpp
- debugbreak
