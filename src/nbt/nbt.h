#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>

struct NbtTreeKey {
    const char* data;
    std::size_t len;
};

using Nbt_treeFind_t = void* (*)(void* tree, const NbtTreeKey* key);
extern Nbt_treeFind_t Nbt_treeFind;

struct ListTagLayout {
    void* vtable;
    void* begin;
    void* end;
    void* cap;
    uint8_t type;
};

inline void* treeFindNode(void* compound, const char* key, std::size_t len) {
    if (!compound || !Nbt_treeFind)
        return nullptr;

    NbtTreeKey searchKey{key, len};

    auto* treeRoot = reinterpret_cast<std::byte*>(compound) + 8;
    auto* treeEnd  = reinterpret_cast<std::byte*>(compound) + 16;

    void* node = Nbt_treeFind(treeRoot, &searchKey);
    return node == treeEnd ? nullptr : node;
}

inline uint32_t nodeType(void* node) {
    return *reinterpret_cast<uint32_t*>(
        reinterpret_cast<std::byte*>(node) + 96);
}

inline void* nodePayload(void* node) {
    return reinterpret_cast<std::byte*>(node) + 56;
}

inline std::byte* nodeValue(void* node) {
    return reinterpret_cast<std::byte*>(node) + 64;
}

template <std::size_t N>
inline bool containsTag(void* compound, const char (&key)[N]) {
    return treeFindNode(compound, key, N - 1) != nullptr;
}

inline bool containsTag(void* compound, const char* key) {
    if (!key)
        return false;
    return treeFindNode(compound, key, std::strlen(key)) != nullptr;
}

template <std::size_t N>
inline void* getListTag(void* compound, const char (&key)[N]) {
    void* node = treeFindNode(compound, key, N - 1);
    if (!node || nodeType(node) != 9)
        return nullptr;

    return nodePayload(node);
}

inline void* getListTag(void* compound, const char* key) {
    if (!key)
        return nullptr;
    void* node = treeFindNode(compound, key, std::strlen(key));
    if (!node || nodeType(node) != 9)
        return nullptr;

    return nodePayload(node);
}

template <std::size_t N>
inline void* getCompoundTag(void* compound, const char (&key)[N]) {
    void* node = treeFindNode(compound, key, N - 1);
    if (!node || nodeType(node) != 10)
        return nullptr;

    return nodePayload(node);
}

inline void* getCompoundTag(void* compound, const char* key) {
    if (!key)
        return nullptr;
    void* node = treeFindNode(compound, key, std::strlen(key));
    if (!node || nodeType(node) != 10)
        return nullptr;

    return nodePayload(node);
}

inline int listSize(ListTagLayout* list) {
    if (!list || !list->begin || !list->end)
        return 0;

    auto diff =
        reinterpret_cast<std::byte*>(list->end) -
        reinterpret_cast<std::byte*>(list->begin);

    if (diff <= 0)
        return 0;

    return static_cast<int>(diff / sizeof(void*));
}

inline void* listAt(ListTagLayout* list, int index) {
    if (!list || index < 0)
        return nullptr;

    int size = listSize(list);
    if (index >= size)
        return nullptr;

    auto** items = reinterpret_cast<void**>(list->begin);
    return items[index];
}

inline bool containsEnchantmentTag(void* compound) {
    if (!compound || !Nbt_treeFind)
        return false;

    return containsTag(compound, "ench") ||
           containsTag(compound, "Enchantments") ||
           containsTag(compound, "StoredEnchantments") ||
           containsTag(compound, "minecraft:enchantments") ||
           containsTag(compound, "minecraft:stored_enchantments");
}

inline bool containsEnchantmentTagTree(void* compound) {
    if (!compound)
        return false;
    if (containsEnchantmentTag(compound))
        return true;

    void* nestedTag = getCompoundTag(compound, "tag");
    if (nestedTag && containsEnchantmentTagTree(nestedTag))
        return true;

    void* componentsTag = getCompoundTag(compound, "components");
    return componentsTag && containsEnchantmentTagTree(componentsTag);
}

template <std::size_t N>
inline bool readIntTag(void* compound, const char (&key)[N], int& outValue) {
    void* node = treeFindNode(compound, key, N - 1);
    if (!node)
        return false;
    switch (nodeType(node)) {
    case 1:
        outValue = *reinterpret_cast<uint8_t*>(nodeValue(node));
        return true;
    case 2:
        outValue = *reinterpret_cast<uint16_t*>(nodeValue(node));
        return true;
    case 3:
        outValue = *reinterpret_cast<int32_t*>(nodeValue(node));
        return true;
    default:
        return false;
    }
}

inline bool readIntTag(void* compound, const char* key, int& outValue) {
    if (!key)
        return false;
    void* node = treeFindNode(compound, key, std::strlen(key));
    if (!node)
        return false;

    switch (nodeType(node)) {
    case 1:
        outValue = *reinterpret_cast<uint8_t*>(nodeValue(node));
        return true;
    case 2:
        outValue = *reinterpret_cast<uint16_t*>(nodeValue(node));
        return true;
    case 3:
        outValue = *reinterpret_cast<int32_t*>(nodeValue(node));
        return true;
    default:
        return false;
    }
}

inline bool readItemDamageTagValue(void* compound, int& outValue) {
    if (!compound)
        return false;

    if (readIntTag(compound, "Damage", outValue) ||
        readIntTag(compound, "damage", outValue) ||
        readIntTag(compound, "minecraft:damage", outValue))
        return true;

    void* nestedTag = getCompoundTag(compound, "tag");
    if (nestedTag && readItemDamageTagValue(nestedTag, outValue))
        return true;

    void* componentsTag = getCompoundTag(compound, "components");
    return componentsTag && readItemDamageTagValue(componentsTag, outValue);
}
