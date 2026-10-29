# ROOM-001 — A wave meets a moving wall

**Verdict: promising but incomplete.** A causal travelling-wave model responds to a moving wall after the source stops, and its result differs measurably from a crossfade of two static-room responses. Whether it sounds musically compelling remains for a listener to judge. I could not listen to the renders in this environment. This is neither a demonstrated 3D room nor a novel acoustics breakthrough.

## Listen and reproduce

Requirements: CMake 3.22 and a C++17 compiler; no JUCE, Python, external samples, trained models, or network required. From the repository root:

```sh
cmake -S experiments/room-001 -B build-room001 -DCMAKE_BUILD_TYPE=Release
cmake --build build-room001 --config Release
ctest --test-dir build-room001 -C Release --output-on-failure
```

Then render (on Windows use `build-room001/Release/room001.exe`; for a single-configuration Unix build use `build-room001/room001`):

```sh
build-room001/Release/room001.exe experiments/room-001/renders
```

On macOS/Linux, replace that executable path with `build-room001/room001`. The optional argument is the output directory; it is created if necessary. An existing set of WAVs with the same names will be overwritten. All signals are deterministic: 48 kHz, stereo, 16-bit PCM, 10 seconds. Synthetic snare-like noise/body source uses a fixed RNG seed. No third-party samples are needed. Renders are excluded from Git; generate them locally.

- `00-source.wav`: dry input, ending by 0.18 s. Every room version uses precisely this source, though their files are wet-only.
- `01-static.wav`: original static paths.
- `02-moving-wall.wav`: after 0.8 s, each right-hand wall expands with a smooth cubic trajectory for one second. Path lengths increase by roughly 12.3–17.55 m. Max speed is up to 26.3 m/s (~0.077c); deliberately exaggerated for audibility.
- `03-static-ir-crossfade.wav`: both static endpoint rooms have been independently excited by the same source from time zero. The two wet signals are mixed using the same smooth 0.8–1.8 s envelope. For these linear static systems, this is equivalent to crossfading the outputs of convolution with the two fixed room impulse responses. It does not move a wall through a living wave field.
- `04-impossible-collapse.wav`: a rapid inward wall movement from 0.8–1.15 s plus an **active reflection** of 1.045 in pressure. This exceeds unity for a stationary wall: impossible for a passive wall, but potentially realizable only with energy supplied by an active boundary. It is an intentional fictional, non-passive case, not a claim of violating physics. The opposite wall is lossy (0.95), so their stationary round-trip factor is 0.99275. Finite compression can still inject energy; there is no general stability proof.

All four room outputs are matched to the same stereo RMS over **0.8–10 seconds**, then scaled together so the largest peak is at most 0.8. This uses a different fixed gain for each file. Therefore the matched files have **different apparent levels even before the motion**; do not use their pre-motion levels as evidence of causality. The causality test below compares the unscaled simulations. Matching long-term RMS does not match short-window loudness or perceived loudness. The dry input is at its own level.

Start at a low monitor level: the active example is deliberately energetic. A useful listening order is static, crossfade, then moving wall; compare the surviving reflections from 0.8 s onward. The fourth file is the risky creative variant. The files are separate so you can align them in a DAW and switch between them.

## Mechanism

Eight independent 1D tubes, each with different starting length, are summed into an artificial stereo field. At the left endpoint, the source is added to a reflected return; both travelling-wave histories persist after the source stops. At right wall encounter time `tau`, the incident wave is read from the outgoing history at `tau - L(tau)/c`. Its pressure is multiplied by the chosen reflection factor and `(c-v)/(c+v)`. The reflected history is then read at the retarded wall time satisfying `t = tau + L(tau)/c`, solved by five Newton iterations. Smooth cubic wall trajectories and linear sample interpolation are used.

This models a wave actually meeting the moving boundary at its position and time and taking the appropriate return journey. Repeated reflections act on persistent wave histories, including after the source has ended. It can be implemented as time-varying delays; the relevant distinction is the explicit two-leg geometry and retarded-time scattering, not that delay lines are somehow nonphysical. The `c=343 m/s` pressure reflection expression is a simplified idealized 1D moving-wall approximation, with frequency-independent wall loss. A 1D duct is not a room: there is no 3D ray geometry, diffraction, inter-path scattering, realistic source/receiver placement, or full moving-medium boundary mechanics. The summed tubes are independent and the stereo weighting is artistic, not spatially accurate.

An active wall can amplify an incident pressure wave; a powered transducer could supply energy, so it is not thermodynamically impossible. The example is impossible **as a passive room**, not an impossible physical universe.

## Measurements from this run

Commands actually run on Windows x64, Visual Studio 2022 MSVC 19.41, release build:

```powershell
cmake -S experiments/room-001 -B build-room001 -G "Visual Studio 17 2022" -A x64
cmake --build build-room001 --config Release --parallel 1
ctest --test-dir build-room001 -C Release --output-on-failure
build-room001/Release/room001.exe --test
build-room001/Release/room001.exe experiments/room-001/renders
```

