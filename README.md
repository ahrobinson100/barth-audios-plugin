# Barth Audios Pitch Transposer & Delay — VST3 Plugin

Full modern replica of the Barth Audios Pitch Transposer & Delay (Klaus Fischer, ~50 units 1978-1983, modernized 2016-2022) as a VST3 plugin. Target: Ableton Live on Windows PC.

## Features

- **Pitch Shifting**: Two-pointer crossfade algorithm (glitch-free), ±5 octaves, independent L/R
- **Time Stretching**: Pitch-independent, 25-400%
- **Delay**: 0-5000ms with feedback (soft-limited), DC blocking
- **4-Step Sequencer**: Forward/backward/random/ping-pong modes, host sync, envelope trigger
- **Frequency Shifter**: Single-sideband via Hilbert transform
- **Ring Modulator**: Carrier x input with depth control
- **Phase Shifter**: 6-stage all-pass, stereo + tandem modes
- **Stereo Reverb**: FDN with Hadamard mixing, adjustable decay + room size
- **Distortion**: Waveshaper (tanh) + bass boost (low shelf)
- **ADSR Envelope**: Transient/keyboard/sequencer trigger, pre/post-reverb, invertible
- **Lo-Fi**: Bit crush (4-16 bit), sample rate divider, anti-alias LPF
- **Signal Routing**: Wet/dry mix, mono/diff output modes

## Build

### GitHub Actions (recommended)
Push to GitHub -> Windows/Mac/Linux builds run automatically. Download VST3 from Actions artifacts.

### Local Build (Windows)
```powershell
git clone --recurse-submodules https://github.com/youruser/barth-audios-plugin
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```

### Local Build (Mac/Linux)
```bash
git clone --recurse-submodules https://github.com/youruser/barth-audios-plugin
cmake -B build -G Ninja
cmake --build build --config Release
```

## Install
Copy `BarthAudios.vst3` to:
- **Windows**: `C:\Program Files\Common Files\VST3\`
- **Mac**: `~/Library/Audio/Plug-Ins/VST3/`
- **Linux**: `~/.vst3/`

## Open Source Foundation

| Project | License | Usage |
|---------|---------|-------|
| [pamplejuce](https://github.com/sudara/pamplejuce) | MIT | Project scaffold, CMake, CI |
| [Airwindows](https://github.com/airwindows/airwindows) | MIT | PitchDelay algorithm reference |
| [Signalsmith](https://github.com/Signalsmith-Audio/pitch-time-example-code) | MIT | Crossfade technique reference |
| [JUCE 8](https://github.com/juce-framework/JUCE) | GPL3 | Audio plugin framework |
| [Catch2](https://github.com/catchorg/Catch2) | BSL-1.0 | Unit tests |
