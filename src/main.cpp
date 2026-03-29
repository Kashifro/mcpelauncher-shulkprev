#include <dlfcn.h>
#include <link.h>
#include <span>
#include <string>
#include <cstdio>
#include <sys/mman.h>
#include <unistd.h>
#include "main.h"
#include <libhat.hpp>
#include <libhat/scanner.hpp>

using namespace hat::literals::signature_literals;

BaseActorRenderContext_ctor_t BaseActorRenderContext_ctor = nullptr;
ItemRenderer_renderGuiItemNew_t ItemRenderer_renderGuiItemNew = nullptr;
void** ItemStack_vtable = nullptr;

extern "C" [[gnu::visibility("default")]] void mod_preinit() {}

namespace {

#if defined(ARCH_ARM64)
constexpr auto kAlign = hat::scan_alignment::X1;
#else
constexpr auto kAlign = hat::scan_alignment::X16;
#endif

bool makeWritable(void* addr) {
    long pageSize = sysconf(_SC_PAGESIZE);
    uintptr_t start = (uintptr_t)addr & ~(pageSize - 1);
    return mprotect((void*)start, pageSize, PROT_READ | PROT_WRITE | PROT_EXEC) == 0;
}

#if defined(ARCH_X86_64)
constexpr auto kNbtTreeFindSignature =
"55 41 57 41 56 41 55 41 54 53 50 48 89 FB 4C 8B 6F 08 48 83 C3 08 4D 85 ED 0F 84 B6 00 00 00 4C"_sig;
constexpr auto kItemStackBaseLoadItemSignature =
"55 41 57 41 56 41 55 41 54 53 48 81 EC ? ? ? ? 49 89 F7 49 89 FC 64 48 8B 04 25 ? ? ? ? 48 89 84 24 ? ? ? ? 48 8D 3D"_sig;
constexpr auto kItemStackBaseGetDamageValueSignature =
"41 57 41 56 53 48 83 EC ? 64 48 8B 04 25 ? ? ? ? 48 89 44 24 ? 48 8B 47 ? 48 85 C0 0F 84 ? ? ? ? ? ? ? 00 0F 84 ? ? ? ? 4C 8B 77"_sig;
constexpr auto kItemStackBaseCtorSignature =
"41 57 41 56 53 49 89 FE 48 8D 05 D1 3B B5 02 48 89 07 48 8D 5F 08 0F 57 C0 0F 11 47 08 0F 11 47"_sig;
constexpr auto kBaseActorRenderContextCtorSignature =
"41 57 41 56 41 54 53 50 49 89 D7 49 89 F4 48 89 FB 48 8D 05 ? ? ? ? ? ? ? 0F 57 C0"_sig;
constexpr auto kItemRendererRenderGuiItemNewSignature =
"55 41 57 41 56 41 55 41 54 53 48 81 EC E8 00 00 00 4C 89 4C 24 28 F3 0F 11 64 24 18 F3 0F 11 5C"_sig;

#elif defined(ARCH_ARM64)
constexpr auto kNbtTreeFindSignature =
"? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 FD 03 00 91 F3 03 00 AA ? ? ? F8 ? ? ? B4 ? ? ? A9 F5 03 13 AA ? ? ? 14 ? ? ? 52 ? ? ? 71 ? ? ? 54 ? ? ? 91 ? ? ? F9 ? ? ? B4 ? ? ? 39 ? ? ? 36 ? ? ? F9 ? ? ? 36 ? ? ? F9 1F 03 16 EB E0 03 14 AA 02 33 96 9A ? ? ? 94 ? ? ? 34 ? ? ? 37 ? ? ? 52 ? ? ? 71 ? ? ? 54 ? ? ? 14 ? ? ? 91 ? ? ? 37 ? ? ? D3 1F 03 16 EB E0 03 14 AA 02 33 96 9A ? ? ? 94 ? ? ? 35 DF 02 18 EB ? ? ? 54 E8 03 1F 2A ? ? ? 71 ? ? ? 54 F5 03 17 AA ? ? ? F9 ? ? ? B5 ? ? ? 14 ? ? ? 54 ? ? ? 17 BF 02 13 EB ? ? ? 54 ? ? ? 39 ? ? ? A9 E0 03 14 AA ? ? ? D3 ? ? ? 72 ? ? ? 91 01 01 8B 9A 57 01 89 9A FF 02 16 EB E2 32 96 9A ? ? ? 94 DF 02 17 EB E8 27 9F 1A 1F 00 00 71 E9 A7 9F 1A 08 01 89 1A 1F 01 00 71 73 12 95 9A E0 03 13 AA ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A8 C0 03 5F D6 ? ? ? A9"_sig;
constexpr auto kItemStackBaseLoadItemSignature =
"? ? ? D1 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? 91 ? ? ? D5 F3 03 00 AA ? ? ? ? ? ? ? 91 ? ? ? F9 F5 03 01 AA"_sig;
constexpr auto kItemStackBaseGetDamageValueSignature =
"? ? ? D1 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? 91 ? ? ? D5 ? ? ? F9 ? ? ? F8 ? ? ? F9 ? ? ? B4 ? ? ? F9 ? ? ? B4 ? ? ? F9 ? ? ? B4"_sig;
constexpr auto kBaseActorRenderContextCtorSignature =
"? ? ? A9 ? ? ? A9 ? ? ? A9 FD 03 00 91 ? ? ? ? ? ? ? 91 ? ? ? A9 ? ? ? A9 F3 03 00 AA F4 03 02 AA"_sig;
constexpr auto kItemRendererRenderGuiItemNewSignature =
"FF C3 05 D1 EC 73 00 FD EB 2B 0F 6D E9 23 10 6D FD 7B 11 A9 FC 6F 12 A9 FA 67 13 A9 F8 5F 14 A9 F6 57 15 A9 F4 4F 16 A9 FD 43 04 91 5B D0 3B D5"_sig;
constexpr auto kItemStackBaseCtorSignature =
"?? ?? ?? A9 ?? ?? ?? F9 ?? ?? ?? A9 FD 03 00 91 ?? ?? ?? 6F ?? ?? ?? ?? ?? ?? ?? 91 F4 03 00 AA ?? ?? ?? F9 F3 03 00 AA ?? ?? ?? 52 ?? ?? ?? F8 ?? ?? ?? A9 ?? ?? ?? B8 ?? ?? ?? 78 ?? ?? ?? 39 ?? ?? ?? 3C ?? ?? ?? 3C ?? ?? ?? 3C ?? ?? ?? 3C ?? ?? ?? 3C ?? ?? ?? F9 ?? ?? ?? 94"_sig;
#endif

} // namespace

