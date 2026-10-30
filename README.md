# GHOST ROOM REWRITES

**Three engines. One instrument. An attempt to change what an audio effect can be.**

> **We are not building another reverb.**
>
> There are already thousands of excellent reverbs and delays. We did not start this project because the world needs one more.
>
> We started it because recent advances in machine learning, acoustic modelling, differentiable DSP and algorithm design suggest that some old assumptions about sound, time and causality might finally be worth challenging.
>
> **Make something we couldn't realistically have made before. Make the impossible audible.**

GHOST ROOM REWRITES (**GRR**) is an experimental audio effect and research playground, intended to become a VST3 instrument for manipulating **echoes, changing acoustic spaces, and the remembered past of a sound**.

The point is not to apply an “AI” badge to familiar DSP. The point is to find out what *new musical behaviour* becomes possible when sound can be resynthesised, spaces can evolve, and reverberant events can remain independently addressable after they have happened.

This repository contains a working **conventional-DSP prototype (v0.1)**. That prototype is **scaffolding, not the destination**.

## The three impossible questions

Each engine has its own on/off switch and wet level. They can run independently or together, in serial or parallel. The individual engines matter; what emerges when they interact may matter even more.

### GHOST — What if an echo could become something else?

A conventional delay repeats a waveform. Filters, saturation, granular processing and pitch shifting can progressively change it. GHOST wants to go further.

**A repeat should be capable of becoming a new sonic identity while remaining recognisably descended from the sound that caused it.**

Imagine a piano chord whose first echo becomes bowed glass, whose second becomes a metallic choir, and whose third becomes something without a familiar instrumental name — while its timing and musical ancestry survive.

The research problem is conditioned audio transformation or resynthesis: retaining the relevant notes, rhythm or gestures while changing timbral identity. The delay interval itself may provide a useful computation window.

**A convincing GHOST experiment:** an audible sequence of independently resynthesised repeats that retain a deliberate musical relationship to the input and are demonstrably different from ordinary pitch/filter/granular feedback.

**What does not satisfy it:** a feedback delay with progressively changing EQ, distortion, pitch or modulation, however lovely it sounds. Those are useful baseline effects, not the research result.

### ROOM — What if the space changed while the sound was still inside it?

Ordinary reverb treats a room as a fixed response, or morphs the parameters of an algorithm. ROOM asks a harder question:

**Can the acoustic environment change, and can reflections already in flight respond meaningfully to that change?**

Imagine striking a snare in a small concrete room. As its sound travels, the walls move, the chamber grows, surfaces change material, and the remaining energy follows the evolving physical or physically inspired acoustic structure.

We want to investigate time-varying propagation, moving boundaries, geometry-aware acoustics, and learned or accelerated acoustic models. Realism is one potential aesthetic; *impossible but coherent spaces* are another.

**A convincing ROOM experiment:** a reproducible sound in which a geometry/material change affects the trajectory or propagation of surviving acoustic energy in a way that a conventional static-room crossfade does not explain.

**What does not satisfy it:** an LFO on reverb decay, damping or wet level; or simply crossfading between two impulse responses. Those can be useful controls and baselines, but aren't the central discovery.

### REWRITE — What if a reverb's past could be edited?

An ordinary reverb blends previous sounds into an overlapping tail. REWRITE asks whether those sounds can continue to exist as *identifiable, editable musical events*.

**A note, hit or phrase should be able to leave a persistent acoustic memory that can be found and changed after the source has stopped.**

Imagine a chord progression already reverberating. Select the second chord's surviving reflections, retune them, move them, freeze them or erase them without disturbing the other chords' tails.

The research problems include event identity and segmentation, persistent per-event acoustic state, source separation when necessary, and retrospective operations that work without interrupting the rest of the sound.

**A convincing REWRITE experiment:** record overlapping events, select a past event, audibly change only its surviving acoustic contribution, and show that other events continue unaffected.

**What does not satisfy the long-term goal:** four threshold-triggered reverb instances with independent knobs. This is an excellent first test harness, but not general musical understanding or retrospective manipulation inside a complex shared field.

## Where things could get truly strange

These are **three independent modules in one plugin**, not three names for presets.

