#pragma once
#include <cstdint>
#include <string>
#include "sharedptr.h"

class Item;
class CompoundTag;

class ItemStackBase {
public:
    WeakPtr<Item>   mItem;
    CompoundTag*    mUserData;
    uint8_t _pad_18[0x88 - 0x18];
    
public:
    virtual ~ItemStackBase();
};

using ItemStackBase_loadItem_t = void (*)(void* stack, void* compound);
extern ItemStackBase_loadItem_t ItemStackBase_loadItem;

using ItemStackBase_getDamageValue_t = short (*)(ItemStackBase*);
extern ItemStackBase_getDamageValue_t ItemStackBase_getDamageValue;

using ItemStackBase_ctor_t = void (*)(ItemStackBase*);
extern ItemStackBase_ctor_t ItemStackBase_ctor;

static_assert(sizeof(ItemStackBase) == 0x88, "Incorrect arm64 ItemStackBase size");
