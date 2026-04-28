# Login Shards

`login.cx3.hpc.ic.ac.uk` is a DNS load balancer over four physical login hosts. The four hosts split into two CPU classes:

| Shard | CPU | Use case |
|---|---|---|
| `login-b`, `login-c` | AMD Rome | Build host for Rome compute nodes |
| `login-ai`, `login-bi` | Intel Ice Lake | Build host for Ice Lake compute nodes |

The load balancer picks one at random — you may get a Rome shard today and an Ice Lake shard tomorrow.

## Does it matter for this course?

Almost never. GCC's binary-portable defaults (no `-march=native`) produce code that runs on both architectures. For the course examples (`pi_serial`, `pi_openmp`, `pi_mpi`) you can build on whichever shard you happened to land on.

## When it does matter

If you turn on aggressive architecture-specific tuning (`-march=native`, `-mavx512f`, etc.) the binary is no longer portable across the two CPU families. To make build host = run host:

- Bypass the load balancer and `ssh login-b.cx3.hpc.ic.ac.uk` (or `login-ai`, etc.) directly.
- Build there.
- Submit jobs with [[../pbs/cpu_type Selection|`cpu_type=rome`]] (or `icelake`) to ensure the compute node matches.

## Related

- [[CX3 Overview]] — the heterogeneous cluster context.
- [[../pbs/cpu_type Selection]] — pinning compute nodes to one architecture.
