#pragma once

#include <cstddef>
#include <cstdint>
#include <xmmintrin.h>

using BaseActorRenderContext_ctor_t = void (*)(void* barc, void* screenContext, void* clientInstance, void* minecraftGame);
extern BaseActorRenderContext_ctor_t BaseActorRenderContext_ctor;

using ItemRenderer_renderGuiItemNew_t = std::uint64_t (*) (
    void* rendererCtx,
    void* barc,
    void* itemStack,
    unsigned int aux,
    unsigned char layer,
    std::uint64_t flags,
    __m128 posX,
    __m128 posY,
    float width,
    float height,
    float scale);
extern ItemRenderer_renderGuiItemNew_t ItemRenderer_renderGuiItemNew;

// 1.26 offsets from RE.
inline constexpr std::size_t kBarcStorageSize = 0x400;
inline constexpr std::size_t kBarcItemRendererOffset = 0x58;      // BaseActorRenderContext + 88
inline constexpr std::size_t kClientMinecraftGameOffset = 0xA8;   // ClientInstance + 168 fallback
inline constexpr std::size_t kClientGetMinecraftGameVfIndex = 83; // vtable slot for getMinecraftGame