CTest: 1/1 passed. Representative results:

- An independently solved moving-wall pulse encounter predicts first return at 0.179140 s; measured 0.179146 s, error 0.0063 ms at 48 kHz. At 96 kHz the error was also 0.0063 ms. The bound checked is two samples, not a convergence study.
- During receding-wall motion, a 700 Hz input produced 614.46 Hz, versus approximately 616.63 Hz from instantaneous `(c-v)/(c+v)` at the mid-point. The 2.18 Hz difference includes a finite measurement window, varying velocity, integer zero crossings and interpolation. It establishes the direction and approximate magnitude for this case, not general Doppler accuracy.
- Before 0.8 s, unscaled static and moving waveforms matched exactly; after that point the maximum channel difference in the three-second test was 0.34845. The source is identically zero from 0.18 s. Moving versus endpoint crossfade differed by an unscaled stereo RMS of 0.05201 over 0.8–3 s. This does not by itself prove perceptual salience or that other sophisticated conventional processing cannot reproduce it.
- Three-second moving test was finite with maximum raw output amplitude 0.30968; the active-wall unit check measured reflection 1.045. All four ten-second files were normalized to a common maximum no greater than 0.8; this is an offline peak scale, not evidence that arbitrary parameter settings are stable. The measured 0.8-s onset sample-to-sample jumps in the matched static/moving files were approximately 0.0244/0.0372; gains differ, so those jumps are not directly comparable. The trajectory has continuous position and velocity, and pre-motion raw samples match; clicks and interpolation error have not been perceptually assessed.
- The release render took approximately 1.32 seconds wall time on the test machine for four eight-path simulations and five WAV writes, at 48 kHz and ten seconds each. This is a single offline run; it is not a real-time guarantee or a portable CPU benchmark. Each path temporarily holds two full-duration double histories (~7.3 MiB at 48 kHz/10 s); output arrays and source add further memory. Long renders scale linearly in duration; no peak memory measurement was taken.

Linear interpolation introduces damping and high-frequency/phase error, and changing read rates can alias. Newton iteration is not explicitly checked for convergence. For the supplied trajectories, wall velocity stays far below sound speed and path length stays positive; arbitrary settings are not supported. Frequency-independent reflectivity and interpolation-induced damping mean energy is not conserved exactly, even with nominally lossless walls. Moving walls can do work on waves; passive-wall energy need not remain constant during motion. The checks establish finite behavior for the stated examples, not a global energy bound or formal stability proof. No perceptual, measured-room, or cross-platform validation was done.

## Prior art and what is different here

- [Dym (1973), time-dependent acoustic boundaries](https://scholarship.claremont.edu/hmc_fac_pub/915/) establishes that changing acoustic boundaries are a long-standing research problem.
- [Smith and Van Duyne (1995), digital waveguide and finite-difference equivalence](https://www.auditory.org/asamtgs/asa95wsh/5aMU/5aMU8.html) and [Smith & Scavone (2006), travelling-wave air-column modelling](https://www.music.mcgill.ca/~gary/papers/p_089.pdf) motivate efficient 1D wave propagation with delay histories. This experiment uses established mathematics, rather than inventing a new propagation method.
- [Hamilton (2016), wave-based room acoustics](https://www.brianhamilton.co/thesis/hamilton2016phdthesis.pdf) discusses energy/passivity and explicitly notes that moving walls can do work on the field.
- [Hölter et al. (2024), FDTD based on nonlinear Euler equations](https://depositonce.tu-berlin.de/items/6709ec4e-c011-4db5-a68b-b06755c3ae04/full) illustrates more complete time-domain acoustics. A proper dynamic 2D FDTD or immersed-boundary model would be a substantially stronger physical comparison but costs more and introduces meshing and stability issues.
- [Bhosale et al. (2026), Echoes after Edits](https://www.merl.com/publications/TR2026-140) estimates post-edit room responses. It addresses geometry-conditioned static RIR estimation, not the specific in-flight moving-wall propagation implemented here. It is not used in this code.
- [Spatially dynamic RIR estimation (2024)](https://www.frontiersin.org/journals/signal-processing/articles/10.3389/frsip.2024.1426082/full) models moving microphone trajectories, another distinct dynamic problem. Neural acoustic fields can represent spatial response variation, but do not automatically simulate a boundary moving through existing waves. No AI was required for this controlled experiment.

These references are a focused starting point, not an exhaustive novelty search. There is no claim that the method, sounds, or comparison are unprecedented.

## Next experiment

Replace the independent tubes with a 2D waveguide mesh or FDTD grid, moving one boundary through an existing field. Record snapshots of the wave field and compare a wave packet striking the wall before versus during motion, against the same endpoint crossfade. Measure grid convergence, energy/work at the boundary, and listen for whether shared 2D propagation is meaningfully more room-like than these independent tubes. Do this before plugin integration.
