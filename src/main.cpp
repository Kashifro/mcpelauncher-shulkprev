#include <dlfcn.h>
#include <link.h>
#include <span>
#include <string>
#include "main.h"
#include <utility>
#include <optional>
#include <safetyhook.hpp>
#include <libhat.hpp>
#include <libhat/scanner.hpp>

BaseActorRenderContext_ctor_t BaseActorRenderContext_ctor = nullptr;
ItemRenderer_renderGuiItemNew_t ItemRenderer_renderGuiItemNew = nullptr;

extern "C" [[gnu::visibility("default")]] void mod_preinit() {}
extern "C" [[gnu::visibility("default")]] void mod_init()
{
    using namespace hat::literals::signature_literals;
    void *mcLib = dlopen("libminecraftpe.so", 0);
    if (!mcLib)
    {
        printf("[ShulkerPreview] failed to open libminecraftpe.so\n");
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
    dl_iterate_phdr([](dl_phdr_info *info, size_t, void *data)
                    { return (*static_cast<decltype(callback) *>(data))(*info); }, &callback);

    auto scan = [range1](const auto&... sig) {
        void* addr;
        ((addr = hat::find_pattern(range1, sig, hat::scan_alignment::X16).get()) || ...);
        return addr;
    };

    SP_loadConfig();
    SP_initModMenu();
    SP_register_keybinds();

    auto treeFindAddr = scan(
        "55 41 57 41 56 41 55 41 54 53 50 48 89 FB 4C 8B 6F 08 48 83 C3 08 4D 85 ED 0F 84 B6 00 00 00 4C"_sig
    );
    Nbt_treeFind =
        reinterpret_cast<Nbt_treeFind_t>(treeFindAddr);

    auto ItemStackbaseloadItemaddr = scan(
        "55 41 57 41 56 41 55 41 54 53 48 81 EC C8 00 00 00 49 89 F7 49 89 FC 64 48 8B 04 25 28 00 00 00 48 89 84 24 C0 00 00 00 48 8D 3D ?? ?? ?? ?? E8"_sig
    );
    ItemStackBase_loadItem =
        reinterpret_cast<ItemStackBase_loadItem_t>(ItemStackbaseloadItemaddr);

    auto isbgetdmgaddr = scan(
        "41 57 41 56 53 48 83 EC ?? 64 48 8B 04 25 ?? ?? ?? ?? 48 89 44 24 ?? 48 8B 47 ?? 48 85 C0 0F 84 ?? ?? ?? ?? ?? ?? ?? 00 0F 84 ?? ?? ?? ?? 4C 8B 77"_sig
    );
    ItemStackBase_getDamageValue = 
    reinterpret_cast<ItemStackBase_getDamageValue_t>(isbgetdmgaddr);

    auto BaseActorRenderContextCtorAddr = scan(
        "41 57 41 56 41 54 53 50 49 89 D7 49 89 F4 48 89 FB 48 8D 05 ?? ?? ?? ?? ?? ?? ?? 0F 57 C0"_sig
    );
    BaseActorRenderContext_ctor =
    reinterpret_cast<BaseActorRenderContext_ctor_t>(BaseActorRenderContextCtorAddr);

    auto ItemRendererRenderGuiItemNewAddr = scan(
        "55 41 57 41 56 41 55 41 54 53 48 81 EC E8 00 00 00 4C 89 4C 24 28 F3 0F 11 64 24 18 F3 0F 11 5C"_sig
    );
    ItemRenderer_renderGuiItemNew =
    reinterpret_cast<ItemRenderer_renderGuiItemNew_t>(ItemRendererRenderGuiItemNewAddr);

    
    // ShulkerBoxBlockItem
    auto ZTS19ShulkerBoxBlockItem = hat::find_pattern(range1, hat::object_to_signature("19ShulkerBoxBlockItem")).get();
    auto _ZTI19ShulkerBoxBlockItem = hat::find_pattern(range2, hat::object_to_signature(ZTS19ShulkerBoxBlockItem)).get() - sizeof(void *);
    auto _ZTV19ShulkerBoxBlockItem = hat::find_pattern(range2, hat::object_to_signature(_ZTI19ShulkerBoxBlockItem)).get() + sizeof(void *);
    void **vtshulk55 = reinterpret_cast<void **>(_ZTV19ShulkerBoxBlockItem);
    // append slot 55
    ShulkerBoxBlockItem_appendFormattedHovertext_orig =
        reinterpret_cast<Shulker_appendHover_t>(vtshulk55[55]);
    vtshulk55[55] = reinterpret_cast<void *>(&ShulkerBoxBlockItem_appendFormattedHovertext_hook);

    // HovertextRenderer
    auto _ZTS17HoverTextRenderer = hat::find_pattern(range1, hat::object_to_signature("17HoverTextRenderer")).get();
    auto _ZTI17HoverTextRenderer = hat::find_pattern(range2, hat::object_to_signature(_ZTS17HoverTextRenderer)).get() - sizeof(void *);
    auto _ZTV17HoverTextRenderer = hat::find_pattern(range2, hat::object_to_signature(_ZTI17HoverTextRenderer)).get() + sizeof(void *);
    // slot17
    void **vtHR = reinterpret_cast<void **>(_ZTV17HoverTextRenderer);
    HoverRenderer_renderHoverBox_orig =
        reinterpret_cast<RenderHoverBoxFn>(vtHR[17]);
    vtHR[17] = reinterpret_cast<void *>(&HoverRenderer_renderHoverBox_hook);

    auto _ZTSMCUIRC = hat::find_pattern(range1, hat::object_to_signature("24MinecraftUIRenderContext")).get();
    auto _ZTIMCUIRC = hat::find_pattern(range2, hat::object_to_signature(_ZTSMCUIRC)).get() - sizeof(void *);
    auto _ZTVMCUIRC = hat::find_pattern(range2, hat::object_to_signature(_ZTIMCUIRC)).get() + sizeof(void *);
    // slot 6
    void **vtMCUIRC = reinterpret_cast<void **>(_ZTVMCUIRC);
    MinecraftUIRenderContext_drawText_orig =
        reinterpret_cast<MinecraftUIRenderContext_drawText_t>(vtMCUIRC[6]);
    vtMCUIRC[6] = reinterpret_cast<void *>(&MinecraftUIRenderContext_drawText_hook);
}
