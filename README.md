# Dark Phaser

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
open "build/DarkPhaser_artefacts/Release/Standalone/Dark Phaser.app"
```

## Layout

```
CMakeLists.txt            # build definition, JUCE dependency, plugin metadata
source/
  PluginProcessor.h/.cpp  # AudioProcessor, parameter state, audio callback
  PluginEditor.h/.cpp     # plugin UI
```

## Plugin metadata

| Field | Value |
| --- | --- |
| Product name | Dark Phaser |
| Company | Rozan Audio |
| Bundle ID | com.rozanaudio.darkphaser |
| Manufacturer code | `Rzan` |
| Plugin code | `Dkph` |
