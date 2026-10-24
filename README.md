# Darth Phaser

A phaser audio plugin built with [JUCE](https://juce.com) and CMake.

> Status: DSP and GUI both implemented. Next up: factory preset polish and plugin validation.

## How it works

A cascade of first-order all-pass filters, summed back with the dry signal. Each all-pass
contributes frequency-dependent phase shift; where the shifted signal lands 180 degrees out
with the dry path, it cancels and a notch appears. An LFO sweeps the all-pass break frequency,
which drags the notches up and down the spectrum.

`source/PhaserEngine.{h,cpp}` holds the DSP and deliberately depends on no JUCE types, so it
can be compiled and tested on its own. `source/Parameters.{h,cpp}` declares the parameters and
`PluginProcessor` maps them onto the engine each block.

| Control | Range | Notes |
| --- | --- | --- |
| Speed | 0.01-10 Hz | Ignored when Tempo Sync is on |
| Tempo Sync / Division | 1/1 to 1/32, plus 1/4T and 1/8T | Follows the host BPM |
| Depth | 0-100 % | Full depth sweeps +/- 2 octaves around Centre |
| Stereo Phase | 0-180 deg | LFO offset between L and R |
| Waveform | Sine, Triangle, Square, Random | Random is sample and hold, per channel |
| Stages | 2, 4, 6, 8, 12 | Notch count is half the stage count |
| Centre | 80 Hz - 5 kHz | Middle of the sweep |
| Spread | 0-100 % | Fans the stages across +/- 1 octave |
| Feedback | -95 to +95 % | Negative inverts; sharpens the notches |
| Damping | 0-100 % | Low-pass inside the feedback path, 20 kHz down to 200 Hz |
| Mix | 0-100 % | **50 % gives the deepest notches** |
| Level | -24 to +12 dB | Output gain |

Mix is a dry/wet crossfade against the all-pass chain, which is the conventional phaser
topology. That means 100 % wet is *all-pass* and sounds close to unprocessed: the notches come
from cancellation against the dry path, so they are deepest at 50 %.

Coefficients are refreshed every 16 samples rather than per sample, since a `tan()` per stage
per sample is wasteful and inaudible at LFO rates. Feedback is soft-clipped with `tanh` and
clamped to +/- 0.95 so high settings cannot run away.

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

## GUI

The editor is built from `design/darth-phaser-layout.png` at a logical 960x600 and scales to
whatever the host gives it, with the aspect ratio locked.

```
source/gui/
  Theme.h              # palette and type
  DarthLookAndFeel.*   # knurled MXR-style knob with a glowing value arc
  Knob.*               # knob + caption + formatted readout, bound to a parameter
  SegmentedControl.*   # STAGES / WAVEFORM / SYNC / DIVISION selectors
  PanelSection.*       # bevelled panel with the amber title bar
  LfoScope.*           # live response curve and LFO trace
  LevelMeter.*         # segmented L/R output meters
```

The scope is live rather than decorative. The upper area plots the actual magnitude response
of the all-pass cascade against the dry path, `|(1-mix) + mix*e^(j*phi)|`, where `phi` is the
summed phase of the stages at their current modulated frequencies, so the notches on screen
are the notches the DSP is producing. Feedback is not folded into the curve. The lower strip
scrolls the LFO waveform under a fixed playhead driven by the engine's real phase.

Two deliberate departures from the mockup:

- The mockup drew the response comb and the LFO sine across the same full-width axis, which
  would need one axis to be frequency and the other time. They are split instead: response on
  top, LFO on the bottom strip.
- `RESO` became `DAMP`, since a separate resonance control would duplicate Feedback.

The header's preset stepper and power LED are wired up: six factory presets in
`source/Presets.cpp`, and a `bypass` parameter exposed to the host through
`getBypassParameter()`.

Fonts are Futura for the wordmark and Avenir Next Condensed for labels, both macOS system
faces; other platforms fall back.

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

The mockup is currently **ahead of the editor** in three places, none of them ported yet:

| In the mockup | In `PluginEditor.cpp` today |
| --- | --- |
| Dedicated STEREO section with the Phase knob and an L/R offset display | Phase knob sits inside MODULATION |
| LFO / SWEEP shows only the red LFO trace | `LfoScope` also draws the live response curve above it |
| New / Save / Delete icons beside the preset display | Preset stepper only, six read-only factory presets |

The preset icons imply user presets stored on disk, which the plugin does not have yet.

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
