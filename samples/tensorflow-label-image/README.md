## Steps to fix the "clock_gettime() failed: Invalid argument" error

```bash
git clone https://github.com/trailofbits/tsc_freq_khz.git
cd tsc_freq_khz
make
sudo insmod ./tsc_freq_khz.ko
cat /sys/devices/system/cpu/cpu0/tsc_freq_khz
```
