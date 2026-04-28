# The Six Essential MPI Calls

Most MPI programs need only these six functions. Anything more is optimisation or specialised use.

```cpp
MPI_Init(&argc, &argv);                                         // 1
MPI_Comm_rank(MPI_COMM_WORLD, &rank);                           // 2
MPI_Comm_size(MPI_COMM_WORLD, &size);                           // 3

// ... do parallel work ...

MPI_Reduce(&local, &global, 1, MPI_DOUBLE, MPI_SUM,             // 4 (or one of:
           /*root=*/0, MPI_COMM_WORLD);                         //    Send/Recv/Bcast)

MPI_Finalize();                                                 // 5
return 0;
```

## The bookends

| Call | Role |
|---|---|
| `MPI_Init(&argc, &argv)` | Initialise the MPI runtime. **Must be the first MPI call.** |
| `MPI_Finalize()` | Shut down. **Must be the last MPI call (per rank).** |

Between them lives the entire parallel body. See [[MPI Init Finalize]].

## Identifying yourself

| Call | Role |
|---|---|
| `MPI_Comm_rank(comm, &rank)` | "Which rank am I?" — fills `rank` with `0 .. size-1`. |
| `MPI_Comm_size(comm, &size)` | "How many ranks total?" — fills `size`. |

`MPI_COMM_WORLD` is the default communicator: "everyone launched together." See [[Rank and Size]].

## One communication call

Most simple programs need exactly one of these:

| Call | What it does |
|---|---|
| `MPI_Send`  / `MPI_Recv` | Point-to-point — one sender, one receiver |
| `MPI_Bcast` | One rank broadcasts to all others |
| `MPI_Reduce` | All ranks contribute; result lands on one root rank |
| `MPI_Allreduce` | All ranks contribute; result lands on **all** ranks |
| `MPI_Gather` / `MPI_Scatter` | Distribute / collect arrays |

For our π example we only need `MPI_Reduce` — every rank computes a partial sum, the root combines them. See [[MPI Reduce]].

## What "collective" means

Calls like `MPI_Reduce`, `MPI_Bcast`, `MPI_Allreduce` are **collectives**: every rank in the communicator must call them, and they implicitly synchronise. If only some ranks call a collective, the others wait forever — classic MPI deadlock. See [[MPI Pitfalls]].

## Beyond the six

Real MPI programs eventually use more — non-blocking sends (`MPI_Isend`), one-sided RMA (`MPI_Put`/`MPI_Get`), derived datatypes, sub-communicators, etc. — but for understanding π and most teaching examples, the six above are enough.

## Related

- [[MPI Init Finalize]]
- [[Rank and Size]]
- [[MPI Reduce]]
- [[../examples/pi_mpi]] — the running example.
