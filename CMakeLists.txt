cmake_minimum_required(VERSION 3.10)

project(shulke)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_VISIBILITY_PRESET default)
set(CMAKE_VISIBILITY_INLINES_HIDDEN OFF)

# When cross-compiling for Android, the NDK toolchain sets
# CMAKE_SYSTEM_PROCESSOR from the selected ANDROID_ABI.
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    message(STATUS "Configuring for arm64-v8a")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    message(STATUS "Configuring for x86_64")
else()
    message(FATAL_ERROR
        "Unsupported processor: ${CMAKE_SYSTEM_PROCESSOR}. "
        "Only x86_64 and arm64-v8a are supported.")
endif()

include(FetchContent)

FetchContent_Declare(libhat
    GIT_REPOSITORY https://github.com/BasedInc/libhat.git
    EXCLUDE_FROM_ALL
)

FetchContent_Declare(properties-parser
    GIT_REPOSITORY https://github.com/minecraft-linux/properties-parser.git
    GIT_TAG 79f5360f889100bd4a1e069f4d9b86e94c902633
    EXCLUDE_FROM_ALL
)

FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8)

FetchContent_MakeAvailable(
    libhat
    properties-parser
    glm
)

add_library(shulke SHARED
    src/main.cpp
    src/item/itemstackbase.cpp
    src/nbt/nbt.cpp
    src/ui/minecraftuirendercontext.cpp
    src/ui/nineslicehelper.cpp
    src/shulkerenderer/shulkerrenderer.cpp
    src/util/keybinds.cpp
    src/util/modmenu.cpp
    src/util/config.cpp
)

target_include_directories(shulke PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(shulke PUBLIC
    glm::glm
    libhat::libhat
    properties-parser
)

target_link_options(shulke PRIVATE
    -static-libstdc++
)

set_target_properties(shulke PROPERTIES
    OUTPUT_NAME shulke
)

install(TARGETS shulke LIBRARY DESTINATION mods)
