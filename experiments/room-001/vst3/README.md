# GRR ROOM-001 audition plugin

This separate stereo VST3 lets you audition the ROOM-001 moving-wall mechanism without changing the original three-engine plugin. It has not been tested in a DAW yet. Windows x64 VST3 compiled with JUCE 8.0.6 and MSVC 19.41; macOS/Linux not tested. The generic JUCE editor offers four parameters:

- **Comparison:** Static room, Moving walls, or Static crossfade. All three models run continuously so switching modes compares the same input history. The crossfade is between the static outputs, not the moving simulation.
- **Dry wet:** 0 is dry, 1 is effect only. Start around 0.35; use 1 to hear the tail alone.
- **Trigger threshold:** an input rising above this envelope threshold arms a cycle. If your sound is quiet, lower it. If you hear no change, use a louder/shorter hit and leave some silence.
- **Output trim:** controls the combined output, defaults to 0.7. The output is hard-clipped at +/-1 as an emergency ceiling, not a transparent limiter. Start with low monitor levels and avoid excessive input.

Use a single short snare or other hit, then leave several seconds of silence. The virtual walls start moving **0.8 seconds after the trigger**, expand over 1 second, stay expanded for 1.2 seconds, and return over 1 second. The trigger rearms approximately 4.1 seconds after a hit; later hits during the cycle still enter the acoustic model but don't start a new cycle. A new event needs the envelope to drop below threshold and cross it again. There is no visible trigger or motion indicator yet. Mode changes and parameter automation are not smoothed and may click; CPU and host behavior have not been measured comprehensively. Both stereo inputs are summed to mono before exciting the waveguide paths. Output is stereo but is not a spatially accurate simulation.

This is **not bit-identical to the offline render**: it takes live input, detects hits, returns walls to their original positions, has a choice control, and does not use the offline long-tail RMS matching. It does not implement the offline active-wall example. There are no claims of realistic architecture or a novel physical algorithm. See the parent README for the propagation model and measurements.

## Windows install

Extract the download and copy the entire `GRR ROOM-001.vst3` folder into `C:\Program Files\Common Files\VST3` (administrator approval may be needed). In FL Studio use Options > Manage plugins, select Verify plugins and Rescan plugins with errors, then Find installed plugins. Look for **GRR ROOM-001** as a mixer effect. It has a different identity from GHOST ROOM REWRITES so both can coexist.

## Build from source

```sh
cmake -S experiments/room-001/vst3 -B build-room001-vst -DCMAKE_BUILD_TYPE=Release -DJUCE_PATH=/path/to/JUCE
cmake --build build-room001-vst --config Release --target Room001Audition_VST3
cmake --build build-room001-vst --config Release --target Room001Smoke
ctest --test-dir build-room001-vst -C Release --output-on-failure
```

JUCE 8.0.6 is fetched if no installed package or JUCE_PATH is supplied; it has its own licensing terms. Output bundle is under `build-room001-vst/Room001Audition_artefacts/Release/VST3/` for multi-configuration generators. Processor smoke checks compare the first 3 seconds of static/moving output after a test tone, verify finite bounded samples, and exercise parameter state restore. They do not load the VST3 through a DAW or validate automation/latency/CPU.
