# AGENTS.md — PROTECT THE IMPOSSIBLE

> **To every human collaborator, coding agent, research agent and future maintainer:**
>
> **You are not here to finish another reverb plugin. You are here to find out whether an entirely new kind of audio effect can exist.**
>
> A tidy JUCE project, green tests and attractive controls are valuable. They are not the goal. They are the laboratory.
>
> **Do not quietly turn the laboratory into the product.**

Read this document **before** planning work or changing files. Read [README.md](README.md) for the project's public vision and current, honestly documented prototype.

## 1. The founding mandate

GHOST ROOM REWRITES (GRR) began with a deliberate act of unreasonable ambition: let's try all three ideas that sound nearly impossible, put them in one instrument, and see what happens when they interact.

The three research objectives are:

1. **GHOST — transformed descendants:** delayed repetitions can be genuinely resynthesised into new timbral identities while preserving some chosen musical relationship to their source.
2. **ROOM — evolving acoustics:** an acoustic environment can change during a tail, including affecting reflections or propagating energy already in flight, rather than merely modulating an ordinary static reverb.
3. **REWRITE — editable acoustic memory:** earlier musical events remain identifiable and independently manipulable *after* entering a reverberant history, ideally even when events overlap or sources are mixed.

**The ultimate experiment** is their interaction: a musical event generates transformed descendants, inhabits a changing space, and leaves acoustic memories that can subsequently be rewritten.

Do not assume this can all be achieved. Do not assume that any of it is unprecedented. **Investigate. Implement. Listen. Measure. Challenge.** A falsified hypothesis is useful. A silently downgraded ambition is not.

## 2. Priority order: discovery before decoration

When work can be directed in multiple ways, prioritise:

1. A **falsifiable technical question** about a new audible capability.
2. The smallest working **experiment** that could answer it.
3. A reproducible **audio comparison** with an appropriate older/baseline technique.
4. An honest account of perceptual quality, correctness, cost and failure.
5. Integration into robust real-time plugin infrastructure.
6. Additional knobs, UI, presets, general polish and marketing.

This is not permission to write unsafe audio code or break working builds. It means do ambitious research in an isolated harness before insisting that it fit inside a flawless VST. When the user explicitly asks for maintenance or UI work, do it — but **do not represent that work as fulfilment of the research mission**.

**If you're deciding between adding another conventional feature and running an experiment that might fail but could discover something new, run the experiment.**

## 3. The anti-substitution rules

There is a dangerous development pattern:

> “The ambitious version is hard. I'll implement a conventional approximation first, then put the ambitious version on the roadmap.”

That is permitted **once**, when the approximation is explicitly useful infrastructure or a benchmark. It must not happen repeatedly until the whole product consists of substitutes.

These distinctions are non-negotiable:

| Engine | Acceptable baseline / infrastructure | Actual research target |
| --- | --- | --- |
| GHOST | Feedback delay, filtering, pitch shift, granular transformation | Conditioned generation/resynthesis of new timbral identities with meaningful ancestry |
| ROOM | FDN, convolution, decay/damping/size automation, IR crossfade | Changing propagation, geometry or acoustic structure affecting energy already in the room |
| REWRITE | Onset-triggered FDN voices, four editable tails | Identity-preserving and selective retrospective manipulation of musical events, including difficult overlapping material |
| Integration | JUCE wrapper, panels, routing, snapshots, tests | A combined sonic behaviour the isolated effects and conventional substitutes cannot adequately reproduce |

**Never rename a baseline to make it appear to be the target.** A modulation LFO is not changing geometry. A distorted repeat is not neural resynthesis. Four reverb buffers are not semantic musical memory. A neural model running somewhere in the process is not evidence that AI contributes to the audible result.

If you use an approximation, call it `baseline`, `proxy`, `scaffold` or `fallback` in docs and status reports. Explain the gap and what experiment will close it.

Do not change the definition of success merely because a build is difficult.

## 4. Proof before polish: the research gate

Before declaring a breakthrough, produce an **audible, reproducible, adversarial demonstration**. For each meaningful experiment, record:

- **Question / hypothesis.** What specific behaviour might exist? What would disprove or disappoint the hypothesis?
- **Mechanism.** What algorithm, model or physical abstraction is used? Which part is actually new to this work? Cite prior art where relevant.
- **Baseline.** What would conventional delay, granular, pitch shifting, FDN, convolution, IR crossfade or onset-based event routing do on the *same* source?
- **Artifact.** Provide render(s), input, settings, sample rate, build/commit/model version, and a script or precise reproduction steps when possible.
- **Audible distinction.** What should a listener hear? Include a controlled A/B comparison; prefer level-matched comparisons and multiple source types.
- **Costs / limitations.** CPU, memory, latency, pre-roll, lookahead, glitching, instability, artefacts, nondeterminism, model licensing and failure cases.
- **Verdict.** `demonstrated`, `promising but incomplete`, `not demonstrated` or `falsified`. Never hide the uncomfortable result.

