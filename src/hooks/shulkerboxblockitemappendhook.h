#pragma once
#include "item/item.h"
#include "item/itemstack.h"
#include "nbt/nbt.h"
#include "item/itemstackbase.h"
#include "shulkerenderer/colors.h"
#include "shulkerenderer/shulkerrenderer.h"
#include "util/config.h"
#include "util/modmenu.h"
#include <string>

class ShulkerBoxBlockItem;

using Shulker_appendHover_t = void (*)(void*, ItemStackBase*, void*, std::string&, bool);

inline Shulker_appendHover_t ShulkerBoxBlockItem_appendFormattedHovertext_orig = nullptr;

inline uint16_t getItemIdDirect(Item* item) {
    return *reinterpret_cast<uint16_t*>(
        reinterpret_cast<uintptr_t>(item) + 0x8A
    );
}

inline void ShulkerBoxBlockItem_appendFormattedHovertext_hook(
    ShulkerBoxBlockItem* self,
    ItemStackBase* stack,
    void* level,
    std::string& out,
    bool flag)
{
    static constexpr char kHex[] = "0123456789abcdef";

    if (ShulkerBoxBlockItem_appendFormattedHovertext_orig)
        ShulkerBoxBlockItem_appendFormattedHovertext_orig(
            self, stack, level, out, flag);

    if (auto pos = out.find('\n'); pos != std::string::npos)
        out.erase(pos);

    if (!stack || !stack->mUserData)
        return;

    if (!ItemStackBase_loadItem)
        return;

    unsigned char tooltipId = storeTooltipPreviewStack(stack);

    char color = '0';

    if (Item* item = stack->mItem.get()) {
        uint16_t id = getItemIdDirect(item);
        color = getShulkerColorCodeFromItemId(id);
    }
    std::string prefix;
    prefix += "\xC2\xA7";
    prefix += kHex[(tooltipId >> 4) & 0xF];
    prefix += "\xC2\xA7";
    prefix += kHex[tooltipId & 0xF];
    prefix += "\xC2\xA7";
    prefix += color;
    
    out.insert(0, prefix);

    out += "\n§7Press §e";
    out += SP_keyCodeToString(spPreviewKey);
    out += "§7 to toggle preview";
    out += "\xC2\xA7v";
}