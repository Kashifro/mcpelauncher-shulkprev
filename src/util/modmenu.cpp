#include "modmenu.h"

#include <cstdio>
#include <dlfcn.h>
#include <string>

#include "config.h"

namespace {
struct MenuEntryABI {
    const char* name;
    void* user;
    bool (*selected)(void* user);
    void (*click)(void* user);
    size_t length;
    MenuEntryABI* subentries;
};

struct control {
    int type;
    union {
        struct {
            const char* label;
            void* user;
            void (*onClick)(void* user);
        } button;
        struct {
            const char* label;
            int min;
            int def;
            int max;
            void* user;
            void (*onChange)(void* user, int value);
        } sliderint;
        struct {
            const char* label;
            float min;
            float def;
            float max;
            void* user;
            void (*onChange)(void* user, float value);
        } sliderfloat;
        struct {
            char* label;
            int size;
        } text;
        struct {
            const char* label;
            const char* def;
            const char* placeholder;
            void* user;
            void (*onChange)(void* user, const char* value);
        } textinput;
    } data;
};

inline void (*showWindowFn)(
    const char* title,
    int isModal,
    void* user,
    void (*onClose)(void* user),
    int count,
    control* controls
) = nullptr;
inline void (*closeWindowFn)(const char* title) = nullptr;
inline void (*addMenuFn)(size_t length, MenuEntryABI* entries) = nullptr;

constexpr const char* keybindWindowTitle = "Shulker Preview Key";
constexpr const char* tintIntensityWindowTitle = "Shulker Preview Tint";

std::string keybindPrompt;
control keybindControl{};

std::string tintIntensityHeader = "Adjust shulker tint intensity (%)";
control tintIntensityControls[2]{};

void closeMenuWindow(const char* title) {
    if (closeWindowFn)
        closeWindowFn(title);
}

void onKeybindWindowClosed(void*) {
    spChangingPreviewKey = false;
    SP_saveConfig();
}

int currentTintPercent() {
    return SP_clampPercent(static_cast<int>(spTintIntensity * 100.0f + 0.5f));
}

void onTintIntensityChanged(void*, int value) {
    spTintIntensity = static_cast<float>(SP_clampPercent(value)) / 100.0f;
    SP_saveConfig();
}

void onTintIntensityWindowClosed(void*) {
    SP_saveConfig();
}

void toggleAnchorMode(void*) {
    spAnchorAboveTooltip = !spAnchorAboveTooltip;
    SP_saveConfig();
}

void rebuildTintIntensityControls() {
    tintIntensityControls[0].type = 3;
    tintIntensityControls[0].data.text.label = const_cast<char*>(tintIntensityHeader.c_str());
    tintIntensityControls[0].data.text.size = 0;

    tintIntensityControls[1].type = 1;
    tintIntensityControls[1].data.sliderint.label = "Intensity";
    tintIntensityControls[1].data.sliderint.min = 0;
    tintIntensityControls[1].data.sliderint.def = currentTintPercent();
    tintIntensityControls[1].data.sliderint.max = 200;
    tintIntensityControls[1].data.sliderint.user = nullptr;
    tintIntensityControls[1].data.sliderint.onChange = onTintIntensityChanged;
}
} // namespace

std::string SP_keyCodeToString(int keyCode) {
    if ((keyCode >= '0' && keyCode <= '9') || (keyCode >= 'A' && keyCode <= 'Z'))
        return std::string(1, static_cast<char>(keyCode));
    if (keyCode == 32)
        return "SPACE";
    if (keyCode >= 112 && keyCode <= 123)
        return "F" + std::to_string(keyCode - 111);

    switch (keyCode) {
    case 8: return "BACKSPACE";
    case 9: return "TAB";
    case 13: return "ENTER";
    case 16: return "SHIFT";
    case 17: return "CTRL";
    case 18: return "ALT";
    case 27: return "ESC";
    case 37: return "LEFT";
    case 38: return "UP";
    case 39: return "RIGHT";
    case 40: return "DOWN";
    default: return "KEY_" + std::to_string(keyCode);
    }
}

void SP_showKeybindWindow() {
    if (!showWindowFn)
        return;

    keybindPrompt = "Press a new preview key (current: " + SP_keyCodeToString(spPreviewKey) + ")";
    keybindControl.type = 3;
    keybindControl.data.text.label = const_cast<char*>(keybindPrompt.c_str());
    keybindControl.data.text.size = 0;

    showWindowFn(
        keybindWindowTitle,
        1,
        nullptr,
        onKeybindWindowClosed,
        1,
        &keybindControl
    );
}

void SP_showTintIntensityWindow() {
    if (!showWindowFn)
        return;

    closeMenuWindow(tintIntensityWindowTitle);
    rebuildTintIntensityControls();

    showWindowFn(
        tintIntensityWindowTitle,
        1,
        nullptr,
        onTintIntensityWindowClosed,
        static_cast<int>(sizeof(tintIntensityControls) / sizeof(tintIntensityControls[0])),
        tintIntensityControls
    );
}

void SP_initModMenu() {
    void* menuLib = dlopen("libmcpelauncher_menu.so", 0);
    if (!menuLib) {
        std::printf("[ShulkerPreview] mod menu unavailable\n");
        return;
    }

    showWindowFn = reinterpret_cast<decltype(showWindowFn)>(dlsym(menuLib, "mcpelauncher_show_window"));
    closeWindowFn = reinterpret_cast<decltype(closeWindowFn)>(dlsym(menuLib, "mcpelauncher_close_window"));
    addMenuFn = reinterpret_cast<decltype(addMenuFn)>(dlsym(menuLib, "mcpelauncher_addmenu"));

    if (!showWindowFn || !addMenuFn) {
        std::printf("[ShulkerPreview] mod menu symbols missing\n");
        return;
    }

    static MenuEntryABI rootEntry{};
    static MenuEntryABI subEntries[3]{};

    subEntries[0].name = "Change preview key";
    subEntries[0].user = nullptr;
    subEntries[0].selected = [](void*) -> bool { return false; };
    subEntries[0].click = [](void*) {
        spChangingPreviewKey = true;
        SP_showKeybindWindow();
    };
    subEntries[0].length = 0;
    subEntries[0].subentries = nullptr;

    subEntries[1].name = "Tint intensity";
    subEntries[1].user = nullptr;
    subEntries[1].selected = [](void*) -> bool { return false; };
    subEntries[1].click = [](void*) { SP_showTintIntensityWindow(); };
    subEntries[1].length = 0;
    subEntries[1].subentries = nullptr;

    subEntries[2].name = "Show above tooltip";
    subEntries[2].user = nullptr;
    subEntries[2].selected = [](void*) -> bool { return spAnchorAboveTooltip; };
    subEntries[2].click = toggleAnchorMode;
    subEntries[2].length = 0;
    subEntries[2].subentries = nullptr;

    rootEntry.name = "Shulker Preview";
    rootEntry.user = nullptr;
    rootEntry.selected = [](void*) -> bool { return false; };
    rootEntry.click = [](void*) {};
    rootEntry.length = sizeof(subEntries) / sizeof(subEntries[0]);
    rootEntry.subentries = subEntries;

    addMenuFn(1, &rootEntry);
    std::printf("[ShulkerPreview] mod menu entry added\n");
}
