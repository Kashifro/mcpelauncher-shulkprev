# mcpelauncher-shulkerboxpreview

A mod for [mcpelauncher](https://github.com/minecraft-linux/mcpelauncher-manifest) that adds shulker box content previews.

<!-- This mod currently supports **Minecraft Bedrock 1.21.114**. -->

## Usage
Create a `mods` directory in your mcpelauncher profile (for example `~/.local/share/mcpelauncher/mods`) if you do not already have one.
Place `libshulke.so` in that `mods` directory, then launch the game
Hover a shulker box item tooltip and press `H` (default) to toggle the preview panel.

From Mods -> `Shulker Preview`, you can:
- Change the preview keybind
- Adjust tint intensity (only scales brightness)

  ![Config](2.png)
## Building

Prerequisites:

- Android NDK r27 or later. [Download](https://developer.android.com/ndk/downloads)
- CMake 3.10 or later

Replace `/path/to/ndk` with your Android NDK path:

```bash
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/ndk/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=x86_64 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

This produces `libshulke.so` in `build/`. Install it as described in [Usage](#usage).

## Preview

![ShulkerBox Preview](1.png)