- **GHOST → ROOM:** a repeat becomes a new instrument, then inhabits a changing physical space.
- **ROOM → REWRITE:** the sound of an evolving room becomes something whose history can be selected and altered.
- **REWRITE → GHOST:** a remembered event is reinterpreted into new descendants.
- **All three:** one musical gesture spawns transformed descendants, travels through a moving acoustic environment, and leaves memories that can later be rewritten.

That last interaction is our north-star thought experiment. It is **not a feature currently implemented**.

The engines must remain independently bypassable and composable. The routing system should preserve meaningful differences between serial and parallel use. Bypassing an engine should eventually let existing tails decay naturally rather than abruptly killing their internal state.

## Our definition of progress

A **working plugin is not proof of an invention**.

We want small, honest, listenable experiments: a hypothesis, a baseline, a new algorithm, a repeatable render, and an explanation of what the result actually demonstrates. A beautiful failure that resolves a difficult question can be more valuable than six polished knobs that do nothing new.

A research milestone should answer:

1. **What can we hear that we could not get from the baseline?**
2. **What is the mechanism, and what recent research (if any) materially enables it?**
3. **What evidence distinguishes it from established techniques?**
4. **What are the latency, CPU, memory, artefact and determinism costs?**
5. **What remains unsolved or unproven?**

Do not claim a technique is unprecedented, “impossible before 2026”, neural, physically accurate, or real-time without evidence. We are trying to discover something genuinely new, not to market a familiar process with grander language.

For development and agent instructions, read **[AGENTS.md](AGENTS.md)**. It is part of the project's founding brief, not boilerplate.

---

## What exists today — prototype v0.1

The current codebase is a **stereo JUCE VST3/standalone plugin** with three switchable conventional DSP engines, independent wet levels, a master dry/wet control, and fixed-order serial or parallel routing. It is a usable laboratory in which to compare ideas.

| Engine | Implemented now | Not implemented yet |
| --- | --- | --- |
| **GHOST** | Cross-fed feedback delay with filtering, saturation and gentle delay-time modulation | Conditioned neural resynthesis, changing timbral identities, per-repeat pitch or content understanding |
| **ROOM** | Four-line feedback delay network with damping and time-varying decay | Moving geometry, in-flight reflection modelling, learned physical acoustics |
| **REWRITE** | Level-threshold event captures into four separately adjustable reverb states | Source-aware event separation, a timeline, per-event retuning/freeze/erase, general retrospective editing |
| **Host / routing** | VST3 and standalone targets; serial/parallel; switches; parameter state | Validated cross-platform behaviour, A/B snapshots, click-free control changes, full tail-state persistence |

The DSP smoke tests have reportedly passed on Windows with MSVC, and the Windows x64 build was reportedly opened in FL Studio. Cross-host testing, macOS/Linux builds, performance bounds and complete automation behaviour remain unverified. The existing tests are **not** listening tests or evidence of research novelty.

**Important limitations:** abrupt parameter changes, switching and slot replacement can click. There is no output limiter. At high feedback settings, monitor at a conservative volume. Bypassed engines continue to evolve their buffers but their returns are immediately muted; re-enabling them may reveal a surviving tail. This is not yet seamless, audible tail-preserving bypass.

### Build

Requirements: CMake 3.22+, C++17 compiler, JUCE 8.0.6 and JUCE's platform prerequisites (Visual Studio with C++ workload on Windows, Xcode on macOS, or the documented Linux dependencies).

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

CMake looks for an installed JUCE CMake package and otherwise fetches JUCE 8.0.6. To use an existing checkout, add `-DJUCE_PATH=/absolute/path/to/JUCE`.

For a typical Windows Visual Studio build:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DJUCE_PATH="C:/path/to/JUCE"
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Generated bundles are under `build/GhostRoomRewrites_artefacts/Release/` for multi-configuration builds (single-configuration builds may omit `Release`). On Windows copy the **entire** generated `GHOST ROOM REWRITES.vst3` bundle to your VST3 installation folder and rescan the DAW.

To build only the standalone DSP smoke tests without JUCE:

```sh
cmake -S . -B build-test -DGRR_DSP_TESTS_ONLY=ON
cmake --build build-test --config Release
ctest --test-dir build-test -C Release --output-on-failure
```

### Pull request preview downloads

