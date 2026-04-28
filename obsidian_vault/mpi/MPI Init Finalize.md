# `MPI_Init` and `MPI_Finalize`

The bookends of every MPI program.

```cpp
int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    // ... entire MPI body lives here ...

    MPI_Finalize();
    return 0;
}
```

## `MPI_Init(&argc, &argv)`

- Must be called **before any other MPI function** in this rank.
- Connects the rank to the MPI runtime, parses any `mpiexec`-injected arguments out of `argv`, sets up `MPI_COMM_WORLD`.
- Must be called **exactly once** per rank. Calling it twice is undefined behaviour.

Why it takes `&argc, &argv`: historically, the MPI launcher passed configuration through extra command-line arguments (host list, port number, etc.). `MPI_Init` strips them so your program sees a clean `argv`. Modern launchers use environment variables, but the API kept the old signature for compatibility — passing `nullptr, nullptr` is also legal.

## `MPI_Finalize()`

- Must be the **last MPI call** per rank.
- Releases MPI resources, flushes any buffered messages.
- After `MPI_Finalize`, no further MPI calls (including `MPI_Comm_rank`) are valid on this rank.

Forgetting `MPI_Finalize` is technically legal — the program will terminate normally — but it can hide resource leaks, leave pending messages dangling, and confuse some launchers about whether the run completed cleanly. Always call it.

## Common shape

```cpp
int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // ... work ...

    MPI_Finalize();
    return 0;
}
```

The first three lines (Init + Comm_rank + Comm_size) are universal. You'll write them in every MPI program you ever produce.

## Returning before `MPI_Finalize`

```cpp
if (rank == 0 && something_wrong) {
    std::cerr << "config error\n";
    return 1;             // BUG: other ranks still inside MPI region
}
```

Returning from `main` (or calling `exit()`) before `MPI_Finalize` will:

- Leave the runtime in an inconsistent state.
- Cause other ranks to either hang or be killed by the launcher.

If you need to abort across all ranks:

```cpp
MPI_Abort(MPI_COMM_WORLD, /*errorcode=*/1);
```

`MPI_Abort` tells the launcher to kill every rank.

## Related

- [[MPI Six Essentials]]
- [[Rank and Size]]
- [[MPI Pitfalls]] — including "forgot MPI_Finalize".
