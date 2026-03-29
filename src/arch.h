#pragma once

#if defined(__x86_64__) || defined(_M_X64)
#   define ARCH_X86_64 1
#   define ARCH_NAME "x86_64"

#elif defined(__aarch64__) || defined(_M_ARM64)
#   define ARCH_ARM64 1
#   define ARCH_NAME "arm64-v8a"

#else
#   error "Unsupported architecture. Only x86_64 and arm64-v8a are supported."
#endif

using RenderVector = float;

inline float makeRenderVector(float value) {
    return value;
}