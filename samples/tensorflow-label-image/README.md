## Steps to fix the "clock_gettime() failed: Invalid argument" error

```bash
git clone https://github.com/trailofbits/tsc_freq_khz.git
cd tsc_freq_khz
make
sudo insmod ./tsc_freq_khz.ko
cat /sys/devices/system/cpu/cpu0/tsc_freq_khz
```

## Performance comparison with Graphene
- Graphene sample: https://github.com/Francis-Liu/graphene/tree/Francis-Liu.tensor-perf/Examples/tensorflow-lite
- Conditions:
  - Both grapnene and mystikos select Python 3.8.10
  - Both graphene and mystikos select 4G EPC memory
  - mystikos has 1024 threads by default; graphene is configued with 10 threads
  - In both graphene and mystikos, the `label_image` program uses 10 threads by default
- Server: Azure ACC VM "Standard DC24s v3 (24 vcpus, 192 GiB memory)"
- Mystikos ext2 rootfs size: 2.5G
- Result: average of 10 repeated runs [PerfResult](PerfResult.xlsx)
  - End-to-end time (turnaround):
    - Native:   0.32 sec
    - Graphene: 5.34 sec
    - Mystikos: 5.29 sec (0.89% faster than Graphene)
  - Processing time:
    - Native:   292.6 ms
    - Graphene: 485.1 ms
    - Mystikos: 334.4 ms (31.07% faster than Graphene)
