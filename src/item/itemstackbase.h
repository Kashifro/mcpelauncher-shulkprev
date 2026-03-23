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
    uint8_t _pad_18[0x80 - 0x18];
    virtual ~ItemStackBase(); //lol idk why removing this virtual makes renderer not work
};

using ItemStackBase_loadItem_t = void (*)(void* stack, void* compound);
extern ItemStackBase_loadItem_t ItemStackBase_loadItem;

using ItemStackBase_getDamageValue_t = short (*)(ItemStackBase*);
extern ItemStackBase_getDamageValue_t ItemStackBase_getDamageValue;

using ItemStackBase_ctor_t = void (*)(ItemStackBase*);
extern ItemStackBase_ctor_t ItemStackBase_ctor;

/*
The ItemStackBase Ctor makes its last write at
*(_QWORD *)(a1 + 120) = 0;
*/
static_assert(sizeof(ItemStackBase) == 0x80, "Incorrect ItemStackBase size");
