# D-Bass

JUCE synth-bass plugin with mono legato phrasing, velocity accent behavior, FM timbre shaping, wavefold/drive distortion, and animated filtering.

## Source

- `Source/BassPluginProcessor.h`
- `Source/BassPluginProcessor.cpp`
- `Source/BassPluginEditor.h`
- `Source/BassPluginEditor.cpp`

## Build

```bash
cmake -S . -B build -DJUCE_DIR=/absolute/path/to/JUCE
cmake --build build --target DBassPlugin --config Release
cmake --build build --target DBassPlugin_Standalone DBassPlugin_AU DBassPlugin_VST3 --config Release
```

## Included bass presets (10)

- `drukqs metallic sub`
- `syro rubber bass`
- `ventolin broken acid bass`
- `sub trench pressure`
- `hollow fm weight`
- `glass growl mono`
- `detuned slab`
- `wide broken roller`
- `clean 2step foundation`
- `acid melt stomp`

## Included plugin artifacts

Prebuilt macOS artifacts are included under:

- `build/DBassPlugin_artefacts/Standalone/D-Bass.app`
- `build/DBassPlugin_artefacts/AU/D-Bass.component`
- `build/DBassPlugin_artefacts/VST3/D-Bass.vst3`
