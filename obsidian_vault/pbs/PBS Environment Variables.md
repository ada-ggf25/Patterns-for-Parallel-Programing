# PBS Environment Variables

PBS sets several environment variables inside a running job. The two you'll meet immediately are `$PBS_O_WORKDIR` and `$PBS_JOBID`.

## `$PBS_O_WORKDIR` — the CWD trap

When your job starts on the compute node, its working directory is **`$HOME`**, not the directory you ran `qsub` from. This breaks any script that does:

```bash
./my_program input.dat       # but input.dat is in the submission dir!
```

PBS sets `$PBS_O_WORKDIR` to the directory you submitted from. Every PBS script should start with:

```bash
cd $PBS_O_WORKDIR
```

This is so universal it should be muscle memory. Forgetting it is the #1 source of "file not found" errors in PBS jobs.

## `$PBS_JOBID`

The job ID assigned by PBS, e.g. `7315932.pbs-7`. Useful for naming output files uniquely:

```bash
./my_program > result_${PBS_JOBID}.txt
```

## Other PBS variables you may see

| Variable | What it holds |
|---|---|
| `$PBS_O_HOST` | Hostname of the node you submitted from |
| `$PBS_O_PATH` | Your `$PATH` at submission time (PBS replaces `$PATH` with its own) |
| `$PBS_NODEFILE` | Path to a file listing the compute hostnames assigned to this job |
| `$PBS_O_QUEUE` | Queue you submitted to |
| `$PBS_ENVIRONMENT` | `PBS_BATCH` for normal jobs, `PBS_INTERACTIVE` for `qsub -I` |
| `$TMPDIR` | Job-local scratch on the compute node — see [[../cluster/Filesystems]] |

`$PBS_NODEFILE` is mostly relevant when you're integrating with non-PBS tools — for OpenMPI you don't need to read it manually; `mpiexec` consults it automatically.

## Related

- [[Job Script Anatomy]] — where `cd $PBS_O_WORKDIR` belongs.
- [[../cluster/Filesystems]] — `$TMPDIR` and friends.
- [[Common PBS Mistakes]] — forgetting `cd $PBS_O_WORKDIR` is on this list.