Every pull request runs the **Build Windows VST3** GitHub Actions workflow. When it passes, open the pull request’s **Checks** tab, select **Build Windows VST3**, and open the job summary. The summary links to a ZIP containing the Windows x64 GHOST ROOM REWRITES and ROOM-001 VST3 bundles. The same ZIP appears under **Artifacts** on the Actions run page. You must be signed in to GitHub to download it; artifacts expire after 30 days. Extract the ZIP and copy the complete `.vst3` folders into `C:\Program Files\Common Files\VST3`, then rescan your DAW. These are experimental, unsigned PR builds, not release installers. The workflow also runs both projects’ smoke tests.

### Explore the existing shell

1. Insert GRR on a **stereo** track. Start with the master mix at about 25–35% and a safe monitoring level.
2. Try **GHOST** alone. Time is currently free-running in seconds, not tempo-synchronised. Mutation affects filtering, saturation and delay modulation; feedback governs repeats.
3. Try **ROOM** alone. Decay governs feedback duration, Material controls high-frequency damping, and Motion modulates decay. There is **no** geometric simulation in this implementation.
4. Try **REWRITE** with separated hits or notes. A rising level envelope crossing Threshold captures an event, subject to a minimum Spacing. New material goes to the active slot until the next capture. Tail 1–4 controls affect corresponding live states. A fifth event clears and reuses the first slot. There is no event timeline, pitch-aware detection or stable voice stealing yet.
5. Combine modules. **Serial** is fixed as GHOST → ROOM → REWRITE; enabled wet returns contribute to the next stage. **Parallel** feeds the same input to enabled engines and averages their wet returns by enabled-engine count. Levels scale wet returns; they are not individual dry/wet crossfades. With everything off, output is dry.

Plugin parameters are exposed and their values are saved through JUCE state handling; **audio history, delay-buffer contents, and capture position are not saved** in sessions. Only stereo processing is supported. Buffer allocation occurs during preparation, but audio-thread worst-case timing has not been fully measured.

### Tests

`tests/dsp_test.cpp` exercises a delay response, dry bypass, event capture, changing one captured tail, and finite/bounded results across all eight on/off combinations in both routing modes. These are smoke tests only, not real-time safety proofs, benchmarks, or claims of musical quality.

## Research roadmap — not a feature treadmill

**Phase 0 — The laboratory (existing prototype).** A buildable host, three independently testable effects, reliable routing, short regression tests. Keep this shell healthy, but don't mistake it for the project.

**Phase 1 — Three risky experiments.** Produce one credible *offline or real-time* proof-of-concept for each core question. Use a script, a native DSP executable or a throwaway prototype if it answers the question sooner. A production VST wrapper is **not required** to test the hypothesis.

- **GHOST:** use genuinely conditioned audio resynthesis on successive repeats. Compare with traditional transformations using matched phrases.
- **ROOM:** model changing reflection paths or acoustic propagation during a tail. Compare with FDN automation and impulse-response crossfading.
- **REWRITE:** capture overlapping musical events, retain identity, and retrospectively modify one surviving contribution. Compare with naive independent onset-triggered reverbs.

**Phase 2 — The interaction experiment.** Combine the strongest mechanisms and make a short, unmistakable A/B audio demonstration of behaviour no single module produces alone. The outcome might suggest a different architecture or even a fourth engine. Follow the sound, not the initial panel labels.

**Phase 3 — Make it an instrument.** Only once there is compelling audible behaviour, tackle host robustness, parameter smoothing, click-free bypass, latency reporting, model packaging/licences, worker-thread scheduling, deterministic offline rendering, preset/state handling, CPU limits and UI polish.

These phases can overlap where useful. None is permission to indefinitely postpone the research in favour of a comfortable ordinary effect.

## Contributing

Bring evidence, difficult questions, strange sounds, useful failures, and disciplined engineering. We welcome conventional DSP as a baseline or a necessary component. We do **not** regard conventional DSP dressed up as a breakthrough.

Before changing the project, read **[AGENTS.md](AGENTS.md)**. A substantial experiment should ideally include a short reproducible render or test, its baseline, its limitations and a clear statement of what was actually discovered.

## Licensing

No project-wide licence has been declared here. Do not assume permission to redistribute third-party code, training weights, audio examples or models. JUCE and any future research dependencies have their own licensing and distribution conditions.

---

**The question we will keep asking:** *Does this make sound do something surprising that we couldn't previously make happen in this way?*

If the answer is no, we haven't failed. We've found a baseline. **Now keep going.**
