#pragma once
#include "ui/hoverrenderer.h"
#include "shulkerenderer/shulkerrenderer.h"
#include "util/keybinds.h"
#include <string>

static bool sPreviewEnabled = false;
static bool sWasToggleKeyDown = false;

using RenderHoverBoxFn = void (*)(void*, MinecraftUIRenderContext*, void*, void*, float);

inline RenderHoverBoxFn HoverRenderer_renderHoverBox_orig = nullptr;

static inline int hex(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0;
}

inline void HoverRenderer_renderHoverBox_hook(
    void* selfPtr,
    MinecraftUIRenderContext* ctx,
    void* client,
    void* aabb,
    float someFloat)
{
    HoverRenderer* self = reinterpret_cast<HoverRenderer*>(selfPtr);

    HoverRenderer_renderHoverBox_orig(selfPtr, ctx, client, aabb, someFloat);

    if (!ctx){
        sWasToggleKeyDown = false;
        return;
    }

    //toggle prev key
    if (!spKeyDown && sWasToggleKeyDown)
        sPreviewEnabled = !sPreviewEnabled;
    sWasToggleKeyDown = spKeyDown;

    if (!sPreviewEnabled)
        return;

    const std::string& text = self->mFilteredContent;

    //marker
    if (text.find("\xC2\xA7v") == std::string::npos)
        return;

    //prefix must contain §H§L§C
    if (text.size() < 9)
        return;
    //decode cache index
    char hi = text[2];
    char lo = text[5];
    int index = (hex(hi) << 4) | hex(lo);
    index &= (SHULKER_CACHE_SIZE - 1);

    if (index < 0 || index >= SHULKER_CACHE_SIZE)
        return;

    // decode color code
    char colorCode = text[8];

    float px = self->mCursorX + self->mOffsetX;
    float py = self->mCursorY + self->mOffsetY + self->mBoxHeight;

    ShulkerRenderer::render(ctx, px, py, index, colorCode);
}