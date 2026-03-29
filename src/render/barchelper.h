#pragma once

#include <cstddef>

#include "helper.h"

inline void* getMinecraftGameFromClient(void* clientInstance) {
    if (!clientInstance)
        return nullptr;

    auto** vtable = *reinterpret_cast<void***>(clientInstance);
    if (vtable && vtable[kClientGetMinecraftGameVfIndex]) {
        auto fn = reinterpret_cast<void*(*)(void*)>(vtable[kClientGetMinecraftGameVfIndex]);
        return fn(clientInstance);
    }

    return *reinterpret_cast<void**>(
        reinterpret_cast<char*>(clientInstance) + kClientMinecraftGameOffset);
}

inline void destroyBaseActorRenderContextInstance(void* barc) {
    if (!barc)
        return;

    auto** vtable = *reinterpret_cast<void***>(barc);
    if (!vtable || !vtable[0])
        return;

    auto dtor = reinterpret_cast<void(*)(void*)>(vtable[0]);
    dtor(barc);
}

inline void* getItemRendererFromBarc(void* barc) {
    if (!barc)
        return nullptr;

    return *reinterpret_cast<void**>(
        reinterpret_cast<std::byte*>(barc) + kBarcItemRendererOffset);
}
