# MPI Pitfalls

The five most common ways an MPI program fails.

## 1. `mpiexec: command not found`

**Symptom:** Job log shows `mpiexec: command not found`; nothing ran.

**Cause:** Forgot `ml OpenMPI`. `mpiexec` is not on the default PATH on CX3 — it ships with the MPI module.

**Fix:**

```bash
ml tools/prod
ml GCC OpenMPI
```

See [[../modules/Loading Combos]].

## 2. Mismatched `ncpus` and `mpiprocs`

**Symptom:** `ranks=8` but you asked for `ncpus=16` — wasted 8 cores. Or `ranks=16` on `ncpus=8` — over-subscribed, slower than 8 ranks.

**Cause:** PBS lets you set them independently; nothing enforces correspondence.

**Fix:** Keep `ncpus == mpiprocs` for pure-MPI jobs. For hybrid: `mpiprocs × ompthreads == ncpus`. See [[../pbs/Resource Selection]].

## 3. Collective deadlock

**Symptom:** Job hangs until walltime kills it. No output produced.

**Cause:** A collective (`MPI_Reduce`, `MPI_Bcast`, `MPI_Allreduce`, `MPI_Barrier`, ...) was called on some ranks but not others. The ranks that called it block waiting for everyone.

```cpp
// BUG: only rank 0 calls Reduce
if (rank == 0) {
    MPI_Reduce(&local, &global, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
}
```

**Fix:** Every rank in the communicator must call every collective. Move the `if (rank == 0)` *after* the collective:

```cpp
MPI_Reduce(&local, &global, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
if (rank == 0) std::cout << global << '\n';
```

See [[MPI Reduce]].

## 4. Filesystem race — every rank reads the same file

**Symptom:** Sporadic I/O errors, or every rank reading the same file slows linearly with rank count.

**Cause:** Every rank tries to read input independently; the parallel filesystem is not happy serving N concurrent reads of the same path.

**Fix:** Have rank 0 read the file, then `MPI_Bcast` the data:

```cpp
if (rank == 0) load_config(&data);
MPI_Bcast(&data, count, MPI_DOUBLE, 0, MPI_COMM_WORLD);
```

## 5. Forgetting `MPI_Finalize`

**Symptom:** Job appears to finish, but `mpiexec` reports a non-zero exit code or warns about leaked resources.

**Cause:** Returning from `main` without calling `MPI_Finalize` first. Legal but sloppy.

**Fix:** Always end with:

```cpp
MPI_Finalize();
return 0;
```

See [[MPI Init Finalize]].

## Bonus: "ran without `mpiexec`"

If you `./pi_mpi` directly instead of `mpiexec ./pi_mpi`, the program runs as a single rank — `MPI_Comm_size` returns 1. The program "works" and produces a correct answer (because rank 0 = whole problem), but no parallelism. Always launch with `mpiexec`. See [[mpiexec]].

## Related

- [[MPI Six Essentials]]
- [[MPI Reduce]]
- [[mpiexec]]
- [[MPI Init Finalize]]
- [[../pbs/Common PBS Mistakes]] — overlapping checklist for the PBS side.
