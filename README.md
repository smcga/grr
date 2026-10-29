# GHOST ROOM REWRITES — prototype 0.1

One stereo effect with three independently switchable engines, a master dry/wet control, per-engine wet levels, and parallel or fixed-order serial routing. This is original experimental DSP code using JUCE for the plugin wrapper; it does not include neural models or the theoretical FFT algorithm. No claim is made that these effects were impossible before 2026.

## Build status

The DSP core and smoke tests compiled and passed on Windows with MSVC. The VST3 and standalone application also compiled successfully with JUCE 8.0.6. The Windows x64 build has been opened successfully in FL Studio. Broader host compatibility, automation, and state restoration remain unverified. macOS and Linux builds are untested. JUCE has its own licensing terms; review them for your use.

## Build the VST3 and standalone app

Requirements: CMake 3.22+, C++17 compiler, JUCE 8.0.6, and platform prerequisites for JUCE (Visual Studio with C++ workload on Windows, Xcode on macOS, or JUCE's documented Linux dependencies). In a terminal in this directory:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

CMake first looks for a JUCE CMake package. If none is found it downloads JUCE 8.0.6 from GitHub. For an existing source checkout without network access, configure with `-DJUCE_PATH=/absolute/path/to/JUCE` instead. On Windows, a typical build is:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DJUCE_PATH="C:/path/to/JUCE"
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Copy the complete generated `GHOST ROOM REWRITES.vst3` folder into `%COMMONPROGRAMFILES%/VST3`, then rescan in your DAW. The VST3 and standalone app appear under `build/GhostRoomRewrites_artefacts/Release/` (single-configuration generators may omit `Release`). Install the generated VST3 bundle into the appropriate VST3 folder and rescan your DAW; do not copy a partially built bundle. To compile only the DSP tests without JUCE use `cmake -S . -B build-test -DGRR_DSP_TESTS_ONLY=ON`, then build and run CTest as above.

## Play it

1. Insert on a **stereo** instrument or audio track; start with master mix around 25–35% and monitor conservatively. This prototype does not include a limiter.
2. Switch on only GHOST. Time is in seconds. Mutation increases filtering, saturation and gentle delay-time modulation on each feedback pass; feedback controls the number of repeats. The channels cross-feed.
3. Switch on only ROOM. Decay controls feedback length, material controls high-frequency damping, and motion slowly varies the decay. It is a four-line feedback delay network, not simulated geometry.
4. Switch on only REWRITE and play separated short notes or hits. A rising envelope crossing Threshold captures a new event, with Spacing limiting how often captures occur. The active event receives the input until the next capture. Tail 1–4 level, decay and tone controls alter the corresponding live reverb state. Events cycle through the four slots; the fifth clears and replaces slot 1. Lower Threshold or increase note spacing if events are missed. The controls identify slots, not musical notes, and there is no timeline or visual indication of the current slot.
5. Try combinations. Parallel feeds enabled engines the same input and averages their wet returns by the number enabled. Serial feeds input through GHOST → ROOM → REWRITE, adding each engine's wet return to the signal feeding the next; the master mixes the original dry signal with the resulting accumulated effect contribution. Engine level controls scale wet returns, not separate dry/wet crossfades. With all engines off the output is dry even when master mix is 100%.

Switching an engine off immediately mutes its return and stops feeding it new input; its buffers continue to evolve. Turning it back on can reveal any tail that still survives. Switches and parameters are not smoothed, so changes or slot replacement can click. Avoid abrupt automation and loud feedback settings until smoothing and output protection are added. Delay time is free-running, not tempo-synced.

## What this does and does not do

- **GHOST:** transformed waveform feedback, not per-repeat pitch shifting, instrument resynthesis, neural generation or musical pitch tracking.
- **ROOM:** a moving decay/damping proxy, not moving geometry, changed delay lengths, physical simulation, learned acoustics or a convolution reverb. Its fixed delay lengths mean the Decay control does not represent a physical room size.
- **REWRITE:** threshold-based event segmentation into four live, individually adjustable FDN states, not source separation. Closely spaced or sustained material may be grouped, missed or split. Capture timing depends on signal level. There is no per-event pitch, pan, freeze, erase button, manual capture, event timeline, or automatic quietest-voice stealing. Slot reuse abruptly clears a tail.
- Parameters are exposed for automation and their values are saved via JUCE state. Audio buffers, tail contents and capture position are not saved with a session. Host support has not yet been tested. No A/B snapshots are implemented.
- The processor is stereo only. It allocates its delay buffers on preparation, and processes samples without intentional heap allocation; voice clearing happens on the audio thread and its worst-case timing has not been measured. Controls update per audio block. Long-tail declaration is an estimate, not a guarantee for every parameter setting.

## Roadmap

1. Build and validate VST3 and standalone on Windows/macOS/Linux, test multiple hosts, measure CPU, check sample rates and block sizes, and add smoothing, click-free bypass/voice stealing and output protection.
2. Add capture indicators and a timeline, manual capture/erase, freeze, per-tail pan and pitch, eight or more voices, and A/B snapshots. Improve event detection and offer configurable voice stealing.
3. Add tempo sync and per-repeat pitch/granular or spectral transformation to GHOST; investigate conditioned neural resynthesis on a worker thread with explicit latency and deterministic fallback.
4. Add smoothly varying delay lengths, early reflections and frequency-dependent decay to ROOM; investigate physically motivated/learned geometry models.
5. Evaluate optional source separation and smarter event analysis off the audio thread for mixed material. Benchmark latency, licensing and model size before promising real-time use.

## Tests

`tests/dsp_test.cpp` covers delay response, all-off dry passthrough, event capture, independently changing one tail, and finite/bounded output for all eight engine combinations in both routing modes under a short signal. These are smoke tests, not listening tests, host compatibility tests, stability proofs or performance benchmarks.
