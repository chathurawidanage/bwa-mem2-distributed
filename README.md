# bwa-mem2 distributed

This is OpenMPI based distributed computation implementation of https://github.com/bwa-mem2/bwa-mem2

## Compiling

```bash
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=/path/to/mpicc -DCMAKE_CXX_COMPILER=/path/to/mpicxx
make
```

## Running

Indexing

```bash
mpirun -np <no_of_process> --hostfile <path_to_host_fule> ./bwa-distributed index <in.fasta>
```

Mapping

```bash
mpirun -np <no_of_process> --hostfile hostsfile ./bwa-distributed mem -t <no_of_threads_per_process> <in.fasta> <reads.fq/fa> -o <output_file_name>
```