extern "C" [[gnu::visibility("default")]] void mod_init()
{
    void* mcLib = dlopen("libminecraftpe.so", RTLD_NOW);
    if (!mcLib) {
        printf("[SP] failed to open libminecraftpe.so\n");
        return;
    }

    std::span<std::byte> range1, range2;
    auto callback = [&](const dl_phdr_info &info)
    {
        if (auto h = dlopen(info.dlpi_name, RTLD_NOLOAD); dlclose(h), h != mcLib)
            return 0;
        range1 = {reinterpret_cast<std::byte *>(info.dlpi_addr + info.dlpi_phdr[1].p_vaddr), info.dlpi_phdr[1].p_memsz};
        range2 = {reinterpret_cast<std::byte *>(info.dlpi_addr + info.dlpi_phdr[2].p_vaddr), info.dlpi_phdr[2].p_memsz};
        return 1;
    };
    dl_iterate_phdr([](dl_phdr_info *info, size_t, void *data) { return (*static_cast<decltype(callback) *>(data))(*info); }, &callback);

    auto scan = [range1](const auto&... sig) {
        void* addr;
        ((addr = hat::find_pattern(range1, sig, hat::scan_alignment::X16).get()) || ...);
        return addr;
    };

    SP_loadConfig();
    SP_initModMenu();
    SP_register_keybinds();

#define FIND(sig) hat::find_pattern(range1, sig, kAlign).get()

    Nbt_treeFind = (Nbt_treeFind_t)FIND(kNbtTreeFindSignature);
    ItemStackBase_loadItem = (ItemStackBase_loadItem_t)FIND(kItemStackBaseLoadItemSignature);
    ItemStackBase_getDamageValue = (ItemStackBase_getDamageValue_t)FIND(kItemStackBaseGetDamageValueSignature);
    BaseActorRenderContext_ctor = (BaseActorRenderContext_ctor_t)FIND(kBaseActorRenderContextCtorSignature);
    ItemRenderer_renderGuiItemNew = (ItemRenderer_renderGuiItemNew_t)FIND(kItemRendererRenderGuiItemNewSignature);
    ItemStackBase_ctor = (ItemStackBase_ctor_t)FIND(kItemStackBaseCtorSignature);

#undef FIND
    //vtable hooks
    auto ZTS = hat::find_pattern(range1, hat::object_to_signature("19ShulkerBoxBlockItem")).get();
    auto ZTI = hat::find_pattern(range2, hat::object_to_signature(ZTS)).get() - sizeof(void*);
    auto ZTV = hat::find_pattern(range2, hat::object_to_signature(ZTI)).get() + sizeof(void*);
    void** vt = (void**)ZTV;

    makeWritable(&vt[55]);
    ShulkerBoxBlockItem_appendFormattedHovertext_orig = (Shulker_appendHover_t)vt[55];
    vt[55] = (void*)&ShulkerBoxBlockItem_appendFormattedHovertext_hook;

    auto ZTS2 = hat::find_pattern(range1, hat::object_to_signature("17HoverTextRenderer")).get();
    auto ZTI2 = hat::find_pattern(range2, hat::object_to_signature(ZTS2)).get() - sizeof(void*);
    auto ZTV2 = hat::find_pattern(range2, hat::object_to_signature(ZTI2)).get() + sizeof(void*);
    void** vt2 = (void**)ZTV2;

    makeWritable(&vt2[17]);
    HoverRenderer_renderHoverBox_orig = (RenderHoverBoxFn)vt2[17];
    vt2[17] = (void*)&HoverRenderer_renderHoverBox_hook;

    auto ZTS3 = hat::find_pattern(range1, hat::object_to_signature("9ItemStack")).get();
    auto ZTI3 = hat::find_pattern(range2, hat::object_to_signature(ZTS3)).get() - sizeof(void*);
    auto ZTV3 = hat::find_pattern(range2, hat::object_to_signature(ZTI3)).get() + sizeof(void*);
    void** vtFull = (void**)ZTV3;

    ItemStack_vtable = &vtFull[2];
}