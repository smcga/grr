# CONVERGENCE-004 — labelled descendants inside a moving room

**Verdict: partial controlled convergence, not a completed shared acoustic-memory instrument.** We route a GHOST-002 neural descendant and an overlapping second event through ROOM-001's same time-varying 1D path geometry. After both sources stop, the walls move; during that movement we selectively erase or approximately octave-shift the ghost event's rendered tail, keeping the other event's contribution. Identities are supplied by the experiment. The edit operates on precomputed separated outputs, **not on the live internal wave state**. It therefore does not yet make an intervention inside a shared physical field. I could not listen to these WAVs.

## Listen

Run the commands below, then compare:

- `01_shared_room_unedited.wav`: the neural descendant and a second chord propagate through the same ROOM-001 moving geometry.
- `02_erase_ghost_tail.wav`: only the ghost contribution fades away, starting at 3.2 s.
- `03_octave_ghost_tail.wav`: only the ghost tail is passed through the rough REWRITE-003 spectral octave operator.
- `04_ghost_room_contribution.wav`, `05_other_room_contribution.wav`: the labelled contributions before editing.
- `06_ghost_descendant_input.wav`: the neural descendant exciting the room.

The first event is GHOST-002's first descendant, generated with 30% inherited waveform blend; the second is a synthetic chord. Their source recordings finish at 2.4 and 2.25 s. Walls begin moving at 2.8 s, and the edit happens during movement at 3.2 s. All are 48 kHz, stereo, 16-bit, eight seconds, with one common peak-protection gain; this retains relative levels, not perceptual loudness matching. The octave is approximate and may smear or distort.

## Mechanism and evidence

This uses the actual ROOM-001 renderer, compiled into `convergence_room`. Its geometry is a bank of independent one-dimensional paths with the same motion schedule for each rendering. We run it on the ghost input, on the other event, and on their combined input. The code verifies `ROOM(A+B)=ROOM(A)+ROOM(B)` to a tolerance of 1e-10; this is valid because the motion is predetermined and the propagation is linear, including its interpolation. We retain A and B arrays and edit A after the sources stop; this is not a conventional dry serial chain, because the selected event identity persists as a separately propagated contribution. However, given known inputs and linear systems, superposition is an established technique. No general source separation or novelty is claimed.

On this run the maximum discrepancy between combined propagation and summed labelled propagation was **9.44e-16**; the erased A component was exactly zero after its 40 ms fade; the unedited B component differed by at most **1.11e-16** from its original due to floating-point arithmetic. This checks the controlled arithmetic but does not measure separation quality or perception. The two stereo double event outputs use **12,288,000 bytes**, excluding inputs, workspaces, ROOM histories, model, and process overhead. One run took about 0.98 s after the neural WAV existed; this excludes GHOST inference/model loading. No DAW, streaming, stress, cross-platform, acoustic realism or listening validation has been done.

The edit operates on a stored, already-rendered tail. It does **not** erase or retune wave energy inside the underlying simulation and then continue propagation of the modified field. A linear model would permit labelled internal wave-state bookkeeping, but this prototype does not implement it. The neural source is not verified as a distinct instrument, and its inherited waveform makes the descendant a blend, not a fully independent synthesis. The spatial field is a stylized sum of tubes, not a coherent 3D room.

## Reproduce

First follow [GHOST-002](../ghost-002/README.md), including downloading the pinned model and generating `melodic_descendant_1.wav`; also install REWRITE-003's NumPy/SciPy/SoundFile dependencies. Then, from repository root:

```powershell
cmake -S experiments/convergence-004 -B build-convergence -G "Visual Studio 17 2022" -A x64
cmake --build build-convergence --config Release
.venv-ghost/Scripts/python.exe experiments/convergence-004/render.py --room-exe build-convergence/Release/convergence_room.exe --ghost experiments/ghost-002/renders/melodic_descendant_1.wav
```

On macOS/Linux use the relevant CMake generator, `build-convergence/convergence_room` for a single-configuration build, and `.venv-ghost/bin/python`. C++17 is required. Intermediate files are created in the render directory and removed automatically. The bridge uses little-endian raw float64 for internal communication; big-endian platforms are not supported without modification. Output filenames will be overwritten. `metrics.json` contains the comparison checks and runtime for each run. The assertions in the rendering script execute the superposition and erasure checks; they are not a substitute for listening.

## Next concrete test

Keep each labelled event as a separate **travelling-wave state inside the same ROOM-001 solver** and apply an edit at 3.2 s to the selected state, then continue evolving all states through the moving boundary. Compare this with post-render editing and include a deliberately nonlinear shared boundary where linear superposition fails. That would test whether identity can survive an intervention *within* a changing field, the missing mechanism here.
