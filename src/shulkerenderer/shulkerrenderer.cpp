#include "shulkerrenderer.h"

#include <cstdio>
#include <cstring>

#include "colors.h"
#include "item/item.h"
#include "item/itemstack.h"
#include "nbt/nbt.h"
#include "render/barchelper.h"
#include "ui/previewui.h"
#include "util/config.h"

namespace {

float clamp01(float v) {
    if (v < 0) return 0;
    if (v > 1) return 1;
    return v;
}

mce::Color applyTintIntensity(const mce::Color& base) {
    float i = spTintIntensity;
    return {
        clamp01(base.r * i),
        clamp01(base.g * i),
        clamp01(base.b * i),
        base.a
    };
}

bool getDurabilityInfo(
    ItemStackBase* stack,
    int storedDamageValue,
    int& remaining,
    float& ratio)
{
    if (!ItemStackBase_getDamageValue)
        return false;

    Item* item = stack->mItem.get();
    if (!item)
        return false;

    short maxDamage = item->getMaxDamage();
    if (maxDamage <= 0)
        return false;

    short damage = ItemStackBase_getDamageValue(stack);
    if (damage <= 0 && storedDamageValue > 0)
        damage = static_cast<short>(storedDamageValue);
    if (damage <= 0)
        return false;

    remaining = maxDamage - damage;
    if (remaining < 0)
        remaining = 0;
    ratio = static_cast<float>(remaining) / static_cast<float>(maxDamage);
    return true;
}

struct ShulkerSlotRenderData {
    void* itemTag = nullptr;
    uint8_t count = 0;
    bool valid = false;
    bool hasGlint = false;
    int damageValue = -1;
};

struct alignas(16) StackStorage {
    std::byte bytes[sizeof(ItemStackBase)];
};

bool buildRenderDataFromNbt(
    ItemStackBase* hoveredStack,
    ShulkerSlotRenderData (&slots)[SHULKER_SLOT_COUNT])
{
    if (!hoveredStack || !hoveredStack->mUserData)
        return false;

    auto* list = reinterpret_cast<ListTagLayout*>(getListTag(hoveredStack->mUserData, "Items"));
    if (!list)
        list = reinterpret_cast<ListTagLayout*>(getListTag(hoveredStack->mUserData, "items"));
    if (!list)
        return false;

    for (auto& slot : slots)
        slot = {};

    int size = listSize(list);
    for (int i = 0; i < size; ++i) {
        void* tag = listAt(list, i);
        if (!tag)
            continue;

        int slotValue = 0;
        if (!readIntTag(tag, "Slot", slotValue)
         && !readIntTag(tag, "slot", slotValue))
            continue;

        int countValue = 1;
        if (!readIntTag(tag, "Count", countValue)
         && !readIntTag(tag, "count", countValue))
            countValue = 1;

        if (countValue < 0)   countValue = 0;
        if (countValue > 255) countValue = 255;

        if (slotValue < 0 || slotValue >= SHULKER_SLOT_COUNT)
            continue;

        auto& slot = slots[slotValue];
        slot.itemTag = tag;
        slot.count = static_cast<uint8_t>(countValue);
        slot.valid = true;
        slot.hasGlint = containsEnchantmentTagTree(tag);
        readItemDamageTagValue(tag, slot.damageValue);
    }

    return true;
}

ItemStackBase* getRenderableStack(
    ItemStackBase* const (&stacks)[SHULKER_SLOT_COUNT],
    int slot)
{
    ItemStackBase* stack = stacks[slot];
    if (!stack)
        return nullptr;
    if (!stack->mItem.get())
        return nullptr;
    return stack;
}

void buildTransientStacks(
    const ShulkerSlotRenderData (&slotData)[SHULKER_SLOT_COUNT],
    StackStorage (&storage)[SHULKER_SLOT_COUNT],
    ItemStackBase* (&stacks)[SHULKER_SLOT_COUNT])
{
    if (!ItemStackBase_loadItem || !ItemStackBase_ctor)
        return;

    static_assert(alignof(StackStorage) >= 16, "Preview stacks must stay 16-byte aligned");

    for (int slot = 0; slot < SHULKER_SLOT_COUNT; ++slot) {
        if (!slotData[slot].valid || !slotData[slot].itemTag)
            continue;

        auto* stack = reinterpret_cast<ItemStackBase*>(storage[slot].bytes);

        ItemStackBase_ctor(stack);

        if (ItemStack_vtable)
            *reinterpret_cast<void***>(stack) = ItemStack_vtable;

        ItemStackBase_loadItem(stack, slotData[slot].itemTag);

        if (stack->mItem.get())
            stacks[slot] = stack;
    }
}

void drawSlotIcons(
    MinecraftUIRenderContext& ctx,
    const ShulkerSlotRenderData (&slotData)[SHULKER_SLOT_COUNT],
    ItemStackBase* const (&stacks)[SHULKER_SLOT_COUNT],
    float ox,
    float oy)
{
    if (!BaseActorRenderContext_ctor || !ItemRenderer_renderGuiItemNew)
        return;

    if (!ctx.mClient || !ctx.mScreenContext)
        return;

    RectangleArea dummy{-10000, -9999, -10000, -9999};
    ctx.fillRectangle(dummy, PreviewUi::kWhite, 1.0f);
    ctx.flushImages(PreviewUi::kWhite, 1.0f, PreviewUi::flushMaterial());

    void* clientInstance = ctx.mClient;
    void* minecraftGame = getMinecraftGameFromClient(clientInstance);
    if (!minecraftGame) return;

    alignas(16) std::byte barcStorage[kBarcStorageSize]{};
    void* barc = barcStorage;

    BaseActorRenderContext_ctor(barc, ctx.mScreenContext, clientInstance, minecraftGame);

    void* itemRenderer = getItemRendererFromBarc(barc);
    if (!itemRenderer) {
        destroyBaseActorRenderContextInstance(barc);
        return;
    }

    bool anyGlint = false;

    PreviewUi::forEachSlot(ox, oy, [&](int slot, float x, float y) {
        ItemStackBase* stack = getRenderableStack(stacks, slot);
        if (!stack) return;

        if (slotData[slot].hasGlint) anyGlint = true;

        float dx = x + PreviewUi::kItemInset;
        float dy = y + PreviewUi::kItemInset;

        float px = dx;
        float py = dy;

        ItemRenderer_renderGuiItemNew(
            itemRenderer,
            barc,
            stack,
            0, 0, 0,
            px, py,
            1.0f, 1.0f, 1.0f
        );
    });

    if (anyGlint) {
        PreviewUi::forEachSlot(ox, oy, [&](int slot, float x, float y) {
            if (!slotData[slot].hasGlint) return;

            ItemStackBase* stack = getRenderableStack(stacks, slot);
            if (!stack) return;

            float dx = x + PreviewUi::kItemInset;
            float dy = y + PreviewUi::kItemInset;

            float px = dx;
            float py = dy;

            ItemRenderer_renderGuiItemNew(
                itemRenderer,
                barc,
                stack,
                0, 1, 1,
                px, py,
                1.0f, 1.0f, 1.0f
            );
        });
    }

    destroyBaseActorRenderContextInstance(barc);
}

} // namespace