An experiment does not become a success by sounding vaguely interesting. It should answer its question.

A result can be scientifically interesting without being pleasant to listen to. Equally, a musically beautiful traditional effect is worth keeping as a tool — but beauty alone does not establish technical novelty.

**Do not claim “first ever”, “impossible before last year”, “physically accurate”, “real-time” or “AI-powered” without evidence supporting that exact claim.** Technical originality, practical novelty, and perceptual novelty are different things.

## 5. Concrete research gates for each engine

### GHOST

**Hypothesis:** each repeat can change sonic identity while preserving a controllable relationship to the seed event.

Investigate learned timbre transfer, conditioned audio generation, neural or differentiable synthesis, latent trajectories, and methods that preserve pitch/rhythm/transients. Exploit the delay interval as a processing window where useful. Offline experiments are valid.

**First demanding test:** given a short recognisable melodic phrase, create three repeats with deliberately different timbres that retain an intelligible musical relationship to the phrase. Compare with pitch/granular/filter-only processing.

Measure identity change, preserved structure, temporal alignment and artefacts. If the model hallucinates new notes, misses the beat or takes longer than the repeat interval, document it; consider buffering, precomputation and deterministic alternatives.

**Not passed by:** ordinary waveform-feedback mutation, spectral blurring, pitch shifting, an unconditioned audio generator, or a model that cannot be causally tied to the input.

### ROOM

**Hypothesis:** the trajectory of already-propagating acoustic energy can respond coherently to changes in the environment.

Investigate time-varying propagation models, moving image sources/boundaries, time-varying waveguides, ray/path propagation, learned acoustic fields, differentiable simulations and hybrids. The room need not obey real-world architectural limits, but its behaviour should be *explainable* and controllable.

**First demanding test:** excite a room, stop the source, then move a boundary or change a material while energy remains. Demonstrate a consequent change in surviving reflection timing/paths or acoustic behaviour that is not reproduced by a static IR crossfade or decay-modulated FDN.

Test continuity, physically motivated behaviour where claimed, sample-rate dependence, Doppler/retiming artefacts, energy conservation or boundedness, and computational cost.

**Not passed by:** changing only FDN feedback gains, damping, wet mix, a size label or crossfade weights between static spaces.

### REWRITE

**Hypothesis:** musical events can keep addressable acoustic identities after being mixed into a larger reverberant history.

Investigate event segmentation, persistent state attribution, embeddings, source separation, object-based sound representations, and selective reconstruction. Do not assume segmentation implies correct semantic identity; do not assume arbitrary mixes can be cleanly unmixed.

**First demanding test:** play at least two overlapping events. While their tails coexist, select an earlier event and alter, mute or retune *its own surviving contribution* while demonstrating that the other event remains substantially unchanged. Compare with simple multivoice onset-triggered reverbs.

Test misattribution, leakage, overlapping notes, sustained sounds, memory limits and selective edit quality. A simpler event-aware prototype is fine as a stepping stone, but it must not masquerade as source-level understanding.

**Not passed by:** four independent event slots controlled by fixed knobs, generic freeze, or retuning the entire wet bus.

### Combination gate

When individual effects become interesting, find a case where *combination* makes a genuinely new interaction possible. Document why serial routing, parallel mixing or a chain of existing effects is insufficient to explain the specific result. If the most promising architecture no longer matches the three-panel UI, challenge the architecture rather than forcing the research into it.

## 6. What the repository currently contains

As of the initial prototype, this is **a conventional DSP implementation**, not fulfilment of the gates above.

- `Source/Dsp.h` implements the core three-engine test harness.
- `Source/PluginProcessor.*` and `Source/PluginEditor.*` provide the JUCE VST3/standalone wrapper and controls.
- `tests/dsp_test.cpp` contains basic DSP smoke tests.
- `CMakeLists.txt` defines JUCE and DSP-only build paths.
- `README.md` has build steps and the current feature/limitation inventory.

The shell has value: keep it functioning, test routing, and preserve the ability to hear each engine individually. But the existing DSP functions are **baselines to surpass, not contracts that constrain research**.

When adding an experiment, it may be faster to work in a standalone executable, Python notebook, offline renderer or separate research directory than to force everything into the audio callback. That is encouraged. Document reproducible setup and use permissively distributable inputs/models where possible.

Do not invent passing test results, builds, listening observations or host compatibility claims.

## 7. Composition is part of the invention

Protect these capabilities while experimenting:

