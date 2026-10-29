# GHOST-002 — neural descendants

**Verdict: promising but incomplete.** This is a working, offline neural resynthesis experiment with a repeat-to-repeat lineage. It does not establish that the output sounds like glass, choir or other specific instruments, or that all musical gestures are preserved. I could not listen to the files. Human A/B listening is required. It is not a new neural model or a claim of a breakthrough.

## Hypothesis and result

Given a short phrase, a pretrained audio autoencoder can re-encode and resynthesise each previous descendant, while gradually altering latent-channel statistics toward changing target references. Does the sound change while the phrase survives, and how does that differ from a conventional filtered-delay sequence?

The larger pretrained model produces reproducible finite audio on the machine used. Increasingly strong latently targeted transformations and recurrent re-encoding change the output. The measured level envelopes still resemble the melodic source to a degree, but progressively drift. Objective statistics do **not** prove a perceptible instrumental identity change or musical correctness. The reference names are descriptions of synthetic control signals, not instrument-recognition results.

## Hear it

Render files contain source followed by three separated 2.4-second generations. Compare:

- `melodic_neural_lineage.wav` / `percussive_neural_lineage.wav`: each generation uses the previous generation as its input.
- `melodic_neural_from_root.wav` / `percussive_neural_from_root.wav`: every generation starts from the same original source, with the same target order and mutation levels. Compare to hear the effect of accumulating generations.
- `melodic_baseline.wav` / `percussive_baseline.wav`: the same source and timing with cascaded conventional low-pass filtering as a deliberately simple delay baseline.
- `*_neural_matched.wav` and `*_baseline_matched.wav`: each repeat is RMS-normalized toward its source, capped at 4x boost, for a less level-biased listening comparison. All files receive a common additional peak-protection gain. Matching RMS is not perceptual loudness matching. The un-matched files preserve each method's natural relative evolution.
- `*_source.wav` and `*_descendant_1.wav` through `*_descendant_3.wav`: individual segments for DAW alignment. No separate WAV files are committed to the repository: the commands below reproduce them.

Synthetic melodic seed: six individually synthesized notes, not a piano recording. Synthetic percussive seed: eight generated impacts. Both use no third-party audio. Use conservative monitor levels. Outputs are mono 48 kHz 16-bit PCM, 9.6 seconds for the sequence. Repeated audio is concatenated at fixed intervals rather than mixed with feedback or a live DAW timeline.

## Model and mechanism

