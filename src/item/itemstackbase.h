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

    ItemStackBase();
    ItemStackBase(const ItemStackBase&);
    ItemStackBase& operator=(const ItemStackBase&);
    virtual ~ItemStackBase();

    virtual void reinit_item(const Item&, int, int);
    virtual void reinit_block(const void*, int);
    virtual void reinit_name(const void*, int, int);
    virtual void setNull(void*);

    virtual std::string toString() const;
    virtual std::string toDebugString() const;

};

using ItemStackBase_loadItem_t = void (*)(void* stack, void* compound);
extern ItemStackBase_loadItem_t ItemStackBase_loadItem;

using ItemStackBase_getDamageValue_t = short (*)(ItemStackBase*);
extern ItemStackBase_getDamageValue_t ItemStackBase_getDamageValue;

static_assert(sizeof(ItemStackBase) == 0x88, "Incorrect ItemStackBase size");