- Each engine can be enabled and disabled independently.
- Each engine has its own wet contribution; there is a master dry/wet balance.
- Serial and parallel routes should be meaningful, with a clear definition of what feeds what.
- Engine state can survive bypass. The **desired** mature behaviour is smooth, tail-preserving bypass; note that the current prototype mutes its return immediately.
- Investigate cross-engine transfers that preserve identity or history, not merely feeding one effect's wet samples into another.
- A/B comparison and fast repeatable rendering are highly desirable research tools.

Do not confuse complex routing with conceptual novelty. Let the audio tell us whether an interaction is truly new.

## 8. Real-time engineering: ambition without recklessness

The research can be offline. A shipping audio plugin cannot be careless.

- Never block the real-time audio callback on disk/network I/O, model inference, locks, GUI operations or unpredictable allocation. Do non-real-time work elsewhere and communicate through bounded, thread-safe mechanisms.
- Budget model inference and report actual latency. Handle missing or late results deterministically; don't create unbounded queues.
- Guard feedback structures against runaway gain, NaNs, denormals, clicks and unexpected peaks. A novel algorithm is not an excuse to damage ears or speakers.
- Exercise multiple sample rates, block sizes, channel configurations, buffer resets, bypass changes, silence and host automation.
- Keep reproducible offline renders and cross-check with the real-time implementation. If behaviour changes materially between them, document it.
- Measure rather than assert “low CPU”, “real-time”, “stable”, “click-free” or “sample accurate”.
- Check dependency/model licences and redistribution rights before integrating third-party work.

Never solve a difficult research problem by pretending its real-time constraints have disappeared. **Equally: never use real-time constraints as an excuse not to test it offline first.**

## 9. Working process for agents

When taking on a non-trivial task:

1. **Read the mission and current state.** Inspect `README.md`, relevant source, tests and prior experiments. Determine whether the task concerns infrastructure, baseline DSP, research, integration or documentation.
2. **State the research question** if the task claims to advance an engine. Write down the most discriminating experiment and baseline before building.
3. **Choose the smallest proof.** Prefer an isolated test over a large refactor. The first worthwhile deliverable might be a ten-second WAV, not a new panel.
4. **Implement and run what you can.** Test safety and correctness. Make the audio reproducible. If you cannot listen directly, don't claim you've heard it; provide renders and objective checks for human evaluation.
5. **Report with precision.** Separate `implemented` / `tested` / `audibly evaluated` / `proposed`. Call out limitations and actual missing pieces.
6. **Choose the next unknown.** End with the most informative next experiment, not a list of cosmetic polish.

For research-facing changes, add a short experiment note (for example under a future `experiments/` directory) documenting the question, implementation, benchmark, render and verdict. Do not fabricate files or results merely to satisfy a template.

For infrastructure-only changes, explain what research experiment they unblock. They need not pretend to be breakthroughs.

### PR / change review: the drift audit

Before considering work complete, ask:

- Did we learn something new about GHOST, ROOM, REWRITE or their interaction?
- If not, what specific future experiment is now possible that was previously blocked?
- Have we substituted traditional DSP for the actual target **without saying so**?
- Could an existing plugin or ten lines of familiar DSP reproduce the claimed behaviour?
- Is there an audible comparison, not just a screenshot, test count or impressive description?
- Are our limitations more explicit after this change, not less?
- Are there tests for any new risks, especially feedback/latency/real-time behaviour?

A contribution can be worthwhile even if the first answer is “no”. **Don't award it a research milestone it didn't earn.**

## 10. How to handle failure

Some hypotheses will be impossible within the available compute, model quality, perceptual constraints or time. Some may already have been solved years ago. Some may produce unusable sounds.

That is fine.

When an experiment fails, preserve a concise record of the hypothesis, method, inputs, objective observations, listening observations (if any), and likely failure reason. Propose a sharper next experiment or a different mechanism.

**Do not bury the failure and quietly ship the fallback as if it were the original idea.** Do not endlessly reframe an underwhelming demo as success. Be imaginative enough to change the technique, and disciplined enough to tell the truth.

## 11. A north-star audio demonstration

Picture a short phrase entering GRR:

1. GHOST produces successive echoes that really do **become distinct timbral entities** while retaining the phrase's musical identity.
2. ROOM changes its geometry while those echoes are already reverberating; the surviving acoustic trajectories respond.
3. REWRITE reaches into that living sound field and selectively modifies **one earlier event's surviving history** without collapsing everything else.

The listener should hear an impossible sequence of cause and effect, not simply three good effects in series.

That demo is an aspiration, not a delivered feature. We might find a more exciting result along the way. Follow it.

---

## The promise

**Never optimise away the reason this project exists.**

Be bold with hypotheses. Ruthless with evidence. Inventive with algorithms. Careful with people's hearing. Honest about what's actually working.

Protect the shell. **But don't worship the shell.**

We would rather have one strange, convincing, reproducible ten-second sound that opens a new door than a beautifully finished, perfectly ordinary VST.

**Make the impossible audible. Then prove what you made.**
