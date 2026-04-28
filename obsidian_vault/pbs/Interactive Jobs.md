# Interactive Jobs — `qsub -I`

For live debugging, exploratory commands, or short compile jobs, you can ask PBS for an interactive shell on a compute node.

```bash
$ qsub -I -l select=1:ncpus=2:mem=8gb -l walltime=02:00:00
qsub: waiting for job 7315933.pbs-7 to start
qsub: job 7315933.pbs-7 ready

[me@cx3-13-2 ~]$
```

You're now sitting on a compute node with the resources you asked for. The shell behaves like any other — except:

- It still counts against your quotas.
- It still has a walltime; PBS will kill the session when it expires.
- Disconnecting (e.g. closing your laptop without a tmux/screen on the login side) terminates the job.

## When to use it

- **Debugging a program that crashes only on compute nodes.** Run it under `gdb` interactively.
- **Exploring topology.** Run `lscpu`, `numactl --hardware`, `lstopo` and read them at leisure.
- **Compile farms.** Long C++ builds that would be killed on a login node.
- **Trying out commands** that you'll later wrap in a non-interactive PBS script.

## When *not* to use it

- **Production runs.** Use a non-interactive script so the job survives a network hiccup and the workflow is reproducible.
- **Long quiet jobs.** The interactive session ties up your terminal for the whole walltime.

## Tips

- **Wrap the outer ssh in `tmux` or `screen`** so a dropped connection doesn't kill the interactive job.
- **Modules don't auto-load.** Load them inside the interactive shell as you would in a script (`ml tools/prod GCC OpenMPI`).
- **You're already in the right CWD** — interactive jobs do *not* drop you in `$HOME` the way batch jobs do, so `cd $PBS_O_WORKDIR` isn't strictly necessary (though harmless).

The natural queue for interactive jobs is `v1_interactive` (see [[Queues]]).

## Related

- [[qsub qstat qdel]] — non-interactive submission.
- [[Queues]] — which queue interactive sessions land in.
