# Darth Phaser

A phaser audio plugin built with [JUCE](https://juce.com) and CMake.

> Status: project scaffold. The audio path is currently pass-through — the phaser DSP is next.

## Formats

- VST3
- Standalone application

Audio Unit is not built yet because it needs a full Xcode install (the Command Line Tools alone
are not enough). Add `AU` to `FORMATS` in `CMakeLists.txt` once Xcode is available.

## Requirements

- CMake 3.22+
- A C++20 compiler (Xcode Command Line Tools on macOS, MSVC on Windows, GCC/Clang on Linux)
- Git (JUCE is fetched automatically via `FetchContent`)

## Build

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The first configure clones JUCE, so it takes a few minutes. Subsequent builds are fast.

For a faster local iteration loop, build a single architecture and use Debug:

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build
```

`COPY_PLUGIN_AFTER_BUILD` is enabled, so a successful build installs the VST3 into the user
plugin folder (`~/Library/Audio/Plug-Ins/VST3` on macOS).

### Run the standalone

```sh
open "build/DarthPhaser_artefacts/Release/Standalone/Darth Phaser.app"
```

## Design

`design/darth-phaser-layout.png` is the GUI mockup the editor is being built toward, and
`design/darth-phaser-layout.html` is the source it renders from:

```sh
"/Applications/Google Chrome.app/Contents/MacOS/Google Chrome" --headless --disable-gpu \
  --screenshot=design/darth-phaser-layout.png --window-size=960,600 \
  --force-device-scale-factor=2 --hide-scrollbars \
  "file://$PWD/design/darth-phaser-layout.html"
```

Theme: near-black chassis with crimson rim light (Darth Vader), MXR Phase 90 orange as the
primary accent with knurled black knobs and white pointers, laid out in the dense multi-section
style of HY-Phaser. Panel size is 960x600.

## Editor setup (clangd / Neovim)

`CMAKE_EXPORT_COMPILE_COMMANDS` is on, so configuring the project writes
`build/compile_commands.json` and symlinks it to the project root. That file is what tells
clangd where the JUCE headers and the generated `JucePlugin_*` defines live — without it every
`#include <juce_.../...>` is reported as "file not found".

So run the configure step at least once before opening the project:

```sh
cmake -B build
```

Then restart the language server (`:LspRestart` in Neovim). Reconfigure whenever you add a
source file or change `CMakeLists.txt`, so the database stays in sync.

`compile_commands.json` is generated and git-ignored; `.clangd` points clangd at it and is
checked in.

## Layout

```
CMakeLists.txt            # build definition, JUCE dependency, plugin metadata
.clangd                   # points clangd at the generated compile database
source/
  PluginProcessor.h/.cpp  # AudioProcessor, parameter state, audio callback
  PluginEditor.h/.cpp     # plugin UI
```

## Plugin metadata

| Field | Value |
| --- | --- |
| Product name | Darth Phaser |
| Company | Rozan Audio |
| Bundle ID | com.rozanaudio.darthphaser |
| Manufacturer code | `Rzan` |
| Plugin code | `Dtph` |
