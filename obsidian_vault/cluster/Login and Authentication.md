# Login and Authentication

CX3 is reached over `ssh` only. Public-key authentication is **disabled** on RCS login nodes: every connection asks for your Imperial College password (the same one used for email/Outlook).

## The basic command

```bash
ssh myusername@login.cx3.hpc.ic.ac.uk
```

The hostname `login.cx3.hpc.ic.ac.uk` is a load-balancer DNS name covering four physical [[Login Shards|login shards]]. You may land on a different one each time.

The first connection prints a host-key fingerprint; accept it (`yes`) and it is stored in `~/.ssh/known_hosts`.

## Why `ssh-copy-id` does nothing useful

`ssh-copy-id` will run without error and write your public key into `~/.ssh/authorized_keys` on CX3. The next `ssh` will still prompt for a password, because RCS has key-based auth disabled at the server. Don't waste time chasing this — passwords are by design.

## Avoid retyping the password — `ControlMaster`

OpenSSH multiplexing reuses an existing connection for new sessions, so the password prompt happens at most once per "control persist" window:

```bash
# ~/.ssh/config
Host login.cx3.hpc.ic.ac.uk
    ControlMaster auto
    ControlPath  ~/.ssh/cm-%r@%h:%p
    ControlPersist 1h
```

After the first `ssh`, subsequent `ssh`, `scp`, `rsync`, etc. piggyback on the live connection without prompting.

## Network prerequisites

| Where you are | What works |
|---|---|
| Imperial-WPA wifi (campus) | direct `ssh` |
| Eduroam (campus) | direct `ssh` |
| Off campus | requires the **Zscaler / Unified Access** VPN |

If you see `ssh: connect to host …: Network is unreachable`, the cluster is fine — your VPN/wifi is the problem. From the open internet CX3 is unreachable.

## Related

- [[CX3 Overview]] — what you're connecting to.
- [[Login Shards]] — which physical box you actually reach.
- [[Filesystems]] — what's visible after you log in.
