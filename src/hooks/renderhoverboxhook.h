#pragma once
#include "ui/hoverrenderer.h"
#include "shulkerenderer/shulkerrenderer.h"
#include "util/keybinds.h"
#include <string>

static bool sPreviewEnabled = false;
static bool sWasToggleKeyDown = false;

using RenderHoverBoxFn = void (*)(void*, MinecraftUIRenderContext*, void*, void*, float);

inline RenderHoverBoxFn HoverRenderer_renderHoverBox_orig = nullptr;

inline int decodeHexNibble(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
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

    if (!spKeyDown && sWasToggleKeyDown)
        sPreviewEnabled = !sPreviewEnabled;
    sWasToggleKeyDown = spKeyDown;

    if (!sPreviewEnabled)
        return;
    
    const std::string& text = self->mFilteredContent;

    if (text.find("\xC2\xA7v") == std::string::npos)
        return;

    if (text.size() < 9)
        return;

    int hi = decodeHexNibble(text[2]);
    int lo = decodeHexNibble(text[5]);
    if (hi < 0 || lo < 0)
        return;

    ItemStackBase* hoveredStack =
        lookupTooltipPreviewStack(static_cast<unsigned char>((hi << 4) | lo));
    if (!hoveredStack)
        return;

    char colorCode = text[8];

    ShulkerRenderer::render(
        ctx,
        self->mCursorX + self->mOffsetX,
        self->mCursorY + self->mOffsetY,
        self->mBoxWidth,
        self->mBoxHeight,
        hoveredStack,
        colorCode
    );
}