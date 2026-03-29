#include "keybinds.h"
#include "config.h"
#include "modmenu.h"

#include <dlfcn.h>
#include <stdio.h>

inline void (*_gw_add_window_creation_callback)(void *, void (*)(void *));
inline void *(*_gw_get_primary_window)();
inline void (*_gw_add_keyboard_callback)(
    void *window,
    void *user,
    bool (*callback)(void *, int, int));

bool spKeyDown = false;

void SP_register_keybinds() {
    auto gw = dlopen("libmcpelauncher_gamewindow.so", 0);
    if (!gw) {
        printf("[ShulkerPreview] cannot open gamewindow lib\n");
        return;
    }

    _gw_add_window_creation_callback =
        (decltype(_gw_add_window_creation_callback))
            dlsym(gw, "game_window_add_window_creation_callback");

    _gw_get_primary_window =
        (decltype(_gw_get_primary_window))
            dlsym(gw, "game_window_get_primary_window");

    _gw_add_keyboard_callback =
        (decltype(_gw_add_keyboard_callback))
            dlsym(gw, "game_window_add_keyboard_callback");

    if (!_gw_add_window_creation_callback || !_gw_get_primary_window || !_gw_add_keyboard_callback) {
        printf("[ShulkerPreview] missing keybind functions\n");
        return;
    }

    // Delay keyboard hookup until the real window exists.
    _gw_add_window_creation_callback(nullptr, [](void*) {
        void* window = _gw_get_primary_window();

        _gw_add_keyboard_callback(window, nullptr, [](void*, int key, int action) -> bool {
            if (spChangingPreviewKey) {
                if (action == 0) {
                    spPreviewKey = key;
                    spKeyDown = false;
                    SP_showKeybindWindow();
                    SP_saveConfig();
                    printf(
                        "[ShulkerPreview] Preview key set to %s (%d)\n",
                        SP_keyCodeToString(spPreviewKey).c_str(),
                        spPreviewKey
                    );
                }
                return true;
            }

            if (key == spPreviewKey) {
                if (action == 0)
                    spKeyDown = true;
                if (action == 2)
                    spKeyDown = false;
            }
            return false;
        });

        printf(
            "[ShulkerPreview] Keybind ready (%s)\n",
            SP_keyCodeToString(spPreviewKey).c_str()
        );
    });

    printf("[ShulkerPreview] Waiting for window...\n");
}