void ShulkerRenderer::render(
    MinecraftUIRenderContext* ctx,
    float tooltipX,
    float tooltipY,
    float tooltipWidth,
    float tooltipHeight,
    ItemStackBase* hoveredStack,
    char colorCode)
{
    (void)tooltipWidth;

    if (!ctx || !hoveredStack)
        return;
    if (!ItemStackBase_loadItem)
        return;

    ShulkerSlotRenderData slotData[SHULKER_SLOT_COUNT]{};
    if (!buildRenderDataFromNbt(hoveredStack, slotData))
        return;

    ItemStackBase* transientStacks[SHULKER_SLOT_COUNT]{};
    StackStorage transientStorage[SHULKER_SLOT_COUNT]{};
    buildTransientStacks(slotData, transientStorage, transientStacks);

    const PreviewUi::Placement placement = PreviewUi::placePanel(
        *ctx,
        tooltipX,
        tooltipY,
        tooltipHeight,
        spAnchorAboveTooltip);

    float x = placement.x;
    float y = placement.y;

    RectangleArea panel = PreviewUi::makePanelRect(x, y);

    const mce::Color tint = applyTintIntensity(getShulkerTint(colorCode));
    const auto& tex = PreviewUi::getTextures(*ctx);

    float ox = x + PreviewUi::kPanelPadding;
    float oy = y + PreviewUi::kPanelPadding;

    PreviewUi::drawPanel(*ctx, tex, panel);
    ctx->flushImages(tint, 1.0f, PreviewUi::flushMaterial());

    PreviewUi::forEachSlot(ox, oy, [&](int, float sx, float sy) {
        PreviewUi::drawSlot(*ctx, tex, PreviewUi::makeSlotRect(sx, sy));
    });

    ctx->flushImages(tint, 1.0f, PreviewUi::flushMaterial());

    drawSlotIcons(*ctx, slotData, transientStacks, ox, oy);

    PreviewUi::forEachSlot(ox, oy, [&](int slot, float sx, float sy) {
        if (!slotData[slot].valid)
            return;

        ItemStackBase* stack = transientStacks[slot];
        if (!stack) return;

        int remaining = 0;
        float ratio = 0.0f;

        if (getDurabilityInfo(stack, slotData[slot].damageValue, remaining, ratio)) {
            PreviewUi::drawDurabilityBar(*ctx, sx, sy, ratio);
        }

        if (slotData[slot].count > 1) {
            char txt[8];
            std::snprintf(txt, sizeof(txt), "%u", slotData[slot].count);
            PreviewUi::drawStackCountText(*ctx, sx, sy, txt);
        }
    });

    ctx->flushText(0.0f, std::nullopt);
}