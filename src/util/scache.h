#pragma once
#include <cstddef>
#include <cstdint>

class ItemStackBase;
//this isnt even required prolly 
struct ItemStackBaseStorage {
    bool constructed = false;         // whether ctor has been called
    alignas(16) std::byte data[0xA0]; // exact ItemStackBase size
};

static inline ItemStackBase* asISB(ItemStackBaseStorage& s) {
    return reinterpret_cast<ItemStackBase*>(s.data);
}

#define SHULKER_CACHE_SIZE 16
#define SHULKER_SLOT_COUNT 27

struct ShulkerSlotCache {
    ItemStackBaseStorage isb;// real cached ItemStackBase
    uint8_t count;           // stable stack size
    bool valid;              // slot contains an item
    bool enchanted;          // has enchant glint
};

extern ShulkerSlotCache ShulkerCache[SHULKER_CACHE_SIZE][SHULKER_SLOT_COUNT];
