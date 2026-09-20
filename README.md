# Operating Systems Coursework

Process and thread programming in C, plus a parallel benchmark measured on a SLURM cluster.

**Course:** CENG322 — Operating Systems  
**Institution:** İzmir Institute of Technology (IYTE) — İzmir, Türkiye

## Files

| File | Contents |
|---|---|
| `pa2.c` | Process management — `fork`, `exec`, `wait` and pipes. Spawns child processes, connects them through file descriptors and reports their exit status |
| `pa3.c` | Threads and synchronisation — POSIX threads with `pthread` mutexes and `sem_t` semaphores guarding shared state |
| `test.c` | Scratch tests for string handling and shared-state behaviour |
| `slurm-homework/` | A parallel workload submitted to a SLURM cluster |

## The SLURM benchmark

`slurm-homework/` holds the job scripts and the recorded output:

| File | Purpose |
|---|---|
| `compile.sh`, `process.sh`, `compute.sh` | Build the workload, split the input, run the compute stage |
| `slurm_test.sh` | `sbatch` submission script |
| `slurm-288700.out`, `slurm-288704.out` | Captured stdout/stderr from two jobs |
| `output_shell.txt`, `output_times.txt` | Wall-clock timing per configuration |

The run compares wall-clock time across different process counts; the timing logs are kept
because they are the evidence behind the reported speedup.

```bash
bash compile.sh && bash process.sh
```

The scripts expect a SLURM cluster, so the recorded job logs are the results from the
original runs.

---

Submitted reports, worksheets and lecture material are archived outside this
repository rather than committed, so the repo stays code-only.
