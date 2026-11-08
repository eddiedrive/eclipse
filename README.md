# Eclipse

Purple soft/hard clipper (VST3) for FL Studio, built with JUCE.

## Easiest: let GitHub build it
1. Create a free GitHub account and a new repository.
2. Upload everything in this folder (including the hidden .github folder) to it.
3. Open the repo's Actions tab. Wait for "Build Eclipse" to finish (about 10 minutes).
4. Open the finished run and download Eclipse-mac or Eclipse-windows from Artifacts.

## Install
- Mac:     put Eclipse.vst3 in ~/Library/Audio/Plug-Ins/VST3
- Windows: put Eclipse.vst3 in C:\Program Files\Common Files\VST3
Then in FL Studio: Options > Manage plugins > Find installed plugins.

## Build locally instead
cmake -B build
cmake --build build --config Release --target Eclipse_VST3
