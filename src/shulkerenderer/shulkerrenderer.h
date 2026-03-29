#pragma once
#include "ui/minecraftuirendercontext.h"
#include "item/itemstackbase.h"

#define SHULKER_SLOT_COUNT 27
#define SHULKER_TOOLTIP_ENTRY_COUNT 256

struct TooltipPreviewEntry {
    ItemStackBase* stack = nullptr;
    unsigned char id = 0;
};

inline TooltipPreviewEntry TooltipPreviewEntries[SHULKER_TOOLTIP_ENTRY_COUNT] = {};
inline unsigned char sNextTooltipPreviewId = 0;

inline unsigned char storeTooltipPreviewStack(ItemStackBase* hoveredStack) {
    ++sNextTooltipPreviewId;
    TooltipPreviewEntries[sNextTooltipPreviewId] = {
        hoveredStack,
        sNextTooltipPreviewId
    };
    return sNextTooltipPreviewId;
}

inline ItemStackBase* lookupTooltipPreviewStack(unsigned char tooltipId) {
    const TooltipPreviewEntry& entry = TooltipPreviewEntries[tooltipId];
    if (entry.id != tooltipId)
        return nullptr;
    return entry.stack;
}

class ShulkerRenderer {
public:
    static void render(
        MinecraftUIRenderContext* ctx,
        float tooltipX,
        float tooltipY,
        float tooltipWidth,
        float tooltipHeight,
        ItemStackBase* hoveredStack,
        char colorCode);
};