This is [RAVE](https://github.com/acids-ircam/RAVE), a neural variational audio autoencoder. The checkpoint is [Tangible Music Lab's 48 kHz Freesound-loop model](https://huggingface.co/Tangible-Music-Lab/RAVE_models), pinned to repository revision `3bf275ff6e223ddf9e82aacc5e9933f4af401fe8`; SHA-256 `0875195056b2990d411a6b38e63c5ea591d2df9b9bd4ac2c25c06a65c9e16de1`. The model card says MIT for the checkpoint; the original RAVE code has separate CC BY-NC terms, and its training corpus contains audio with varied Creative Commons terms. Check provenance and rights before redistribution or commercial use. No checkpoint is committed here.

Three generated reference signals (inharmonic tones, voiced harmonic tones, band-limited noise) are encoded once. For each descendant, the model encodes the current input, then channel-wise latent mean and standard deviation are shifted toward a selected reference's statistics before decoding. Mutation linearly controls this shift, increasing across the three generations. It is a manual latent-statistics experiment inspired by adaptive normalization, **not** the pretrained model's advertised AdaIN training or a validated glass/choir style transfer. `--identity` selects the three reference directions and order.

`--inheritance` is explicitly a dry waveform blend between each generation's input and neural output. At 1 it yields the generation input; at 0 the repeat is purely decoded. It is not a pitch detector or independent structural-content representation. `--evolution chain` feeds each result to the next neural pass; `--evolution root` starts each repeat from the original. The default files also include a root comparison. New model instances are loaded for each transform so the checkpoint's streaming cache cannot silently leak across phrases. The code seeds Torch and verifies a pinned model hash. This is local inference of downloaded serialized TorchScript: only load checkpoints you trust.

The checkpoint decodes a whole number of frames; this implementation zero-pads or truncates to the input duration and does **not** compensate measured neural delay. For custom mono/stereo WAV, `--input` downmixes to mono and resamples to 48 kHz; the input replaces the synthetic melodic seed while the synthetic percussive demo is still generated. Do not assume it handles long files efficiently or achieves pitch preservation on polyphonic input.

## Reproduce

Requires Python 3.13, approximately 150 MB for the checkpoint plus PyTorch dependencies, and C++ is not required. The test machine used CPU PyTorch 2.15.0, NumPy 2.5.4, SciPy 1.18.1, SoundFile 0.14.0 and four CPU threads. Version-pinned requirements reflect the environment used, not necessarily the only supported environment. No GPU or external audio inputs required.

From the repository root (Windows commands shown; use your environment's Python on macOS/Linux):

```powershell
python -m venv .venv-ghost
.venv-ghost/Scripts/python.exe -m pip install -r experiments/ghost-002/requirements.txt
.venv-ghost/Scripts/python.exe experiments/ghost-002/download_model.py
.venv-ghost/Scripts/python.exe experiments/ghost-002/test_experiment.py experiments/ghost-002/models/freesoundloop10k-default.ts
.venv-ghost/Scripts/python.exe experiments/ghost-002/render.py --model experiments/ghost-002/models/freesoundloop10k-default.ts --out experiments/ghost-002/renders
```

The downloader validates the model's SHA-256. Downloading it requires internet access; inference is offline afterward. With your own recording, add `--input "path/to/phrase.wav"`; other controls include `--identity noise metal voice --mutation 0.4 --inheritance 0.5 --evolution root`. Mutation and inheritance are between 0 and 1. This overwrites generated files with matching names in the selected output directory. Input recordings are not copied into the repository.

## Measurements and checks

Actual commands executed on Windows used an isolated `../ghost-env` venv and `../ghost-models/freesound-full.ts` holding the same pinned model. `test_experiment.py` passed: repeated runs with a fixed seed/fresh model matched exactly on this machine; inheritance=1 returned the input exactly; changing latent target strength changed the decoded output (RMS difference 0.1613 in the test). The renderer ran both the default chain and a custom-input/root-mode example. The existing JUCE shell's DSP-only CTest passed (1/1). The GPU was not used.

For default melodic lineage, 1024-sample-frame RMS envelope correlations with the source were **0.891, 0.776, 0.692** across generations, while the low-pass baseline retained approximately **1.000, 1.000, 1.000**. Allowing up to ±500 ms of alignment gave approximately **0.931, 0.897, 0.852**, with best lags **10.67, 21.33, 32.00 ms**. These are coarse envelope alignment estimates (10.67 ms hop), not a direct latency specification. Percussive unaligned envelope correlations fell **0.917 → 0.611 → 0.472**; approximate best aligned correlations were **0.959 → 0.859 → 0.883**. These are not perceptual or onset-detection scores. RMS normalization was not used for metric calculation.

Spectral centroids of the three melodic descendants were approximately **1220, 912, 2301 Hz**, versus **1024, 911, 691 Hz** for the conventional filtered repeats; this indicates spectral differences but cannot classify timbre. A local-frequency measurement around the *known* synthetic notes is included in `results.json`, but is biased by searching near the expected note and by the 30% waveform blend. It cannot establish pitch correctness, absence of hallucinated notes, or polyphonic preservation.

The final run took approximately **0.69–0.94 seconds per 2.4-second generation**, including loading a fresh model for each transform, on CPU with four Torch threads. That is not an end-to-end plugin benchmark; no streaming or real-time guarantee follows, and the complete phrase is required before processing here. Peak memory and cross-platform reproducibility were not measured. Each model load is ~150 MB on disk plus runtime memory. Model noise generation and floating-point behavior may vary on different PyTorch versions or devices.

An earlier alternative used the publisher's smaller 44.1 kHz Raspberry Pi checkpoint (`3ec093e132ce75d7fee3b8b734c739ebf8711a57ee332a60bce4359e2e34073e`). With the same overall idea, its melodic unaligned envelope correlations declined from 0.146 to -0.236 and -0.158. A separate unstyled reconstruction with the larger checkpoint produced 0.913 (0.974 after coarse alignment) and was chosen for the main experiment. This was not a controlled trained-model benchmark: sample rates, latencies and processing differ. It is evidence for a sharper follow-up, not a general quality ranking.

## Related work and next step

RAVE's [original research](https://arxiv.org/abs/2111.05011) established fast neural audio reconstruction and latent manipulation; [DDSP timbre transfer](https://magenta.tensorflow.org/demos/colab/) uses pitch/loudness-conditioned synthesis, and [control-transfer diffusion](https://github.com/NilsDem/control-transfer-diffusion/) investigates controllable audio style transfer. The present experiment combines known techniques into a repeatable lineage test; it does not establish an invention.

The next most informative GHOST experiment is to separate timing/pitch conditioning from timbre generation (for example, explicit pitch/onset-conditioned synthesis) and compare it with these recurrent latent transformations on the same phrase, including polyphonic and percussion failure cases. Then use human listening to determine if target identities are meaningfully distinct. See [REWRITE-003](../rewrite-003/README.md) and [CONVERGENCE-004](../convergence-004/README.md) for controlled follow-on experiments; neither establishes general source identity or a fully editable shared wave field.
