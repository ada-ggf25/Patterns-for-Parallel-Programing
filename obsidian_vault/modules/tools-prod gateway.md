# `tools/prod` — the gateway module

A login node fresh out of the box exposes only a small subset of the CX3 software catalogue. Most of what you actually want — modern GCC, OpenMPI, SciPy bundles, R bundles, CMake — is gated behind a meta-module called `tools/prod`.

```bash
ml tools/prod
```

After this, `ml av` shows the full set. Always load it **first**, before anything else:

```bash
ml tools/prod
ml GCC OpenMPI CMake
```

## Why it works this way

`tools/prod` extends Lmod's module path — the directory list it scans for available modules. Until you load it, the directories holding the production toolchain literally aren't in Lmod's search path, so `ml av` doesn't see them and `ml GCC/14.2.0` will fail with "module not found".

`ml spider` *does* see modules behind gateways, which is why `ml spider GCC` will list versions you can't yet load. The "you may want to load tools/prod first" hint that spider prints is telling you about this gateway.

## Where you'll see this

- **Every PBS script in this course** opens with `ml tools/prod`.
- Common pitfall: trying `ml GCC OpenMPI` without `tools/prod` and getting "module not found". Always load `tools/prod` first.

## Related

- [[Lmod Commands]] — `ml spider` to find gated modules.
- [[Loading Combos]] — what to load after the gateway.
